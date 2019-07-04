/***************************************************************************
    File                 : WelcomeScreenHelper.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)
    Description          : Helper class for the welcome screen
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
#ifndef WELCOMESCREENHELPER_H
#define WELCOMESCREENHELPER_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <memory>

class ImportDatasetWidget;
class DatasetHandler;
class Spreadsheet;
class DatasetModel;

class WelcomeScreenHelper:  public QObject {
    Q_OBJECT

public:
    WelcomeScreenHelper();
    ~WelcomeScreenHelper();

    Q_INVOKABLE QVariant datasetName();
    Q_INVOKABLE QVariant datasetDescription();
    Q_INVOKABLE QVariant datasetColumns();
    Q_INVOKABLE QVariant datasetRows();
    Q_INVOKABLE QVariant getProjectThumbnail(const QUrl& url);
	Q_INVOKABLE QVariant getExampleProjectThumbnail(const QString&);
	Q_INVOKABLE QVariant getExampleProjects();
	Q_INVOKABLE QVariant searchExampleProjects(const QString& searchtext);
	Q_INVOKABLE QVariant getExampleProjectTags(const QString&);
	Q_INVOKABLE void setWidthScale(QString sectionID, double scale);
	Q_INVOKABLE void setHeightScale(QString sectionID, double scale);
	Q_INVOKABLE QVariant getWidthScale(QString sectionID);
	Q_INVOKABLE QVariant getHeightScale(QString sectionID);


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

	void processExampleProjects();
	void loadConfig();

signals:
    void datasetFound();
    void datasetNotFound();
    void showFirstDataset();
	void openExampleProject(QString);


};
#endif //WELCOMESCREENHELPER_H
