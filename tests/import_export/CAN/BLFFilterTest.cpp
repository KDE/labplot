/*
	File                 : BLFFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the BLF Filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BLFFilterTest.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/CANFilterPrivate.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "qtestcase.h"
#include <math.h>

#ifdef HAVE_VECTOR_BLF

#include <Vector/BLF.h>

static const std::string PRIMITIVE_DBC =
	R"(VERSION "1.0.0"

NS_ :

BS_:

BU_: DBG DRIVER IO MOTOR SENSOR

)";

// ##########################################################################################################
// ##### Helper functions ###################################################################################
// ##########################################################################################################
void BLFFilterTest::createDBCFile(const QString& filename, const std::string& content) {
	auto* file = std::fopen(filename.toStdString().c_str(), "w");
	QVERIFY(file);
	std::fputs(PRIMITIVE_DBC.c_str(), file);
	std::fputs(content.c_str(), file);
	std::fclose(file);
}

Vector::BLF::CanMessage2* BLFFilterTest::createCANMessage(uint32_t id, uint64_t timestamp, const std::vector<uint8_t>& data) {
	auto* canMessage = new Vector::BLF::CanMessage2();
	canMessage->channel = 1;
	canMessage->flags = 1; // TX
	canMessage->dlc = std::min<uint8_t>(data.size(), 8);
	canMessage->id = id;
	canMessage->objectTimeStamp = timestamp;
	canMessage->objectFlags = Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans;
	if (canMessage->data.size() < canMessage->dlc)
		canMessage->data.resize(canMessage->dlc);

	for (int i = 0; i < canMessage->dlc; i++) {
		canMessage->data[i] = data.at(i);
	}
	return canMessage;
}

void BLFFilterTest::createBLFFile(const QString& filename, const QVector<Vector::BLF::CanMessage2*>& messages) {
	Vector::BLF::File blfFile;
	blfFile.open(filename.toStdString().c_str(), std::ios_base::out);
	QVERIFY(blfFile.is_open());

	for (auto msg : messages) {
		blfFile.write(msg);
	}
	// Finish creating files
	blfFile.close();
}

// ##########################################################################################################
// ##### Test functions #####################################################################################
// ##########################################################################################################
void BLFFilterTest::testInvalidBLF() {
	VectorBLFFilter filter;
	QCOMPARE(filter.isValid(QStringLiteral("UnavailableBLFFile.blf")), false); // File not available

	// Create invalid blf file
	QTemporaryFile invalidBLFFile;
	QVERIFY(invalidBLFFile.open());
	QFile f(invalidBLFFile.fileName());
	QVERIFY(f.open(QIODevice::WriteOnly));
	f.write(QStringLiteral("InvalidFile.blf").toStdString().c_str());
	f.close();
	QCOMPARE(filter.isValid(invalidBLFFile.fileName()), false);
}

void BLFFilterTest::testNotFoundBLF() {
	VectorBLFFilter filter;
	QCOMPARE(filter.isValid(QStringLiteral("UnavailableBLFFile.blf")), false); // File not available
}

void BLFFilterTest::testInvalidDBC() {
	/* open file for writing */
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	std::vector<uint8_t> data{0x01, 0x02};
	QVector<Vector::BLF::CanMessage2*> messages{createCANMessage(234, 5, data)};
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 234 MSG1: 8 Vector__XXX
SG_ Sig1 : 0|8@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
SG_ Sig2 : 8|8@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// File is valid, but dbc file not
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 2); // column count not changed means no data was added
}

void BLFFilterTest::testValidBLFValidDBCSingleMessageBigEndian() {
	/* open file for writing */
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	std::vector<uint8_t> data{0x01, 0x02};
	QVector<Vector::BLF::CanMessage2*> messages{createCANMessage(234, 5, data)}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Sig1 : 7|8@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig2 : 15|8@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 3); // time + Sig1 + Sig2

	{
		// Time
		const auto* c = s.column(0);
		QCOMPARE(c->rowCount(), 1);
		QCOMPARE(c->valueAt(0), 5e-9); // nanoseconds
	}

	{
		// Sig1
		const auto* c = s.column(1);
		QCOMPARE(c->rowCount(), 1);
		QCOMPARE(c->valueAt(0), 0x01 * 0.1);
	}

	{
		// Sig2
		const auto* c = s.column(2);
		QCOMPARE(c->rowCount(), 1);
		QCOMPARE(c->valueAt(0), 0x02 * 0.1);
	}
}

