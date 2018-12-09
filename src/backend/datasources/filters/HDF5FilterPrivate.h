/***************************************************************************
File                 : HDF5FilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for HDF5Filter.
--------------------------------------------------------------------
Copyright            : (C) 2015-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef HDF5FILTERPRIVATE_H
#define HDF5FILTERPRIVATE_H

#include <QList>
#ifdef HAVE_HDF5
#include <hdf5.h>
#endif

class AbstractDataSource;

class HDF5FilterPrivate {

public:
	explicit HDF5FilterPrivate(HDF5Filter*);
	
#ifdef HAVE_HDF5
	static void handleError(int err, const QString& function, const QString& arg = QString());
#endif

	void parse(const QString & fileName, QTreeWidgetItem* rootItem);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::Replace);
	QVector<QStringList> readCurrentDataSet(const QString& fileName, AbstractDataSource*, bool &ok,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);

	const HDF5Filter* q;

	QString currentDataSetName{""};
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

private:
#ifdef HAVE_HDF5
	int m_status;
#endif
	const static int MAXNAMELENGTH = 1024;
	const static int MAXSTRINGLENGTH = 1024*1024;
	QList<unsigned long> m_multiLinkList;	// used to find hard links

#ifdef HAVE_HDF5
	QString translateHDF5Order(H5T_order_t);
	QString translateHDF5Type(hid_t);
	QString translateHDF5Class(H5T_class_t);
	QStringList readHDF5Compound(hid_t tid);
	template <typename T> QStringList readHDF5Data1D(hid_t dataset, hid_t type, int rows, int lines,
							void* dataPointer = nullptr);
	QStringList readHDF5CompoundData1D(hid_t dataset, hid_t tid, int rows, int lines, QVector<void*>& dataPointer);
	template <typename T> QVector<QStringList> readHDF5Data2D(hid_t dataset, hid_t ctype, int rows, int cols, int lines,
								 QVector<void*>& dataPointer);
	QVector<QStringList> readHDF5CompoundData2D(hid_t dataset, hid_t tid, int rows, int cols, int lines);
	QStringList readHDF5Attr(hid_t aid);
	QStringList scanHDF5Attrs(hid_t oid);
	QStringList readHDF5DataType(hid_t tid);
	QStringList readHDF5PropertyList(hid_t pid);
	void scanHDF5DataType(hid_t tid, char* dataTypeName,  QTreeWidgetItem* parentItem);
	void scanHDF5Link(hid_t gid, char* linkName,  QTreeWidgetItem* parentItem);
	void scanHDF5DataSet(hid_t dsid, char* dataSetName,  QTreeWidgetItem* parentItem);
	void scanHDF5Group(hid_t gid, char* groupName, QTreeWidgetItem* parentItem);
#endif
};

#endif
