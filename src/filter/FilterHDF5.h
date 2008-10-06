/***************************************************************************
    File                 : FilterHDF5.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : HDF5 import/export filter
                           
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

#ifndef FILTERHDF5_H
#define FILTERHDF5_H

#include <vector>
#include <QString>
#include <QStringList>
#include "../MainWin.h"

#ifdef HAVE_HDF5
#include <hdf5.h>
#endif

class FilterHDF5
{
public:
	FilterHDF5(QString filename=0);
	bool fileOK() const { return fileok; }
	void importFile();
	int exportFile(MainWin *mw, bool all, int datatype, int order, int start, int end);
#ifdef HAVE_HDF5
	int numGroups() const;
	int numSets() const;
	int numAttributes() const;
	int numSetAttributes(int set) const;
	// QString version() { return QString(nc_inq_libvers());}
	QString groupName(int set) const;
	QString datasetName(int set) const;
	QString columnName(int set, int col) const;
	double Data(int set, int row, int col) const;	       //!< get data of column c and rows r of spreadsheet s
	int Rows(int set) const;
	int Cols(int set) const;
	QString getAttribute(int i) const;
	QString getSetAttribute(int set,int i) const;
#endif
protected:
private:
	bool fileok;			//!< is a hdf5 file and is readable
	QString filename;
};

#endif //FILTERHDF5_H
