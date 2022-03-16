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

#include <QTextCodec>
#include <QDataStream>

void SpiceFileReader::init() {
	bool ok;

	mInitialized = true;
	mInfoString = "";

	if (!mFile.isOpen() && !open())
		return;

	QTextStream stream(&mFile);

	// Determine if ltspice or ngspice or none of both
	QByteArray l = mFile.readLine();
	int pos = l.count();
	if (!QString(l).startsWith(QLatin1String("Title:"))) {
		if (!convertLTSpiceBinary(l).startsWith(QLatin1String("Title:")))
			return;
		mNgspice = false;
		mInfoString += convertLTSpiceBinary(l + mFile.read(1)); // because of utf16 end of line "\n 0x00" the 0x00 must be flushed
		stream.setCodec(QTextCodec::codecForMib(1015));
		pos ++;
	} else // title: removed trailing '\r' and '\n'
		addInfoStringLine(QString(l).trimmed());

	QString line = stream.readLine();
	if (!line.startsWith(QLatin1String("Date:")))
		return;
	mDatetime = QDateTime::fromString(line.split("Date:")[1].simplified()); // TODO: set format
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("Plotname:")))
		return;
	mPlotName = line.split("Plotname:")[1].simplified();
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("Flags:")))
		return;
	mFlags = parseFlags(line.split("Flags:")[1].simplified());
	addInfoStringLine(line);

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("No. Variables:")))
		return;
	addInfoStringLine(line);
	int numberVariables = line.split("No. Variables:")[1].simplified().toInt(&ok);
	if (!ok)
		return;

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("No. Points:")))
		return;
	addInfoStringLine(line);
	mNumberPoints = line.split("No. Points:")[1].simplified().toInt(&ok);
	if (!ok)
		return;

	if (!mNgspice) {
		line = stream.readLine();
		if (!line.startsWith(QLatin1String("Offset:"))) // LTSpice specific
			return;
		addInfoStringLine(line);
		mOffset = line.split("Offset:")[1].simplified().toDouble(&ok);
		if (!ok)
			return;
	}

	if (!mNgspice) {
		line = stream.readLine();
		if (!line.startsWith(QLatin1String("Command:"))) // LTSpice specific
			return;
		addInfoStringLine(line);
		mCommand = line.split("Command:")[1].simplified();
	}

	line = stream.readLine();
	if (!line.startsWith(QLatin1String("Variables:")))
		return;
	addInfoStringLine(line);

	mVariables.resize(numberVariables);
	for (int i = 0; i < numberVariables; i++) {
		line = stream.readLine();
		auto sl = line.split("\t");
		if (sl.length() < 4)
			return;
		auto index = sl.at(1).toInt(&ok);
		if (!ok)
			return;
		mVariables[i] = {index, sl[2], sl[3]};
		addInfoStringLine(line);
	}

	line = stream.readLine();
	mBinary = line.startsWith("Binary");

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
		return false;

	// Assumption: if not a nullptr is in the array,
	// it is a valid pointer to an array
	for (uint i = 0; i < data.size(); i++) {
		if (data.at(i) == nullptr)
			return false;
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
				DEBUG("NgSpiceReader::readData: The data is corrupted")
				return false;
			}
			const int readLines = (int)(length/lineBytes);
			const int patchesIndexOffset = patchesCount * mNumberLines;

			for (int l = 0; l < qMin(readLines, mNumberLines); l++) {
				const int lineNumber = l * lineBytes;
				double value;
				memcpy(&value, &binary[lineNumber], 8);

				(*static_cast<QVector<double>*>(data[0]))[patchesIndexOffset + l] = value;

				if (isComplex) {
					memcpy(&value, &binary[lineNumber + 1 * 8], 8);
					(*static_cast<QVector<double>*>(data[1]))[patchesIndexOffset + l] = value;
				}


				for (int i = numberValuesPerVariable; i < mVariables.count() * numberValuesPerVariable; i ++) {
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
				linesRead ++;
				if (maxLines > 0 && linesRead >= maxLines)
					return linesRead;
			}
			patchesCount++;
		}
	} else { // Ascii
		QTextStream stream(&mFile);
		if (!mNgspice)
			stream.setCodec(QTextCodec::codecForMib(1015));

		for (int s = 0; s < skipLines; s++) {
			for (int i = 0; i < mVariables.count(); i++)
				stream.readLine();
			stream.readLine(); // read empty line
		}

		QString line;
		QLocale locale(QLocale::C);
		bool isNumber(false);
		linesRead = 0; // indexes the position in the vector(column). Because of the continue the loop index cannot be used
		const int points = maxLines > 0 ? qMin(mNumberPoints - skipLines, maxLines) : mNumberPoints - skipLines;
		for (int l = 0; l < points; l++) {

			for (int j = 0; j < mVariables.count(); j++) {
				line = stream.readLine();
				QStringList tokens = line.split(QLatin1Char('\t'));

				//skip lines that don't contain the proper number of tokens (wrong format, corrupted file)
				if (tokens.size() < 2)
					continue;

				QString valueString = tokens.at(1).simplified(); //string containing the value(s), 0 is the index of the data
				if (isComplex) {
					QStringList realImgTokens = valueString.split(QLatin1Char(','));
					if (realImgTokens.size() == 2) { //sanity check to make sure we really have both parts
						//real part
						double value = locale.toDouble(realImgTokens.at(0), &isNumber);
						static_cast<QVector<double>*>(data[2*j])->operator[](linesRead) = (isNumber ? value : qQNaN());

						//imaginary part
						value = locale.toDouble(realImgTokens.at(1), &isNumber);
						static_cast<QVector<double>*>(data[2*j+1])->operator[](linesRead) = (isNumber ? value : qQNaN());
					}
				} else {
					const double value = locale.toDouble(valueString, &isNumber);
					auto* v = static_cast<QVector<double>*>(data[j]);
					v->operator[](linesRead) = (isNumber ? value : qQNaN());
				}
			}
			linesRead ++;
			stream.readLine(); // read the empty line between every dataset
			if (maxLines > 0 && linesRead >= maxLines)
				return linesRead;
		}

	}
	return linesRead;
}

QString SpiceFileReader::convertLTSpiceBinary(const QByteArray& s) {
	// (1015 is UTF-16, 1014 is UTF-16LE, 1013 is UTF-16BE, 106 is UTF-8)
	// https://stackoverflow.com/questions/14131127/qbytearray-to-qstring

	return QTextCodec::codecForMib(1015)->toUnicode(s);
}

int SpiceFileReader::parseFlags(const QString& s) {
	//real, forward, double, complex

	auto sl = s.split(" ");
	int value = 0;

	value |= sl.contains("real") ? Flags::real : 0;
	value |= sl.contains("complex") ? (value & ~Flags::real) : Flags::real; // TODO: check that real and complex are not in the same data
	value |= sl.contains("forward") ? Flags::forward : 0;
	value |= sl.contains("log") ? Flags::log : 0;
	value |= sl.contains("double") ? Flags::yDouble : 0;
	return value;
}
