/***************************************************************************
    File                 : AbstractFileFilter.h
    Project              : LabPlot/SciDAVis
    Description          : file I/O-filter related interface for plugins
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke*web.de)
    					   (use @ for *)
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

#ifndef ABSTRACTFILEFILTER_H
#define ABSTRACTFILEFILTER_H

// #include <QtPlugin>
#include "backend/lib/XmlStreamReader.h"
#include <QXmlStreamWriter>

class AbstractDataSource;

class AbstractFileFilter : public QObject {
	Q_OBJECT
// 	Q_INTERFACES(AbstractFileFilter)

	public:
		AbstractFileFilter() {};
		virtual ~AbstractFileFilter() {}
		enum ImportMode {Append, Prepend, Replace};
		
		virtual void read(const QString& fileName, AbstractDataSource* dataSource, ImportMode mode = Replace) = 0;
		virtual void write(const QString& fileName, AbstractDataSource* dataSource) = 0;

		virtual void loadFilterSettings(const QString& filterName) = 0;
		virtual void saveFilterSettings(const QString& filterName) const = 0;

		virtual void save(QXmlStreamWriter*) const = 0;
		virtual bool load(XmlStreamReader*) = 0;

	signals:
		void completed(int) const; //!< int ranging from 0 to 100 notifies about the status of a read/write process		
};

// Q_DECLARE_INTERFACE(AbstractFileFilter, "net.sf.scidavis.datasources.abstractfilefilter/0.1")

#endif
