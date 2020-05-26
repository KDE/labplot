/***************************************************************************
File                 : NgspiceRawNgspiceRawAsciiFilterPrivate.h
Project              : LabPlot
Description          : Ngspice RAW ASCII filter
--------------------------------------------------------------------
Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)
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

#ifndef NGSPICERAWASCIIFILTERPRIVATE_H
#define NGSPICERAWASCIIFILTERPRIVATE_H

class AbstractDataSource;

class NgspiceRawAsciiFilterPrivate {

public:
	explicit NgspiceRawAsciiFilterPrivate(NgspiceRawAsciiFilter*);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);

	const NgspiceRawAsciiFilter* q;

	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};

private:
	std::vector<void*> m_dataContainer; // pointers to the actual data containers
};

#endif
