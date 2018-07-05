/***************************************************************************
    File                 : JsonOptionsWidget.h
    Project              : LabPlot
    Description          : Widget providing options for the import of json data.
    --------------------------------------------------------------------
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Andrey Cygankov (craftplace.ms@gmail.com)

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

#ifndef JSONOPTIONSWIDGET_H
#define JSONOPTIONSWIDGET_H

#include "ui_jsonoptionswidget.h"

class ImportFileWidget;
class JsonFilter;
class QJsonModel;
class QJsonTreeItem;

class JsonOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit JsonOptionsWidget(QWidget*, ImportFileWidget*);
	void applyFilterSettings(JsonFilter*, const QModelIndex&) const;
	void clearModel();
	void loadSettings() const;
	void saveSettings();
	void loadDocument(QString filename);
	QJsonModel* model();

private:
	void setTooltips();
	QVector<int> getIndexRows(const QModelIndex&) const;

	QString m_filename;
	Ui::JsonOptionsWidget ui;
	ImportFileWidget* m_fileWidget;
	QJsonModel* m_model;
};

#endif
