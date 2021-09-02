/*
File                 : MatioOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of Matio data
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MATIOOPTIONSWIDGET_H
#define MATIOOPTIONSWIDGET_H

#include "ui_matiooptionswidget.h"

class MatioFilter;
class ImportFileWidget;

class MatioOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit MatioOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(MatioFilter*, const QString& fileName);
	const QStringList selectedNames() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::MatioOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void selectionChanged();
};

#endif
