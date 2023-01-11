/*
	File                 : BLFFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the BLF Filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BLFFilterTest.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "qtestcase.h"

#include <Vector/BLF.h>

static const std::string PRIMITIVE_DBC =
	R"(VERSION "1.0.0"

NS_ :

BS_:

BU_: DBG DRIVER IO MOTOR SENSOR

)";

//#define WRITE_CAN_MESSAGE(file, id, data) \
//    new Vector::BL

#define CREATE_DBC_FILE(filename, content)                                                                                                                     \
	do {                                                                                                                                                       \
		auto* file = std::fopen(dbcFile.fileName().toStdString().c_str(), "w");                                                                                \
		QVERIFY(file);                                                                                                                                         \
		std::fputs(PRIMITIVE_DBC.c_str(), file);                                                                                                               \
		std::fputs(dbcContent, file);                                                                                                                          \
		std::fclose(file);                                                                                                                                     \
	} while (false);

void createDBCFile(const QString& filename, const std::string& content) {
    auto* file = std::fopen(filename.toStdString().c_str(), "w");                                                                                \
    QVERIFY(file);                                                                                                                                         \
    std::fputs(PRIMITIVE_DBC.c_str(), file);                                                                                                               \
    std::fputs(content.c_str(), file);                                                                                                                          \
    std::fclose(file);
}

Vector::BLF::CanMessage2* createCANMessage(uint32_t id, uint64_t timestamp, const std::vector<uint8_t>& data) {
    auto* canMessage = new Vector::BLF::CanMessage2();
    canMessage->channel = 1;
    canMessage->flags = 1; // TX
    canMessage->dlc = std::min<uint8_t>(data.size(), 8);
    canMessage->id = id;
    canMessage->objectTimeStamp = timestamp;
    if (canMessage->data.size() < canMessage->dlc)
        canMessage->data.resize(canMessage->dlc);

    for (int i=0; i < canMessage->dlc; i++) {
        canMessage->data[i] = data.at(i);
    }
    return canMessage;
}

void createBLFFile(const QString& filename, QVector<Vector::BLF::CanMessage2*> messages) {
    Vector::BLF::File blfFile;
    blfFile.open(filename.toStdString().c_str(), std::ios_base::out);
    QVERIFY(blfFile.is_open());

    for (auto msg: messages) {
        blfFile.write(msg);
    }
    // Finish creating files
    blfFile.close();
}


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

void BLFFilterTest::testValidBLFValidDBCSingleMessage() {
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

    // Valid blf and valid dbc
    filter.setDBCFile(dbcFile.fileName());
    Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
    filter.readDataFromFile(blfFileName.fileName(), &s);
    QCOMPARE(s.columnCount(), 3); // time + Sig1 + Sig2

    {
        // Time
        const auto* c = s.column(0);
        QCOMPARE(c->rowCount(), 1);
        QCOMPARE(c->valueAt(0), 5);
    }

    {
        // Sig1
        const auto* c = s.column(1);
        QCOMPARE(c->rowCount(), 1);
        QCOMPARE(c->valueAt(0), 0x01);
    }

    {
        // Sig2
        const auto* c = s.column(2);
        QCOMPARE(c->rowCount(), 1);
        QCOMPARE(c->valueAt(0), 0x02);
    }
}

QTEST_MAIN(BLFFilterTest)
