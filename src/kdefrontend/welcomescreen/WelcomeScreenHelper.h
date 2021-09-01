/*
    File                 : WelcomeScreenHelper.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Ferencz Kovacs <kferike98@gmail.com>
    Description          : Helper class for the welcome screen

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WELCOMESCREENHELPER_H
#define WELCOMESCREENHELPER_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <memory>
#include <QPixmap>

class ImportDatasetWidget;
class DatasetHandler;
class Spreadsheet;
class DatasetModel;

class WelcomeScreenHelper : public QObject {
    Q_OBJECT

public:
    WelcomeScreenHelper();
    ~WelcomeScreenHelper();

    Q_INVOKABLE QVariant datasetName();
    Q_INVOKABLE QVariant datasetDescription();
    Q_INVOKABLE QVariant datasetColumns();
    Q_INVOKABLE QVariant datasetRows();
    Q_INVOKABLE QVariant getProjectThumbnail(const QUrl&);
	Q_INVOKABLE QVariant getExampleProjectThumbnail(const QString&);
	Q_INVOKABLE QVariant getExampleProjects();
	Q_INVOKABLE QVariant searchExampleProjects(const QString& searchtext);
	Q_INVOKABLE QVariant getExampleProjectTags(const QString&);
	Q_INVOKABLE void setWidthScale(const QString& sectionID, double scale);
	Q_INVOKABLE void setHeightScale(const QString& sectionID, double scale);
	Q_INVOKABLE QVariant getWidthScale(const QString& sectionID);
	Q_INVOKABLE QVariant getHeightScale(const QString& sectionID);
	Q_INVOKABLE QVariant getMaxIcon();
	Q_INVOKABLE QVariant getMinIcon();
	Q_INVOKABLE QVariant getBackIcon();
	Q_INVOKABLE QVariant getForwardIcon();

    Spreadsheet* releaseConfiguredSpreadsheet();
	DatasetModel* getDatasetModel();

public slots:
	void datasetClicked(const QString& category, const QString& subcategory, const QString& datasetName);
	void exampleProjectClicked(const QString&);

private:
    DatasetModel* m_datasetModel{nullptr};
    ImportDatasetWidget* m_datasetWidget{nullptr};
    DatasetHandler* m_datasetHandler{nullptr};
    mutable std::unique_ptr<Spreadsheet> m_spreadsheet{nullptr};
	QStringList m_projectNameList;
	QMap<QString,QStringList> m_tagMap;
	QMap<QString,QStringList> m_datasetTag;
	QMap<QString, QString> m_pathMap;
	QMap<QString, double> m_widthScale;
	QMap<QString, double> m_heightScale;
	QPixmap m_maxIcon;
	QPixmap m_minIcon;

	void processExampleProjects();
	void loadConfig();

signals:
    void datasetFound();
    void datasetNotFound();
    void showFirstDataset();
	void openExampleProject(QString);
};
#endif //WELCOMESCREENHELPER_H
