/***************************************************************************
    File                 : ImportSQLDatabaseWidget.cpp
    Project              : LabPlot
    Description          : SQLDatabase
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2016-2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef IMPORTSQLDATABASEWIDGET_H
#define IMPORTSQLDATABASEWIDGET_H

#include <QSqlDatabase>

#include "ui_importsqldatabasewidget.h"

class AspectTreeModel;
class Project;
class QStandardItemModel;

class ImportSQLDatabaseWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseWidget(QWidget*);
	~ImportSQLDatabaseWidget();

private:
	Ui::ImportSQLDatabaseWidget ui;
	QList<QString> vendorList;
	QList<QString> tableNamesList;
	QSqlDatabase m_db;
	AspectTreeModel* m_aspectTreeModel;
	Project* mainProject;
	QStandardItemModel* m_databaseTreeModel;

	void updateStatus();
	void setProjectModel();
	void setDatabaseModel();
	void previewColumn(QString, QString, int, bool showPreview = false);

private slots:
	void loadSettings();
	void showDatabaseManager();
	void connectDatabase();
	void showPreview();
	void importData(bool showPreview = false);

signals:
	void statusChanged(QString);

};

#endif // IMPORTSQLDATABASEWIDGET_H
