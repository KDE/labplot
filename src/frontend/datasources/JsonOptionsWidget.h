/*
	File                 : JsonOptionsWidget.h
	Project              : LabPlot
	Description          : Widget providing options for the import of json data.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JSONOPTIONSWIDGET_H
#define JSONOPTIONSWIDGET_H

#include "ui_jsonoptionswidget.h"

class JsonFilter;
class QAbstractItemModel;
class QJsonModel;

class JsonOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit JsonOptionsWidget(QWidget*);

	void applyFilterSettings(JsonFilter*, const QModelIndex&) const;
	void clearModel();
	void loadSettings() const;
	void saveSettings();
	void loadDocument(const QString& filename);
	QAbstractItemModel* model();

private:
	void setTooltips();
	QVector<int> getIndexRows(const QModelIndex&) const;

	QString m_filename;
	Ui::JsonOptionsWidget ui;
	QPointer<QJsonModel> m_model;

Q_SIGNALS:
	void error(const QString&);
};

#endif
