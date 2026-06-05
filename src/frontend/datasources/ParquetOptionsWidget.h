/*
	File                 : ParquetOptionsWidget.h
	Project              : LabPlot
	Description          : widget providing options for the import of Apache Parquet/Arrow IPC/ORC data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARQUETOPTIONSWIDGET_H
#define PARQUETOPTIONSWIDGET_H

#include "ui_parquetoptionswidget.h"

class ParquetFilter;
class ImportFileWidget;

class ParquetOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ParquetOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(ParquetFilter*, const QString& fileName);
	QStringList selectedColumnNames() const;
	int lines() const {
		return ui.sbPreviewLines->value();
	}
	QTableWidget* previewWidget() const {
		return ui.twPreview;
	}

private:
	Ui::ParquetOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private Q_SLOTS:
	void selectionChanged();
	void selectAll();
	void deselectAll();
};

#endif
