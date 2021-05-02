/***************************************************************************
File                 : MatioFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for MatioFilter.
--------------------------------------------------------------------
Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef MATIOFILTERPRIVATE_H
#define MATIOFILTERPRIVATE_H

#ifdef HAVE_MATIO
#include <matio.h>
#endif

class AbstractDataSource;

class MatioFilterPrivate {

public:
	explicit MatioFilterPrivate(MatioFilter*);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void parse(const QString & fileName);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);

//helper functions
#ifdef HAVE_MATIO
	static QString className(matio_classes classType);
	static QString typeName(matio_types dataType);
#endif

	const MatioFilter* q;

	size_t varCount;
	QString currentVarName;
	QVector<QStringList> varsInfo;
	int startRow{-1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

private:
#ifdef HAVE_MATIO
	int m_status;
#endif
};

#endif
