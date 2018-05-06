/*
 * OriginAnyParser.h
 *
 * Copyright 2017 Miquel Garriga <gbmiquel@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Parser for all versions. Based mainly on Origin750Parser.h
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
	explicit OriginAnyParser(const string& fileName);
	bool parse();

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
	bool getColumnInfoAndData(const string&, unsigned int, const string&, unsigned int);
	void getMatrixValues(const string&, unsigned int, short, char, char, vector<Origin::Matrix>::difference_type);
	void getWindowProperties(Origin::Window&, const string&, unsigned int);
	void getLayerProperties(const string&, unsigned int);
	Origin::Color getColor(const string&);
	void getAnnotationProperties(const string&, unsigned int, const string&, unsigned int, const string&, unsigned int, const string&, unsigned int);
	void getCurveProperties(const string&, unsigned int, const string&, unsigned int);
	void getAxisBreakProperties(const string&, unsigned int);
	void getAxisParameterProperties(const string&, unsigned int, int);
	void getNoteProperties(const string&, unsigned int, const string&, unsigned int, const string&, unsigned int);
	void getColorMap(ColorMap&, const string&, unsigned int);
	void getZcolorsMap(ColorMap&, const string&, unsigned int);
	void getProjectLeafProperties(tree<ProjectNode>::iterator, const string&, unsigned int);
	void getProjectFolderProperties(tree<ProjectNode>::iterator, const string&, unsigned int);
	void outputProjectTree();

	inline time_t doubleToPosixTime(double jdt)
	{
		/* 2440587.5 is julian date for the unixtime epoch */
		return (time_t) floor((jdt - 2440587) * 86400. + 0.5);
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
