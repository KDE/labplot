/***************************************************************************
    File                 : ImportDatasetWidget.h
    Project              : LabPlot
    Description          : import online dataset widget
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)

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
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "QMap"
#include "QCompleter"

class DatasetHandler;

class ImportDatasetWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImportDatasetWidget(QWidget* parent);
    ~ImportDatasetWidget() override;
    AbstractFileFilter* currentFileFilter() const ;
	QString getSelectedDataset();
	void loadDatasetToProcess(DatasetHandler* datasetHandler);


private:    
    Ui::ImportDatasetWidget ui;
    mutable std::unique_ptr<AbstractFileFilter> m_currentFilter;
	QMap<QString, QMap<QString, QVector<QString>>> m_datasetsMap;
	QString m_selectedSubcategory;
	QCompleter* m_categoryCompleter;
	QCompleter* m_datasetCompleter;

	void updateDatasetCompleter();
	void updateCategoryCompleter();
    void loadDatasetCategoriesFromJson();
	void listDatasetsForSubcategory(QTreeWidgetItem* item, int column);

private slots:
	void scrollToCategoryTreeItem(const QString& rootName);
	void scrollToDatasetListItem(const QString& rootName);

signals:
	void datasetSelected();

};


#endif // IMPORTDATASETWIDGET_H
