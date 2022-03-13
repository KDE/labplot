#ifndef SPICEREADER_H
#define SPICEREADER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QVector>

class SpiceFileReader: public QObject {
	Q_OBJECT
public:
	struct Variable {
		int index;
		QString variableName;
		QString type;
	};

	enum Flags {
		real = 0x1, // if zero means the data is complex
		forward = 0x2,
		yDouble = 0x4, // y data is stored as double precision instead of float
		log = 0x8, // ?
	};

	SpiceFileReader(const QString& filename): mFilename(filename){
		mFile.setFileName(mFilename);
	}

	bool open();

	static QString convertLTSpiceBinary(const QByteArray& s);
	void init();
	int readData(std::vector<void*>& data, int skipLines = 0, int maxLines = -1);

	bool validSpiceFile() {
		if (!mInitialized)
			init();
		return mValid;
	}

	QString infoString() {
		return mInfoString;
	}
	bool binary() const {return mBinary;}
	int flags() const {return mFlags;}
	bool isReal() const {return (mFlags & Flags::real) > 0;}
	bool isDouble() const {return (mFlags & Flags::yDouble) > 0;}
	int numberSimulationPoints() {return mNumberPoints;}
	const QVector<Variable>& variables() const {return mVariables;}

	void setBulkReadLines(const int lines) {mNumberLines = lines;}

Q_SIGNALS:
	void processed(double percentage);
private:

	int parseFlags(const QString&);

	void addInfoStringLine(const QString& line) {
		if (!mInfoString.isEmpty())
			mInfoString += QLatin1String("<br>");
		mInfoString += line;
		if (line.at(line.length() - 1) != "\n")
			mInfoString += "\n";
	}

	// LTSpice data;
	QString mInfoString; // The complete header as string
	QString mTitle;
	QDateTime mDatetime;
	QString mPlotName;
	int mFlags;
	int mNumberPoints;
	double mOffset; // LtSpice specific
	QString mCommand; // LtSpice specific
	QVector<Variable> mVariables;

	bool mBinary{true}; // If the read file is binary of ascii

	QFile mFile;
	bool mValid{false}; // True if the file is a valid spice file
	bool mInitialized{false};
	bool mNgspice {true}; // ngspice raw file uses utf-8, but ltspice raw files use utf-16
	const QString mFilename;
	QString mFirstLine;

	// 10 variables and complex --> 100000 * (8 + 10*8) * 2 = 17.6MB
	int mNumberLines = 100000; // maximum lines in one (binary only)

};

#endif // SPICEREADER_H
