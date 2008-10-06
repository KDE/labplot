/***************************************************************************
    File                 : Project.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : LabPlot project class
                           
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

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>

#include "definitions.h"

class Project
{
public:
	Project();
	QDomElement save(QDomDocument doc);
	void open(QDomNode node);
	ACCESS(QString, filename, Filename);
	ACCESS(int, version, Version);
	ACCESS(QString, labPlot, LabPlot);
	ACCESS(QString, title, Title);
	ACCESS(QString, author, Author);
	ACCESS(QDateTime, created, Created);
	ACCESS(QDateTime, modified, Modified);
	ACCESS(QString, notes, Notes);
	ACCESSFLAG(m_changed, Changed);
private:
	QString m_filename;
	int m_version;
	QString m_labPlot;
	QString m_title;
	QString m_author;
	QDateTime m_created;
	QDateTime m_modified;
	QString m_notes;
	bool m_changed;		//!< set when project was changed
};

#endif //PROJECT_H
