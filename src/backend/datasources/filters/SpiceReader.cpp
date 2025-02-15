/*
	File                 : SpiceReader.cpp
	Project              : LabPlot
	Description          : Reading spice files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SpiceReader.h"

#include "backend/lib/macros.h"

#include <QDataStream>
#include <QStringDecoder>

void SpiceFileReader::init() {
	bool ok;

	mInitialized = true;
	mInfoString = QString();

	if (!mFile.isOpen() && !open())
		return;

	QTextStream stream(&mFile);

	// Determine if ltspice or ngspice or none of both
	auto l = mFile.readLine();
	int pos = l.length();
	if (!QLatin1String(l).startsWith(QLatin1String("Title:"))) {
		if (!convertLTSpiceBinary(l).startsWith(QLatin1String("Title:")))
			return;
		mNgspice = false;
		mInfoString += convertLTSpiceBinary(l + mFile.read(1)); // because of utf16 end of line "\n 0x00" the 0x00 must be flushed
		stream.setEncoding(QStringConverter::Utf16);
		pos++;
	} else // title: removed trailing '\r' and '\n'
		addInfoStringLine(QLatin1String(l).trimmed());

	QString line = stream.readLine();
	if (!line.startsWith(QLatin1String("Date:")))
		return;
	mDatetime = QDateTime::fromString(line.split(QLatin1String("Date:"))[1].simplified()); // TODO: set format
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("Plotname:")))
		return;
	mPlotName = line.split(QLatin1String("Plotname:"))[1].simplified();
	mMode = plotNameToPlotMode(mPlotName);
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("Flags:")))
		return;
	mFlags = parseFlags(line.split(QLatin1String("Flags:"))[1].simplified());
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("No. Variables:")))
		return;
	addInfoStringLine(line);
	int numberVariables = line.split(QLatin1String("No. Variables:"))[1].simplified().toInt(&ok);
	if (!ok)
		return;

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("No. Points:")))
		return;
	addInfoStringLine(line);
	mNumberPoints = line.split(QLatin1String("No. Points:"))[1].simplified().toInt(&ok);
	if (!ok)
		return;

	if (!mNgspice) {
		line = stream.readLine();
		if (!line.startsWith(QLatin1String("Offset:"))) // LTSpice specific
			return;
		addInfoStringLine(line);
		mOffset = line.split(QLatin1String("Offset:"))[1].simplified().toDouble(&ok);
		if (!ok)
			return;
	}

	line = stream.readLine();
	if (!mNgspice) {
		while (!line.startsWith(QLatin1String("Variables:")) && !stream.atEnd()) {
			auto list = line.split(QLatin1Char(':'));
			if (list.length() < 2)
				return;
			addInfoStringLine(line);
			mLtSpiceOptions.insert(list[0].simplified(), list[1].simplified());
			line = stream.readLine();
		}
	}

	if (!line.startsWith(QLatin1String("Variables:"))) {
		DEBUG("SpiceReader: line does not start with the Variables key: " << line.toStdString());
		return;
	}
	addInfoStringLine(line);

	mVariables.resize(numberVariables);
	for (int i = 0; i < numberVariables; i++) {
		line = stream.readLine();
		auto sl = line.split(QLatin1Char('\t'));
		if (sl.length() < 4)
			return;
		auto index = sl.at(1).toInt(&ok);
		if (!ok)
			return;
		mVariables[i] = {index, sl[2], sl[3]};
		addInfoStringLine(line);
	}

	line = stream.readLine();
	mBinary = line.startsWith(QStringLiteral("Binary"));

	pos += stream.pos();
	// mFile must be reset and then seeked, because above the stream is used to read and then QFile fails
	// when mixing them together
	// Set pos to first databyte
	mFile.reset();
	mFile.seek(pos);

	mValid = true;
}

bool SpiceFileReader::open() {
	if (!mFile.open(QIODevice::ReadOnly)) {
		DEBUG("Failed to open the file " << STDSTRING(mFilename));
		return false;
	}
	return true;
}

int SpiceFileReader::readData(std::vector<void*>& data, int skipLines, int maxLines) {
	if (!mInitialized)
		init();

	const bool isComplex = !isReal();
	const int numberValuesPerVariable = 1 + isComplex;

	if (data.size() < (uint)(mVariables.count() * numberValuesPerVariable))
		return 0;

	// Assumption: if not a nullptr is in the array,
	// it is a valid pointer to an array
	for (uint i = 0; i < data.size(); i++) {
		if (data.at(i) == nullptr)
			return 0;
	}

	int linesRead = 0;
	if (mBinary) {
		// NgSpice: All data are 64bit
		// LtSpice: AC (complex): all data are 64bit
		// LtSpice: Transient: time is 64bit, y data is 32bit
		// LtSpice: Transient (double flag set): all data are 64bit
		const int yDataBytes = mNgspice ? 8 : ((isComplex | isDouble()) + 1) * 4; // in ltspice the values are stored normaly as single precision
		// the lines multiplied with the number of bytes per lines gives the number of bytes to read
		const int lineBytes = (8 + yDataBytes * (mVariables.count() - 1)) * numberValuesPerVariable;
		int patchesCount = 0;
		if (skipLines > 0)
			mFile.read(skipLines * lineBytes);
		while (!mFile.atEnd()) {
			const QByteArray ba = mFile.read(mNumberLines * lineBytes);
			const char* binary = ba.data();
			const int length = ba.length();
			if (length % lineBytes != 0) {
				DEBUG(Q_FUNC_INFO << ": The data is corrupted")
				return 0;
			}
			const int readLines = (int)(length / lineBytes);
			const int patchesIndexOffset = patchesCount * mNumberLines;

			for (int l = 0; l < std::min(readLines, mNumberLines); l++) {
				// time / frequency real part
				const int lineNumber = l * lineBytes;
				double value;
				memcpy(&value, &binary[lineNumber], 8);

				if (!mNgspice && mLTSpiceBug) {
					// Bug in the ltspice binary raw format
					// For more information see MR !108
					value = std::fabs(value);
				}

				(*static_cast<QVector<double>*>(data[0]))[patchesIndexOffset + l] = value;

				if (isComplex) {
					// time / frequency imaginary part
					memcpy(&value, &binary[lineNumber + 1 * 8], 8);
					(*static_cast<QVector<double>*>(data[1]))[patchesIndexOffset + l] = value;
				}

				for (int i = numberValuesPerVariable; i < mVariables.count() * numberValuesPerVariable; i++) {
					const int lineIndex = 8 * numberValuesPerVariable + (i - numberValuesPerVariable) * yDataBytes;
					if (lineIndex % (numberValuesPerVariable * 4) != 0)
						return linesRead;
					if (yDataBytes == 4) {
						float f = 0;
						memcpy(&f, &binary[lineNumber + lineIndex], yDataBytes);
						value = f;
					} else {
						memcpy(&value, &binary[lineNumber + lineIndex], yDataBytes);
					}
					(*static_cast<QVector<double>*>(data[i]))[patchesIndexOffset + l] = value;
				}
				linesRead++;
				if (maxLines > 0 && linesRead >= maxLines)
					return linesRead;
			}
			patchesCount++;
		}
	} else { // Ascii
		QTextStream stream(&mFile);
		if (!mNgspice)
			stream.setEncoding(QStringConverter::Utf16);

		for (int s = 0; s < skipLines; s++) {
			for (int i = 0; i < mVariables.count(); i++)
				stream.readLine();
			stream.readLine(); // read empty line
		}

		QString line;
		QLocale locale(QLocale::C);
		bool isNumber(false);
		linesRead = 0; // indexes the position in the vector(column). Because of the continue the loop index cannot be used
		const int points = maxLines > 0 ? std::min(mNumberPoints - skipLines, maxLines) : mNumberPoints - skipLines;
		for (int l = 0; l < points; l++) {
			for (int j = 0; j < mVariables.count(); j++) {
				line = stream.readLine();
				auto tokens = line.split(QLatin1Char('\t'));

				// skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
				if (tokens.size() < 2)
					continue;

				QString valueString = tokens.at(1).simplified(); // string containing the value(s), 0 is the index of the data
				if (isComplex) {
					auto realImgTokens = valueString.split(QLatin1Char(','));
					if (realImgTokens.size() == 2) { // sanity check to make sure we really have both parts
						// real part
						double value = locale.toDouble(realImgTokens.at(0), &isNumber);
						static_cast<QVector<double>*>(data[2 * j])->operator[](linesRead) = (isNumber ? value : NAN);

						// imaginary part
						value = locale.toDouble(realImgTokens.at(1), &isNumber);
						static_cast<QVector<double>*>(data[2 * j + 1])->operator[](linesRead) = (isNumber ? value : NAN);
					}
				} else {
					const double value = locale.toDouble(valueString, &isNumber);
					auto* v = static_cast<QVector<double>*>(data[j]);
					v->operator[](linesRead) = (isNumber ? value : NAN);
				}
			}
			linesRead++;
			stream.readLine(); // read the empty line between every dataset
			if (maxLines > 0 && linesRead >= maxLines)
				return linesRead;
		}
	}
	return linesRead;
}

QString SpiceFileReader::convertLTSpiceBinary(const QByteArray& s) {
	auto toUtf16 = QStringDecoder(QStringDecoder::Utf16);
	return toUtf16(s);
}

SpiceFileReader::PlotMode SpiceFileReader::plotNameToPlotMode(const QString& name) {
	mLTSpiceBug = true;
	if (name.contains(QLatin1String("Transient")))
		return PlotMode::Transient;
	else if (name.contains(QLatin1String("FFT")))
		return PlotMode::FFT;
	else if (name.contains(QLatin1String("DC"))) {
		mLTSpiceBug = false;
		return PlotMode::DC;
	} else if (name.contains(QLatin1String("AC")))
		return PlotMode::AC;
	else if (name.contains(QLatin1String("Noise")))
		return PlotMode::Noise;

	return PlotMode::Unknown;
}

int SpiceFileReader::parseFlags(const QString& s) {
	// real, forward, double, complex

	auto sl = s.split(QLatin1Char(' '));
	int value = 0;

	value |= sl.contains(QLatin1String("real")) ? Flags::real : 0;
	value |= sl.contains(QLatin1String("complex")) ? (value & ~Flags::real) : Flags::real; // TODO: check that real and complex are not in the same data
	value |= sl.contains(QLatin1String("forward")) ? Flags::forward : 0;
	value |= sl.contains(QLatin1String("log")) ? Flags::log : 0;
	value |= sl.contains(QLatin1String("double")) ? Flags::yDouble : 0;
	return value;
}
