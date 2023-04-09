
/*
    File                 : OriginFile.cpp
    Description          : Origin file import class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2005-2008, 2017 Stefan Gerlach
    SPDX-FileCopyrightText: 2007-2008 Alex Kargovsky Ion Vasilief <kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr (use @ for *)>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config.h"
#include "OriginFile.h"

#include <fstream>
#include <string>

using namespace std;

OriginFile::OriginFile(const string &fileName) : fileVersion(0), buildVersion(0), ioError(0)
{
    ifstream file(fileName.c_str(), ios_base::binary);

    if (!file.is_open()) {
        ioError = errno;
        return;
    }

#ifdef GENERATE_CODE_FOR_LOG
    FILE *logfile = nullptr;
    logfile = fopen("./opjfile.log", "w");
    if (logfile == nullptr) {
        ioError = errno;
        return;
    }
#endif // GENERATE_CODE_FOR_LOG
    LOG_PRINT(logfile, "File: %s\n", fileName.c_str())

    string vers;
    getline(file, vers);
    file.close();

    long majorVersion = strtol(vers.substr(5, 1).c_str(), nullptr, 10);
    // char locale_decpoint = vers[6];
    buildVersion = strtol(vers.substr(7).c_str(), nullptr, 10);
    // long buildNumber = strtol(vers.substr(12).c_str(),0,10);

    // translate version
    // see https://www.originlab.com/index.aspx?go=SUPPORT&pid=3325
    unsigned int newFileVersion = 0;
    if (majorVersion == 3) {
        if (buildVersion < 830)
            fileVersion = 350;
        else
            fileVersion = 410;
    } else if (buildVersion >= 110 && buildVersion <= 141) // 4.1
        fileVersion = 410;
    else if (buildVersion <= 210) // 5.0
        fileVersion = 500;
    else if (buildVersion < 2624) // 6.0
        fileVersion = 600;
    else if (buildVersion < 2628) // 6.0 SR1
        fileVersion = 601;
    else if (buildVersion < 2635) // 6.0 SR4
        fileVersion = 604;
    else if (buildVersion < 2656) // 6.1
        fileVersion = 610;
    else if (buildVersion < 2659) // 7.0 SR0 (2656-2658)
        fileVersion = 700;
    else if (buildVersion < 2664) // 7.0 SR1 (2659-2663)
        fileVersion = 701;
    else if (buildVersion < 2672) // 7.0 SR2 (2664-2671)
        fileVersion = 702;
    else if (buildVersion < 2673) // 7.0 SR3 (2672-2672)
        fileVersion = 703;
    else if (buildVersion < 2766) // 7.0 SR4 (2673-2765)
        fileVersion = 704;
    else if (buildVersion < 2878) // 7.5 (2766-2877)
        fileVersion = 750;
    else if (buildVersion < 2881) // 8.0 SR0 (2878-2880)
        fileVersion = 800;
    else if (buildVersion < 2892) // 8.0 SR1,SR2,SR3 (2878-2891)
        fileVersion = 801;
    else if (buildVersion < 2944) // 8.0 SR4, 8.1 SR1-SR4 (2891-2943)
        fileVersion = 810;
    else if (buildVersion < 2947) // 8.5 SR0, SR1 (2944-2946)
        fileVersion = 850;
    else if (buildVersion < 2962) // 8.5.1 SR0, SR1, SR2
        fileVersion = 851;
    else if (buildVersion < 2980) // 8.6 SR0, SR1, SR2, SR3
        fileVersion = 860;
    else if (buildVersion < 3025) // 9.0 SR0, SR1, SR2
        fileVersion = 900;
    else if (buildVersion < 3078) // 9.1 SR0, SR1, SR2, SR3
        fileVersion = 910;
    else if (buildVersion < 3117) { // 2015 (9.2) SR0, SR1, SR2
        fileVersion = 920;
        newFileVersion = 20150;
    } else if (buildVersion < 3169) { // 2016 (9.3.0) SR0
        fileVersion = 930;
        newFileVersion = 20160;
    } else if (buildVersion < 3172) { // 2016.1 (9.3.1), 2016.2 (9.3.2) SR1, SR2
        fileVersion = 931;
        newFileVersion = 20161;
    } else if (buildVersion < 3225) { // 2017.0 (9.4.0.220) SR0 3224
        fileVersion = 940;
        newFileVersion = 20170;
    } else if (buildVersion < 3228) { // 2017.1 (9.4.1.354), 2017.2 (9.4.2.380) SR1, SR2 3227
        fileVersion = 941;
        newFileVersion = 20171;
    } else if (buildVersion < 3269) { // 2018.0 (9.5.0.193), 2018.1 (9.5.1.195) SR0, SR1 3268
        fileVersion = 950;
        newFileVersion = 20180;
    } else if (buildVersion < 3296) { // 2018b.0 (9.5.5.409) SR0, SR1 3295
        fileVersion = 955;
        newFileVersion = 20185;
    } else if (buildVersion < 3331) { // 2019.0 (9.6.0.172) SR0 3330
        fileVersion = 960;
        newFileVersion = 20190;
    } else if (buildVersion < 3360) { // 2019b.0 (9.6.5.169) SR0 3359
        fileVersion = 965;
        newFileVersion = 20195;
    } else if (buildVersion < 3381) { // 2020.0 (9.7.0.185), 2020.1 (9.7.0.188) SR0, SR1 3380
        fileVersion = 970;
        newFileVersion = 20200;
    } else if (buildVersion < 3426) { // 2020b.0 (9.7.5.184) SR0 3425
        fileVersion = 975;
        newFileVersion = 20205;
    } else if (buildVersion < 3446) { // 2021.0 (9.8.0.200) SR0 3445
        fileVersion = 980;
        newFileVersion = 20210;
    } else if (buildVersion < 3479) { // 2021b.0 (9.8.5.201), 2021b.1 (9.8.5.204), 2021b.2
                                      // (9.8.5.212) SR0, SR1, SR2 3478
        fileVersion = 985;
        newFileVersion = 20215;
    } else {
        // > 2021bSR2
        fileVersion = 986;
        newFileVersion = 20216;
        LOG_PRINT(logfile, "Found project version 2021bSR3 (9.8.6) or newer\n")
    }

    if (newFileVersion == 0) {
        LOG_PRINT(logfile, "Found project version %.2f\n", fileVersion / 100.0)
    } else if (fileVersion < 941) {
        LOG_PRINT(logfile, "Found project version %.1f (%.2f)\n", newFileVersion / 10.0,
                  fileVersion / 100.0)
    }

    // Close logfile, will be reopened in parser routine.
    // There are ways to keep logfile open and pass it to parser routine,
    // but I choose to do the same as with 'file', close it and reopen in 'parse'
    // routines.
    parser.reset(createOriginAnyParser(fileName));
    ioError = 0;

#ifdef GENERATE_CODE_FOR_LOG
    fclose(logfile);
#endif
}

bool OriginFile::parse()
{
    if (ioError != 0)
        return false;
    parser->buildVersion = buildVersion;
    parser->fileVersion = fileVersion;
    return parser->parse();
}

double OriginFile::version() const
{
    return (parser->fileVersion) / 100.0;
}

const tree<Origin::ProjectNode> *OriginFile::project() const
{
    return &parser->projectTree;
}

vector<Origin::SpreadColumn>::size_type OriginFile::datasetCount() const
{
    return parser->datasets.size();
}

Origin::SpreadColumn &OriginFile::dataset(vector<Origin::SpreadColumn>::size_type ds) const
{
    return parser->datasets[ds];
}

vector<Origin::SpreadSheet>::size_type OriginFile::spreadCount() const
{
    return parser->spreadSheets.size();
}

Origin::SpreadSheet &OriginFile::spread(vector<Origin::SpreadSheet>::size_type s) const
{
    return parser->spreadSheets[s];
}

vector<Origin::Matrix>::size_type OriginFile::matrixCount() const
{
    return parser->matrixes.size();
}

Origin::Matrix &OriginFile::matrix(vector<Origin::Matrix>::size_type m) const
{
    return parser->matrixes[m];
}

vector<Origin::Function>::size_type OriginFile::functionCount() const
{
    return parser->functions.size();
}

vector<Origin::Function>::difference_type OriginFile::functionIndex(const string &name) const
{
    return parser->findFunctionByName(name);
}

Origin::Function &OriginFile::function(vector<Origin::Function>::size_type f) const
{
    return parser->functions[f];
}

vector<Origin::Graph>::size_type OriginFile::graphCount() const
{
    return parser->graphs.size();
}

Origin::Graph &OriginFile::graph(vector<Origin::Graph>::size_type g) const
{
    return parser->graphs[g];
}

vector<Origin::Note>::size_type OriginFile::noteCount() const
{
    return parser->notes.size();
}

Origin::Note &OriginFile::note(vector<Origin::Note>::size_type n) const
{
    return parser->notes[n];
}

vector<Origin::Excel>::size_type OriginFile::excelCount() const
{
    return parser->excels.size();
}

Origin::Excel &OriginFile::excel(vector<Origin::Excel>::size_type e) const
{
    return parser->excels[e];
}

string OriginFile::resultsLogString() const
{
    return parser->resultsLog;
}

string liboriginVersionString()
{
    return LIBORIGIN_VERSION_STRING;
}

unsigned int liboriginVersion()
{
    return LIBORIGIN_VERSION;
}

unsigned int liboriginVersionMajor()
{
    return LIBORIGIN_VERSION_MAJOR;
}

unsigned int liboriginVersionMinor()
{
    return LIBORIGIN_VERSION_MINOR;
}

unsigned int liboriginVersionPatch()
{
    return LIBORIGIN_VERSION_PATCH;
}
