/***************************************************************************
File                 : DatasetMetadataManagerWidget.cpp
Project              : LabPlot
Description          : widget for managing a metadata file of a dataset
--------------------------------------------------------------------
Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)

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

#ifndef DATASETMETADATAMANAGERWIDGET_H
#define DATASETMETADATAMANAGERWIDGET_H

#include "ui_datasetmetadatamanagerwidget.h"


class DatasetMetadataManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit DatasetMetadataManagerWidget(QWidget*, const QMap<QString, QMap<QString, QVector<QString>>>&);
    ~DatasetMetadataManagerWidget() override;
    bool checkDataValidity();
    void updateDocument(const QString& fileName);
    void createNewMetadata(const QString& dirPath);

private:
    Ui::DatasetMetadataManagerWidget ui;
    QStringList m_categoryList;
    QMap<QString, QStringList> m_subcategoryMap;
    QMap<QString, QStringList> m_datasetMap;
	QStringList m_columnDescriptions;

    void initCategories(const QMap<QString, QMap<QString, QVector<QString>>>&);
    void initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>&);
    void initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>&);
    bool checkFileName();
	bool urlExists();

private slots:
    void updateSubcategories(const QString&);
	void addColumnDescription();

signals:
    void checkOk();
};

#endif // DATASETMETADATAMANAGERWIDGET_H
