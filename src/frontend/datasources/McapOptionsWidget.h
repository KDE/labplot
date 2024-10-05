/*
	File                 : McapOptionsWidget.h
	Project              : LabPlot
	Description          : Widget providing options for the import of json data.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MCAPOPTIONSWIDGET_H
#define MCAPOPTIONSWIDGET_H

#include "ui_mcapoptionswidget.h"

class McapFilter;
class QAbstractItemModel;

class McapOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit McapOptionsWidget(QWidget*);

	void applyFilterSettings(McapFilter*) const;
	void loadSettings() const;
	void saveSettings();

private:
	QString m_filename;
	Ui::McapOptionsWidget ui;

Q_SIGNALS:
	void error(const QString&);
};

#endif
