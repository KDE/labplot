/***************************************************************************
File                 : HDFFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for HDFFilter.
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef HDFFILTERPRIVATE_H
#define HDFFILTERPRIVATE_H

#ifdef HAVE_HDF5
#include <hdf5.h>
#endif

class AbstractDataSource;

class HDFFilterPrivate {

	public:
		explicit HDFFilterPrivate(HDFFilter*);

		void parse(const QString & fileName, QTreeWidgetItem* rootItem);
		void read(const QString & fileName, AbstractDataSource* dataSource,
					AbstractFileFilter::ImportMode importMode = AbstractFileFilter::Replace);
		void write(const QString & fileName, AbstractDataSource* dataSource);

		const HDFFilter* q;

		bool autoModeEnabled;

	private:
		void clearDataSource(AbstractDataSource*) const;
#ifdef HAVE_HDF5
		void scanHDFGroup(hid_t gid, QTreeWidgetItem* parentItem);
#endif
};

#endif
