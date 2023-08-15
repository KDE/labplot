/*
    File                 : OriginFile.h
    Description          : Origin file import class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2005-2007 Stefan Gerlach
    SPDX-FileCopyrightText: 2007-2008 Alex Kargovsky <kargovsky*yumr.phys.msu.su>
    SPDX-FileCopyrightText: 2007-2008 Ion Vasilief <ion_vasilief*yahoo.fr>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORIGIN_FILE_H
#define ORIGIN_FILE_H

#include "OriginObj.h"
#include "OriginParser.h"

#include <memory>

class ORIGIN_EXPORT OriginFile
{
public:
    explicit OriginFile(const std::string &fileName);

    bool parse(); //!< parse Origin file
    double version() const; //!< get version of Origin file

    std::vector<Origin::SpreadColumn>::size_type datasetCount() const; //!< get number of datasets
    Origin::SpreadColumn &
    dataset(std::vector<Origin::SpreadColumn>::size_type ds) const; //!< get dataset ds

    std::vector<Origin::SpreadSheet>::size_type spreadCount() const; //!< get number of spreadsheets
    Origin::SpreadSheet &
    spread(std::vector<Origin::SpreadSheet>::size_type s) const; //!< get spreadsheet s

    std::vector<Origin::Matrix>::size_type matrixCount() const; //!< get number of matrices
    Origin::Matrix &matrix(std::vector<Origin::Matrix>::size_type m) const; //!< get matrix m

    std::vector<Origin::Function>::size_type functionCount() const; //!< get number of functions
    std::vector<Origin::Function>::difference_type
    functionIndex(const std::string &name) const; //!< get index (or -1) of function named name
    Origin::Function &
    function(std::vector<Origin::Function>::size_type f) const; //!< get function f

    std::vector<Origin::Graph>::size_type graphCount() const; //!< get number of graphs
    Origin::Graph &graph(std::vector<Origin::Graph>::size_type g) const; //!< get graph g

    std::vector<Origin::Note>::size_type noteCount() const; //!< get number of notes
    Origin::Note &note(std::vector<Origin::Note>::size_type n) const; //!< get note n

    std::vector<Origin::Excel>::size_type excelCount() const; //!< get number of excels
    Origin::Excel &excel(std::vector<Origin::Excel>::size_type e) const; //!< get excel e

    const tree<Origin::ProjectNode> *project() const; //!< get project tree
    std::string resultsLogString() const; //!< get Results Log

private:
    unsigned int fileVersion, buildVersion, ioError;
    std::unique_ptr<OriginParser> parser;
};

std::string ORIGIN_EXPORT liboriginVersionString();
unsigned int ORIGIN_EXPORT liboriginVersion();
unsigned int ORIGIN_EXPORT liboriginVersionMajor();
unsigned int ORIGIN_EXPORT liboriginVersionMinor();
unsigned int ORIGIN_EXPORT liboriginVersionBugfix();

#endif // ORIGIN_FILE_H
