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

#define CREATE_DBC_FILE(filename, content)\
    do {\
        auto* file = std::fopen(dbcFile.fileName().toStdString().c_str(), "w");\
        QVERIFY(file);\
        std::fputs(PRIMITIVE_DBC.c_str(), file);\
        std::fputs(dbcContent, file);\
        std::fclose(file);\
    } while (false);

void BLFFilterTest::test() {


    QTemporaryFile dbcFile(QStringLiteral("XXXXXX.dbc"));
    QVERIFY(dbcFile.open());

    /* open file for writing */
    QTemporaryFile blfFileName(QStringLiteral("XXXXXX.blf"));
    QVERIFY(blfFileName.open());

    {
        Vector::BLF::File blfFile;
        blfFile.open(blfFileName.fileName().toStdString().c_str(), std::ios_base::out);
        QVERIFY(blfFile.is_open());

        /* write a CanMessage */
        std::vector<uint8_t> data{0x01, 0x02};
        do {
            auto * canMessage = new Vector::BLF::CanMessage;
            canMessage->channel = 1;
            canMessage->flags = 1; // TX
            canMessage->dlc = std::min<uint8_t>(data.size(), 8);
            canMessage->id = 234;
            for (int i=0; i < canMessage->dlc; i++) {
                canMessage->data[i] = data[i];
            }
            blfFile.write(canMessage);
            //file.close();
        } while (false);

        // Finish creating files
        blfFile.close();
    }

    const auto dbcContent = R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Sig1 : 0|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX)";
    CREATE_DBC_FILE(dbcFile, dbcContent);



    // Start Test

    VectorBLFFilter filter;
    QCOMPARE(filter.isValid(QStringLiteral("UnavailableBLFFile.blf")), false); // File not available
    {
        QTemporaryFile invalidBLFFile;
        QVERIFY(invalidBLFFile.open());
        QFile f(invalidBLFFile.fileName());
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QStringLiteral("InvalidFile.blf").toStdString().c_str());
        f.close();
        QCOMPARE(filter.isValid(invalidBLFFile.fileName()), false);
    }
    QCOMPARE(filter.isValid(blfFileName.fileName()), true);

    {
        // File is valid, but dbc file not
        Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
        filter.readDataFromFile(blfFileName.fileName(), &s);
        QCOMPARE(s.columnCount(), 2); // column count not changed means no data was added
    }

    {
        // Valid blf and valid dbc
        filter.setDBCFile(dbcFile.fileName());
        Spreadsheet s(QStringLiteral("TestSpreadsheet"), false);
        filter.readDataFromFile(blfFileName.fileName(), &s);
        QCOMPARE(s.columnCount(), 1);
    }



}

QTEST_MAIN(BLFFilterTest)
