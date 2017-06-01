/***************************************************************************
    File                 : opj2dat.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Stefan Gerlach
                           (C) 2017 Miquel Garriga
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : Origin project converter

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 3 of the License, or      *
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

#include "OriginFile.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;


int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Usage : ./opj2dat [--check-only] <file.opj>" << endl;
		return -1;
	}

	cout << "opj2dat " << LIBORIGIN_VERSION_STRING << ", Copyright (C) 2008 Stefan Gerlach, 2017 Miquel Garriga" << endl;

	if (string(argv[1]) == "-v")
		return 0;

	bool write_spreads = true;
	if ((argc > 2) && (string(argv[1]) == "--check-only")) write_spreads = false;

	string inputfile=argv[argc-1];
	OriginFile opj(inputfile);
	int status = opj.parse();
	cout << "Parsing status = " << status << endl;
	cout << "OPJ PROJECT \"" << inputfile.c_str() << "\" VERSION = " <<  opj.version() << endl;

	cout << "number of datasets     = " << opj.datasetCount() << endl;
	cout << "number of spreadsheets = " << opj.spreadCount() << endl;
	cout << "number of matrixes     = " << opj.matrixCount() << endl;
	cout << "number of excels       = " << opj.excelCount() << endl;
	cout << "number of functions    = " << opj.functionCount() << endl;
	cout << "number of graphs       = " << opj.graphCount() << endl;
	cout << "number of notes        = " << opj.noteCount() << endl;

	for (unsigned int s=0;s<opj.spreadCount();s++) {
		Origin::SpreadSheet spread = opj.spread(s);
		int columnCount=spread.columns.size();
		cout << "Spreadsheet " << (s+1) << endl;
		cout << " Name: " << spread.name.c_str() << endl;
		cout << " Label: " << spread.label.c_str() << endl;
		cout << "	Columns: " << columnCount << endl;
		for (int j=0;j<columnCount;j++) {
			Origin::SpreadColumn column = spread.columns[j];
			cout << "	Column " << (j+1) << " : " << column.name.c_str() << " / type : " << column.type << ", rows : " << spread.maxRows << endl;
		}

		if (write_spreads) {
			ostringstream sfilename;
			sfilename << inputfile.c_str() << "." << (s+1) << ".dat";
			cout << "saved to " << sfilename.str() << endl;

			ofstream outf;
			outf.open(sfilename.str().c_str(), ios_base::out);
			if (!outf.good()) {
				cout << "Could not open " << sfilename.str() << endl;
				return -1;
			}
			// header
			for (int j=0;j<columnCount;j++) {
				outf << spread.columns[j].name.c_str() << "; ";
				cout << spread.columns[j].name.c_str();
			}
			outf << endl;
			cout << endl << " Data: " << endl;
			// data
			for (int i=0;i<(int)spread.maxRows;i++) {
				for (int j=0;j<columnCount;j++) {
					if (i<(int)spread.columns[j].data.size()) {
						Origin::variant value=spread.columns[j].data[i];
						double v=0.;
						if (value.type() == typeid(double)) {
							v = boost::get<double>(value);
							if (v != _ONAN) {
								outf << v << "; ";
							} else {
								outf << nan("NaN") << "; ";
							}
						}
						if (value.type() == typeid(string)) {
							outf << boost::get<string>(value).c_str() << "; ";
						}
					} else {
						outf << "; ";
					}
				}
			outf << endl;
			}
			outf.close();
		}
	}
	return 0;
}

