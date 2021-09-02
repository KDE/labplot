/*
File                 : NetCDFOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of NetCDF data
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NETCDFOPTIONSWIDGET_H
#define NETCDFOPTIONSWIDGET_H

#include "ui_netcdfoptionswidget.h"

class NetCDFFilter;
class ImportFileWidget;

class NetCDFOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit NetCDFOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(NetCDFFilter*, const QString& fileName);
	const QStringList selectedNames() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::NetCDFOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void netcdfTreeWidgetSelectionChanged();
};

#endif
