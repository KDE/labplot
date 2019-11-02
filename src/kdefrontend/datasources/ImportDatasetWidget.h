/***************************************************************************
	File                 : ImportDatasetWidget.h
	Project              : LabPlot
	Description          : import online dataset widget
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)
	Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)
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


#ifndef IMPORTDATASETWIDGET_H
#define IMPORTDATASETWIDGET_H

#include "ui_importdatasetwidget.h"
#include "QWidget"
#include "QMap"
#include <QJsonArray>
#include <QJsonObject>

class QCompleter;
class DatasetHandler;
class DatasetModel;

typedef QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>> DatasetsMap;

class ImportDatasetWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportDatasetWidget(QWidget* parent);
	~ImportDatasetWidget() override;

	QString getSelectedDataset() const;
	void loadDatasetToProcess(DatasetHandler*);
	const DatasetsMap& getDatasetsMap();
	void setCollection(const QString&);
	void setCategory(const QString&);
	void setSubcategory(const QString&);
	void setDataset(const QString&);
	void processTest(const QString& category, const QString& subcategory, const QString& dataset, DatasetHandler*);

private:
	Ui::ImportDatasetWidget ui;
	DatasetsMap m_datasetsMap;
	QString m_selectedSubcategory;
	QCompleter* m_categoryCompleter;
	QCompleter* m_datasetCompleter;
	QString m_jsonDir;
	bool m_loadingCategories{false};
	QString m_selectedCategory;
	QString m_selectedCollection;
	DatasetModel* m_datasetModel{nullptr};

	QJsonArray m_collections;
	QJsonObject m_datasetObject;

	QString validCollectionName(const QString&);
// 	void downloadCollectionsFile();
// 	void downloadCollectionFile(const QString&);
// 	void uploadCategoryFile();
// 	void uploadDatasetFile(const QString&);
	void updateDatasetCompleter();
	void updateCategoryCompleter();
	void loadCategories();
	void listDatasetsForSubcategory(QTreeWidgetItem*);
	void restoreSelectedSubcategory(const QString&);
// 	void highlightLocalMetadataFiles();
	QJsonObject loadDatasetObject();

private slots:
	void datasetChanged();
	void updateCategoryTree(const QString&);
	void scrollToCategoryTreeItem(const QString&);
	void scrollToDatasetListItem(const QString&);
	void showDatasetMetadataManager();
// 	void refreshCategories();
// 	void clearCache();
// 	void restoreBackup();

signals:
	void datasetSelected();
	void datasetDoubleClicked();
};

#endif // IMPORTDATASETWIDGET_H