// Use the previous value if there is no value at the current timestamp
void BLFFilterTest::testUsePreviousValueBigEndian() {
	/* open file for writing */
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(532, 5, {2, 62, 1, 161, 39, 6, 0, 214}),
		createCANMessage(565, 6, {0, 0, 0, 0, 0, 0, 0, 245}),
		createCANMessage(565, 8, {0, 0, 0, 0, 0, 0, 0, 148}),
		createCANMessage(532, 10, {2, 62, 1, 161, 39, 6, 0, 205}),
		createCANMessage(532, 12, {2, 62, 1, 161, 39, 6, 0, 206}),
		createCANMessage(532, 14, {2, 62, 1, 161, 39, 6, 0, 207}),
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 532 MSG1: 8 BMS1
 SG_ MSG1Sig1 : 39|16@0+ (0.01,0) [0|0] "%" Vector__XXX
 SG_ MSG1Sig2 : 23|16@0+ (0.1,0) [0|6553.5] "V" Vector__XXX
 SG_ MSG1Sig3 : 55|16@0- (0.1,0) [0|0] "" Vector__XXX
 SG_ MSG1Sig4 : 7|16@0+ (0.1,0) [0|6553.5] "V" Vector__XXX
BO_ 565 MSG2: 8 Vector__XXX
 SG_ MSG2Sig1 : 55|16@0- (0.1,0) [0|0] "" Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatPrevious);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 6); // time + Msg1Sig1 + Msg1Sig2 + Msg1Sig3 + Msg1Sig4 + Msg2Sig1

	{
		const auto* c = s.column(0);
		QCOMPARE(c->name(), i18n("Time_s"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{5e-9, 6e-9, 8e-9, 10e-9, 12e-9, 14e-9};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		const auto* c = s.column(1);
		QCOMPARE(c->name(), QStringLiteral("MSG1Sig1_%"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{99.90, 99.90, 99.90, 99.90, 99.90, 99.90};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		const auto* c = s.column(2);
		QCOMPARE(c->name(), QStringLiteral("MSG1Sig2_V"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{41.7, 41.7, 41.7, 41.7, 41.7, 41.7};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		const auto* c = s.column(3);
		QCOMPARE(c->name(), QStringLiteral("MSG1Sig3_"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{21.4, 21.4, 21.4, 20.5, 20.6, 20.7};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		const auto* c = s.column(4);
		QCOMPARE(c->name(), QStringLiteral("MSG1Sig4_V"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{57.4, 57.4, 57.4, 57.4, 57.4, 57.4};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		const auto* c = s.column(5);
		QCOMPARE(c->name(), QStringLiteral("MSG2Sig1_"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{std::nan(""), 24.5, 14.8, 14.8, 14.8, 14.8};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}
}

// Use NAN if the current message does not contain no value for the signal
void BLFFilterTest::testUseNANBigEndian() {
	/* open file for writing */
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(234, 5, {0x01, 0x02}),
		createCANMessage(123, 6, {0xFF, 0xA2}),
		createCANMessage(123, 8, {0x23, 0xE2}),
		createCANMessage(234, 10, {0xD3, 0xB2}),
		createCANMessage(234, 12, {0xE1, 0xC7}),
		createCANMessage(234, 14, {0xD1, 0xC7}),
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Msg1Sig1 : 7|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Msg1Sig2 : 15|8@0+ (1,0) [-3276.8|-3276.7] "km/h" Vector__XXX
BO_ 123 MSG2: 8 Vector__XXX
 SG_ Msg2Sig1 : 7|8@0+ (1,0) [-3276.8|-3276.7] "mm" Vector__XXX
 SG_ Msg2Sig2 : 15|8@0+ (1,0) [-3276.8|-3276.7] "m" Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(true);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatNAN);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 5); // time + Msg1Sig1 + Msg1Sig2 + Msg2Sig1 + Msg2Sig2

	{
		// Time
		const auto* c = s.column(0);
		QCOMPARE(c->name(), i18n("Time_s"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{5e-9, 6e-9, 8e-9, 10e-9, 12e-9, 14e-9};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig1
		const auto* c = s.column(1);
		QCOMPARE(c->name(), QStringLiteral("Msg1Sig1_C"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{0x01, NAN, NAN, 0xD3, 0xE1, 0xD1};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig2
		const auto* c = s.column(2);
		QCOMPARE(c->name(), QStringLiteral("Msg1Sig2_km/h"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{0x02, NAN, NAN, 0xB2, 0xC7, 0xC7};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig1
		const auto* c = s.column(3);
		QCOMPARE(c->name(), QStringLiteral("Msg2Sig1_mm"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{NAN, 0xFF, 0x23, NAN, NAN, NAN};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig2
		const auto* c = s.column(4);
		QCOMPARE(c->name(), QStringLiteral("Msg2Sig2_m"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{NAN, 0xA2, 0xE2, NAN, NAN, NAN};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}
}

void BLFFilterTest::testTimeNative() {
	/* open file for writing */
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(234, 5, {0x01, 0x02}),
		createCANMessage(123, 6, {0xFF, 0xA2}),
		createCANMessage(123, 8, {0x23, 0xE2}),
		createCANMessage(234, 10, {0xD3, 0xB2}),
		createCANMessage(234, 12, {0xE1, 0xC7}),
		createCANMessage(234, 14, {0xD1, 0xC7}),
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Msg1Sig1 : 0|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Msg1Sig2 : 8|8@0+ (1,0) [-3276.8|-3276.7] "km/h" Vector__XXX
BO_ 123 MSG2: 8 Vector__XXX
 SG_ Msg2Sig1 : 0|8@0+ (1,0) [-3276.8|-3276.7] "mm" Vector__XXX
 SG_ Msg2Sig2 : 8|8@0+ (1,0) [-3276.8|-3276.7] "m" Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(false);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatNAN);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 5); // time + Msg1Sig1 + Msg1Sig2 + Msg2Sig1 + Msg2Sig2

	{
		// Time
		const auto* c = s.column(0);
		QCOMPARE(c->name(), i18n("Time_ns"));
		QCOMPARE(c->rowCount(), 6);

		QVector<double> refData{5, 6, 8, 10, 12, 14};
		QCOMPARE(refData.size(), 6);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}
}

// Value5 is a value larger than one byte, but not exactly a multiple
// And is not aligned with a complete byte
void BLFFilterTest::testBigNumberNotByteAlignedLittleEndian() {
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(337, 5, {0, 4, 252, 19, 0, 0, 0, 0}),
		createCANMessage(337, 10, {47, 4, 60, 29, 0, 0, 0, 0}),
		createCANMessage(337, 15, {57, 4, 250, 29, 0, 0, 0, 0}),
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 337 STATUS: 8 Vector__XXX
 SG_ Value6 : 27|3@1+ (1,0) [0|7] ""  Vector__XXX
 SG_ Value5 : 16|11@1+ (0.1,-102) [-102|102] "%"  Vector__XXX
 SG_ Value2 : 8|2@1+ (1,0) [0|2] ""  Vector__XXX
 SG_ Value3 : 10|1@1+ (1,0) [0|1] ""  Vector__XXX
 SG_ Value7 : 30|2@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value4 : 11|4@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value1 : 0|8@1+ (1,0) [0|204] "Km/h"  Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(true);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatNAN);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	QCOMPARE(filter.d->readDataFromFile(blfFileName.fileName(), 4), 3);
	const auto dc = filter.dataContainer();
	QCOMPARE(dc.size(), 8); // Time + 7 signals
	{
		// Time
		const auto* v = static_cast<QVector<double>*>(dc.at(0));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 5e-9);
		QCOMPARE(v->at(1), 10e-9);
		QCOMPARE(v->at(2), 15e-9);
	}

	{
		// Value6
		const auto* v = static_cast<QVector<double>*>(dc.at(1));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 2.);
		QCOMPARE(v->at(1), 3.);
		QCOMPARE(v->at(2), 3.);
	}

	{
		// Value5
		const auto* v = static_cast<QVector<double>*>(dc.at(2));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 0.);
		QCOMPARE(v->at(1), 32.);
		QCOMPARE(v->at(2), 51.);
	}

	{
		// Value2
		const auto* v = static_cast<QVector<double>*>(dc.at(3));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 0.);
		QCOMPARE(v->at(1), 0.);
		QCOMPARE(v->at(2), 0.);
	}

	{
		// Value3
		const auto* v = static_cast<QVector<double>*>(dc.at(4));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 1.);
		QCOMPARE(v->at(1), 1.);
		QCOMPARE(v->at(2), 1.);
	}

	{
		// Value7
		const auto* v = static_cast<QVector<double>*>(dc.at(5));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 0.);
		QCOMPARE(v->at(1), 0.);
		QCOMPARE(v->at(2), 0.);
	}

	{
		// Value4
		const auto* v = static_cast<QVector<double>*>(dc.at(6));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 0.);
		QCOMPARE(v->at(1), 0.);
		QCOMPARE(v->at(2), 0.);
	}

	{
		// Value1
		const auto* v = static_cast<QVector<double>*>(dc.at(7));
		QCOMPARE(v->length(), 3);
		QCOMPARE(v->at(0), 0.);
		QCOMPARE(v->at(1), 47.);
		QCOMPARE(v->at(2), 57.);
	}
}

// Use NAN if the current message does not contain no value for the signal
void BLFFilterTest::testUseNANLittleEndian() {
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(337, 5, {0, 4, 252, 19, 0, 0, 0, 0}),
		createCANMessage(541, 10, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
		createCANMessage(337, 15, {47, 4, 60, 29, 0, 0, 0, 0}),
		createCANMessage(337, 20, {57, 4, 250, 29, 0, 0, 0, 0}),
		createCANMessage(541, 25, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 337 STATUS: 8 Vector__XXX
 SG_ Value6 : 27|3@1+ (1,0) [0|7] ""  Vector__XXX
 SG_ Value5 : 16|11@1+ (0.1,-102) [-102|102] "%"  Vector__XXX
 SG_ Value2 : 8|2@1+ (1,0) [0|2] ""  Vector__XXX
 SG_ Value3 : 10|1@1+ (1,0) [0|1] ""  Vector__XXX
 SG_ Value7 : 30|2@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value4 : 11|4@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value1 : 0|8@1+ (1,0) [0|204] "Km/h"  Vector__XXX"
BO_ 541 MSG2: 8 Vector__XXX
 SG_ MSG2Value4 : 48|16@1+ (0.01,-40) [-40|125] "C"  Vector__XXX
 SG_ MSG2Value1 : 0|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value3 : 32|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value2 : 16|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(true);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatNAN);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 12); // time + 7 * Msg1 + 4* Msg2

	{
		// Time
		const auto* c = s.column(0);
		QCOMPARE(c->name(), i18n("Time_s"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{5e-9, 10e-9, 15e-9, 20e-9, 25e-9};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig6
		const auto* c = s.column(1);
		QCOMPARE(c->name(), QStringLiteral("Value6_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{2., std::nan("0"), 3., 3., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig5
		const auto* c = s.column(2);
		QCOMPARE(c->name(), QStringLiteral("Value5_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., std::nan("0"), 32., 51., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig2
		const auto* c = s.column(3);
		QCOMPARE(c->name(), QStringLiteral("Value2_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., std::nan("0"), 0., 0., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig3
		const auto* c = s.column(4);
		QCOMPARE(c->name(), QStringLiteral("Value3_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{1., std::nan("0"), 1., 1., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig7
		const auto* c = s.column(5);
		QCOMPARE(c->name(), QStringLiteral("Value7_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., std::nan("0"), 0., 0., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg12Sig4
		const auto* c = s.column(6);
		QCOMPARE(c->name(), QStringLiteral("Value4_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., std::nan("0"), 0., 0., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig1
		const auto* c = s.column(7);
		QCOMPARE(c->name(), QStringLiteral("Value1_Km/h"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., std::nan("0"), 47., 57., std::nan("0")};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig4
		const auto* c = s.column(8);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value4_C"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan("0"), 22.2, std::nan("0"), std::nan("0"), 22.2};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig1
		const auto* c = s.column(9);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value1_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan("0"), 99.91, std::nan("0"), std::nan("0"), 99.91};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig3
		const auto* c = s.column(10);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value3_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan("0"), 79.3, std::nan("0"), std::nan("0"), 79.3};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig2
		const auto* c = s.column(11);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value2_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan("0"), 85.66, std::nan("0"), std::nan("0"), 85.66};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}
}

// Use previous value if the current message does not contain no value for the signal
void BLFFilterTest::testUsePreviousValueLittleEndian() {
	QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
	QVERIFY(blfFileName.open());
	QVector<Vector::BLF::CanMessage2*> messages{
		createCANMessage(337, 5, {0, 4, 252, 19, 0, 0, 0, 0}),
		createCANMessage(541, 10, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
		createCANMessage(337, 15, {47, 4, 60, 29, 0, 0, 0, 0}),
		createCANMessage(337, 20, {57, 4, 250, 29, 0, 0, 0, 0}),
		createCANMessage(541, 25, {7, 39, 118, 33, 250, 30, 76, 24}), // 99.91, 85.66, 79.3, 22.2
	}; // time is in nanoseconds
	createBLFFile(blfFileName.fileName(), messages);

	QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
	QVERIFY(dbcFile.open());
	const auto dbcContent = R"(BO_ 337 STATUS: 8 Vector__XXX
 SG_ Value6 : 27|3@1+ (1,0) [0|7] ""  Vector__XXX
 SG_ Value5 : 16|11@1+ (0.1,-102) [-102|102] "%"  Vector__XXX
 SG_ Value2 : 8|2@1+ (1,0) [0|2] ""  Vector__XXX
 SG_ Value3 : 10|1@1+ (1,0) [0|1] ""  Vector__XXX
 SG_ Value7 : 30|2@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value4 : 11|4@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value1 : 0|8@1+ (1,0) [0|204] "Km/h"  Vector__XXX"
BO_ 541 MSG2: 8 Vector__XXX
 SG_ MSG2Value4 : 48|16@1+ (0.01,-40) [-40|125] "C"  Vector__XXX
 SG_ MSG2Value1 : 0|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value3 : 32|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
 SG_ MSG2Value2 : 16|16@1+ (0.01,0) [0|100] "%"  Vector__XXX
)";
	createDBCFile(dbcFile.fileName(), dbcContent);

	// Start Test

	VectorBLFFilter filter;
	filter.setConvertTimeToSeconds(true);
	filter.setTimeHandlingMode(CANFilter::TimeHandling::ConcatPrevious);
	QCOMPARE(filter.isValid(blfFileName.fileName()), true);

	// Valid blf and valid dbc
	filter.setDBCFile(dbcFile.fileName());
	Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
	filter.readDataFromFile(blfFileName.fileName(), &s);
	QCOMPARE(s.columnCount(), 12); // time + 7 * Msg1 + 4* Msg2

	{
		// Time
		const auto* c = s.column(0);
		QCOMPARE(c->name(), i18n("Time_s"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{5e-9, 10e-9, 15e-9, 20e-9, 25e-9};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig6
		const auto* c = s.column(1);
		QCOMPARE(c->name(), QStringLiteral("Value6_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{2., 2., 3., 3., 3.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig5
		const auto* c = s.column(2);
		QCOMPARE(c->name(), QStringLiteral("Value5_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., 0., 32., 51., 51.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig2
		const auto* c = s.column(3);
		QCOMPARE(c->name(), QStringLiteral("Value2_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., 0., 0., 0., 0.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig3
		const auto* c = s.column(4);
		QCOMPARE(c->name(), QStringLiteral("Value3_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{1., 1., 1., 1., 1.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig7
		const auto* c = s.column(5);
		QCOMPARE(c->name(), QStringLiteral("Value7_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., 0., 0., 0., 0.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg12Sig4
		const auto* c = s.column(6);
		QCOMPARE(c->name(), QStringLiteral("Value4_"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., 0., 0., 0., 0.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg1Sig1
		const auto* c = s.column(7);
		QCOMPARE(c->name(), QStringLiteral("Value1_Km/h"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{0., 0., 47., 57., 57.};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			QCOMPARE(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig4
		const auto* c = s.column(8);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value4_C"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan(""), 22.2, 22.2, 22.2, 22.2};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig1
		const auto* c = s.column(9);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value1_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan(""), 99.91, 99.91, 99.91, 99.91};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig3
		const auto* c = s.column(10);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value3_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan(""), 79.3, 79.3, 79.3, 79.3};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}

	{
		// Msg2Sig2
		const auto* c = s.column(11);
		QCOMPARE(c->name(), QStringLiteral("MSG2Value2_%"));
		QCOMPARE(c->rowCount(), 5);

		QVector<double> refData{std::nan(""), 85.66, 85.66, 85.66, 85.66};
		QCOMPARE(refData.size(), 5);
		for (int i = 0; i < c->rowCount(); i++) {
			VALUES_EQUAL(c->valueAt(i), refData.at(i));
		}
	}
}

#endif

QTEST_MAIN(BLFFilterTest)
