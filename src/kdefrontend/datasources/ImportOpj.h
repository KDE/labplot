/***************************************************************************
    File                 : ImportOpj.h
    Project              : LabPlot
    Description          : import Origin project
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef IMPORTOPJ_H
#define IMPORTOPJ_H

#include <liborigin/OriginFile.h>

class MainWin;
class QString;

class ImportOpj {

public:
	explicit ImportOpj(MainWin* parent, const QString& filename);
	~ImportOpj() {};

private:
	int importTables(const OriginFile &opj);
	int importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread);
	int importMatrix(const OriginFile &opj, const Origin::Matrix &matrix);
	int importNotes(const OriginFile &opj);
	int importGraphs(const OriginFile &opj);
	QString parseOriginText(const QString &str);
	QString parseOriginTags(const QString &str);
	MainWin *mw;
};

#endif //IMPORTOPJ_H
