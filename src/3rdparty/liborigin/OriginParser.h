/*
    File                 : OriginParser.h
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008 Alex Kargovsky (kargovsky@yumr.phys.msu.su)
    SPDX-FileCopyrightText: 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Origin file parser base class
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ORIGIN_PARSER_H
#define ORIGIN_PARSER_H

#include "OriginObj.h"
#include "tree.hh"

#ifdef GENERATE_CODE_FOR_LOG
#define LOG_PRINT( logfile, ... ) { fprintf(logfile, __VA_ARGS__); }
#else // !GENERATE_CODE_FOR_LOG
#define LOG_PRINT( logfile, ... ) {};
#endif

class OriginParser
{
public:
	virtual ~OriginParser() = default;
	virtual bool parse() = 0;

	vector<Origin::SpreadSheet>::difference_type findSpreadByName(const string& name) const;
	vector<Origin::Matrix>::difference_type findMatrixByName(const string& name) const;
	vector<Origin::Function>::difference_type findFunctionByName(const string& name) const;
	vector<Origin::Excel>::difference_type findExcelByName(const string& name) const;

protected:
	vector<Origin::SpreadColumn>::difference_type findSpreadColumnByName(vector<Origin::SpreadSheet>::size_type spread, const string& name) const;
	vector<Origin::SpreadColumn>::difference_type findExcelColumnByName(vector<Origin::Excel>::size_type excel, vector<Origin::SpreadSheet>::size_type sheet, const string& name) const;
	pair<string, string> findDataByIndex(unsigned int index) const;
	pair<Origin::ProjectNode::NodeType, string> findObjectByIndex(unsigned int index) const;
	pair<Origin::ProjectNode::NodeType, Origin::Window> findWindowObjectByIndex(unsigned int index) const;
	void convertSpreadToExcel(vector<Origin::SpreadSheet>::size_type spread);

	int findColumnByName(int spread, const string& name);
private:
	bool iequals(const string&, const string&, const std::locale& = std::locale()) const;

public:
	vector<Origin::SpreadColumn> datasets;
	vector<Origin::SpreadSheet> spreadSheets;
	vector<Origin::Matrix> matrixes;
	vector<Origin::Excel> excels;
	vector<Origin::Function> functions;
	vector<Origin::Graph> graphs;
	vector<Origin::Note> notes;
	tree<Origin::ProjectNode> projectTree;
	string resultsLog;
	unsigned int windowsCount;
	unsigned int fileVersion, buildVersion;
};

OriginParser* createOriginAnyParser(const string& fileName);

#endif // ORIGIN_PARSER_H
