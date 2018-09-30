/***************************************************************************
    File                 : OriginParser.h
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Alex Kargovsky (kargovsky@yumr.phys.msu.su)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Origin file parser base class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef ORIGIN_PARSER_H
#define ORIGIN_PARSER_H

#include "OriginObj.h"
#include "tree.hh"

#ifdef GENERATE_CODE_FOR_LOG

#ifdef HAVE_WINDOWS
#define LOG_PRINT( logfile, ... ) { fprintf(logfile, __VA_ARGS__); }
#else	// NOT WINDOWS

#ifdef NDEBUG
#define LOG_PRINT( logfile, args... ) { fprintf(logfile, args); }
#else
#define LOG_PRINT( logfile, args... ) { int ioret = fprintf(logfile, args); assert(ioret > 0); }
#endif

#endif

#else // !GENERATE_CODE_FOR_LOG
#define LOG_PRINT( logfile, args, ... ) {};
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
