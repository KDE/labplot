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

using namespace Origin;
using namespace endianfstream;

class OriginAnyParser : public OriginParser
{
public:
    explicit OriginAnyParser(const std::string &fileName);
    bool parse() override;

protected:
    unsigned int readObjectSize();
    std::string readObjectAsString(unsigned int);
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
    bool getColumnInfoAndData(const std::string &, unsigned int, const std::string &, unsigned int);
    void getMatrixValues(const std::string &, unsigned int, short, char, char,
                         std::vector<Origin::Matrix>::difference_type);
    void getWindowProperties(Origin::Window &, const std::string &, unsigned int);
    void getLayerProperties(const std::string &, unsigned int);
    Origin::Color getColor(const std::string &);
    void getAnnotationProperties(const std::string &, unsigned int, const std::string &,
                                 unsigned int, const std::string &, unsigned int,
                                 const std::string &, unsigned int);
    void getCurveProperties(const std::string &, unsigned int, const std::string &, unsigned int);
    void getAxisBreakProperties(const std::string &, unsigned int);
    void getAxisParameterProperties(const std::string &, unsigned int, int);
    void getNoteProperties(const std::string &, unsigned int, const std::string &, unsigned int,
                           const std::string &, unsigned int);
    void getColorMap(ColorMap &, const std::string &, unsigned int);
    void getZcolorsMap(ColorMap &, const std::string &, unsigned int);
    void getProjectLeafProperties(tree<ProjectNode>::iterator, const std::string &, unsigned int);
    void getProjectFolderProperties(tree<ProjectNode>::iterator, const std::string &, unsigned int);
    void outputProjectTree(std::ostream &);

    inline time_t doubleToPosixTime(double jdt)
    {
        /* 2440587.5 is julian date for the unixtime epoch */
        return (time_t)floor((jdt - 2440587) * 86400. + 0.5);
    }

    iendianfstream file;
    FILE *logfile;

    std::streamsize d_file_size;
    std::streamoff curpos;
    unsigned int objectIndex, parseError;
    std::vector<Origin::SpreadSheet>::difference_type ispread;
    std::vector<Origin::Matrix>::difference_type imatrix;
    std::vector<Origin::Excel>::difference_type iexcel;
    int igraph, ilayer;
};

#endif // ORIGIN_ANY_PARSER_H
