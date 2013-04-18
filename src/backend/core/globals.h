/***************************************************************************
    File                 : globals.h
    Project              : SciDAVis
    Description          : Definition of global constants and enums
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email addresses) 

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

#ifndef SCIDAVIS_GLOBALS_H
#define SCIDAVIS_GLOBALS_H

#include <QObject>
#include <QString>

class SciDAVis : public QObject
{
	Q_OBJECT

	private:
		SciDAVis() {} // don't allow instanciating

	public:
		virtual ~SciDAVis() {} // avoid the warning message

		static int version();

		static QString versionString();

		static QString extraVersion();

		static QString copyrightString();

		static QString releaseDateString();

		static void about();

		static const QString appName;

	private:
		static const int scidavis_version;
		static const char * extra_version;
		static const char * copyright_string;
		static const char * release_date;
};

#endif

