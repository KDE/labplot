/*
    OriginAnyParser.h

    SPDX-FileCopyrightText: 2017 Miquel Garriga <gbmiquel@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later

    Parser for all versions. Based mainly on Origin750Parser.h
*/
#ifndef ORIGIN_ANY_PARSER_H
#define ORIGIN_ANY_PARSER_H

#include "OriginParser.h"
#include "endianfstream.hh"
#include <string>
#include <cmath> // for floor()

using namespace std;
using namespace Origin;

class OriginAnyParser : public OriginParser
{
public:
    explicit OriginAnyParser(const string &fileName);
    bool parse() override;

protected:
    unsigned int readObjectSize();
    string readObjectAsString(unsigned int);
    void readFileVersion();
    void readGlobalHeader();
    bool readDataSetElement();
    bool readWindowElement();
    bool readLayerElement();
    unsigned int readAnnotationList();
    bool readAnnotationElement();
    bool readCurveElement();
    bool readAxisBreakElement();
    bool readAxisParameterElement(unsigned int);
    bool readParameterElement();
    bool readNoteElement();
    void readProjectTree();
    unsigned int readFolderTree(tree<ProjectNode>::iterator, unsigned int);
    void readProjectLeaf(tree<ProjectNode>::iterator);
    void readAttachmentList();
    bool getColumnInfoAndData(const string &, unsigned int, const string &, unsigned int);
    void getMatrixValues(const string &, unsigned int, short, char, char,
                         vector<Origin::Matrix>::difference_type);
    void getWindowProperties(Origin::Window &, const string &, unsigned int);
    void getLayerProperties(const string &, unsigned int);
    Origin::Color getColor(const string &);
    void getAnnotationProperties(const string &, unsigned int, const string &, unsigned int,
                                 const string &, unsigned int, const string &, unsigned int);
    void getCurveProperties(const string &, unsigned int, const string &, unsigned int);
    void getAxisBreakProperties(const string &, unsigned int);
    void getAxisParameterProperties(const string &, unsigned int, int);
    void getNoteProperties(const string &, unsigned int, const string &, unsigned int,
                           const string &, unsigned int);
    void getColorMap(ColorMap &, const string &, unsigned int);
    void getZcolorsMap(ColorMap &, const string &, unsigned int);
    void getProjectLeafProperties(tree<ProjectNode>::iterator, const string &, unsigned int);
    void getProjectFolderProperties(tree<ProjectNode>::iterator, const string &, unsigned int);
    void outputProjectTree(std::ostream &);

    inline time_t doubleToPosixTime(double jdt)
    {
        /* 2440587.5 is julian date for the unixtime epoch */
        return (time_t)floor((jdt - 2440587) * 86400. + 0.5);
    }

    iendianfstream file;
    FILE *logfile;

    streamsize d_file_size;
    streamoff curpos;
    unsigned int objectIndex, parseError;
    vector<Origin::SpreadSheet>::difference_type ispread;
    vector<Origin::Matrix>::difference_type imatrix;
    vector<Origin::Excel>::difference_type iexcel;
    int igraph, ilayer;
};

#endif // ORIGIN_ANY_PARSER_H
