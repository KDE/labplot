/***************************************************************************
File                 : DatasetMetadataManagerWidget.h
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

class DatasetModel;

class DatasetMetadataManagerWidget : public QWidget {
    Q_OBJECT

public:
	explicit DatasetMetadataManagerWidget(QWidget*, const QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
    virtual ~DatasetMetadataManagerWidget() override;
    bool checkDataValidity();
	void updateDocument(const QString& fileName);
    QString getMetadataFilePath() const;

private:
	Ui::DatasetMetadataManagerWidget ui;
	DatasetModel* m_datasetModel;
    QStringList m_columnDescriptions;
	QString m_metadataFilePath;
	QString m_baseColor;
	QString m_textColor;

    void initCategories(const QMap<QString, QMap<QString, QVector<QString>>>&);
    void initSubcategories(const QMap<QString, QMap<QString, QVector<QString>>>&);
    void initDatasets(const QMap<QString, QMap<QString, QVector<QString>>>&);
    bool checkFileName();
    bool urlExists();
    bool checkDatasetName();
    bool checkDescription();
    bool checkCategories(QComboBox*);
    void loadSettings();
    void enableDatasetSettings(bool);
	QJsonObject createDatasetObject();

private slots:
	void updateCategories(const QString&);
    void updateSubcategories(const QString&);
    void addColumnDescription();
    void removeColumnDescription();

signals:
    void checkOk();
};

#endif // DATASETMETADATAMANAGERWIDGET_H
