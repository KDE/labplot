/***************************************************************************
    File                 : DatasetModel.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)
	Description          :  Wrapper class for datasets, and also for their categories and subcategories
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
#ifndef DATASETMODEL_H
#define DATASETMODEL_H

#include <QObject>
#include <QMap>
#include <QVariant>

class ImportDatasetWidget;
class DatasetHandler;
class Spreadsheet;

class DatasetModel : public QObject {
    Q_OBJECT

public:
	DatasetModel(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
    ~DatasetModel();

	QStringList collections();
	QStringList categories(const QString&);
	QStringList subcategories(const QString&, const QString&);
	QStringList datasets(const QString&, const QString&, const QString&);
	int datasetCount(const QString& collection);
	int datasetCount(const QString& collection, const QString& category);
	int datasetCount(const QString& collection, const QString& category, const QString& subcategory);

	Q_INVOKABLE QVariant allCategories();
	Q_INVOKABLE QVariant allSubcategories(const QString&);
	Q_INVOKABLE QVariant allDatasets(const QString&, const QString&);
	Q_INVOKABLE QVariant allDatasetsList();

private:
	QStringList m_collectionList;
	QStringList m_allCategories;
	QMap<QString, QStringList> m_allSubcategories;
	QMap<QString, QMap<QString, QStringList>> m_allDatasets;
	QMap<QString, QStringList> m_categories;
	QMap<QString, QMap<QString, QStringList>> m_subcategories;
	QMap<QString,QMap<QString, QMap<QString, QStringList>>> m_datasets;
	QStringList m_datasetList;

	void initCollections(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
	void initCategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
	void initSubcategories(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
	void initDatasets(const QMap<QString, QMap<QString, QMap<QString, QVector<QString>>>>& datasetMap);
};

#endif //DATASETMODEL_H
