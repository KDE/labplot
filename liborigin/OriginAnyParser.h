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
	OriginAnyParser(const string& fileName);
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
	bool getColumnInfoAndData(string, unsigned int, string, unsigned int);
	void getMatrixValues(string, unsigned int, short, char, char, int);
	void getWindowProperties(Origin::Window&, string, unsigned int);
	void getLayerProperties(string, unsigned int);
	Origin::Color getColor(string);
	void getAnnotationProperties(string, unsigned int, string, unsigned int, string, unsigned int, string, unsigned int);
	void getCurveProperties(string, unsigned int, string, unsigned int);
	void getAxisBreakProperties(string, unsigned int);
	void getAxisParameterProperties(string, unsigned int, int);
	void getNoteProperties(string, unsigned int, string, unsigned int, string, unsigned int);
	void getColorMap(ColorMap&, string, unsigned int);
	void getZcolorsMap(ColorMap&, string, unsigned int);
	void getProjectLeafProperties(tree<ProjectNode>::iterator, string, unsigned int);
	void getProjectFolderProperties(tree<ProjectNode>::iterator, string, unsigned int);
	void outputProjectTree();

	inline time_t doubleToPosixTime(double jdt)
	{
		/* 2440587.5 is julian date for the unixtime epoch */
		return (time_t) floor((jdt - 2440587) * 86400. + 0.5);
	}

	iendianfstream file;
	FILE *logfile;

	unsigned long d_file_size;
	unsigned int objectIndex;
	int ispread, imatrix, iexcel, igraph;
	int ilayer;
};

#endif // ORIGIN_ANY_PARSER_H
