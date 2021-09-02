/*
File                 : DatasetMetadataManagerWidget.h
Project              : LabPlot
Description          : widget for managing a metadata file of a dataset
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2019 Ferencz Kovacs <kferike98@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/


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

	void setCollection(const QString&);
	void setCategory(const QString&);
	void setSubcategory(const QString&);
	void setShortName(const QString&);
	void setFullName(const QString&);
	void setDescription(const QString&);
	void setURL(const QString&);

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
