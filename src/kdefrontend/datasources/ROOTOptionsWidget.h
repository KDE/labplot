/*
    File                 : ROOTOptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of ROOT data
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ROOTOPTIONSWIDGET_H
#define ROOTOPTIONSWIDGET_H

#include "ui_rootoptionswidget.h"

class ImportFileWidget;
class ROOTFilter;

/// Widget providing options for the import of ROOT data
class ROOTOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ROOTOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	/// Fill the list of available histograms
	void updateContent(ROOTFilter* filter, const QString &fileName);
	/// Return a list of selected histograms
	const QStringList selectedNames() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	int startRow() const { return ui.sbFirst->value(); }
	int endRow() const { return ui.sbLast->value(); }
	QVector<QStringList> columns() const;
	void setNRows(int nrows);
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::ROOTOptionsWidget ui;
	QTreeWidgetItem* histItem;
	QTreeWidgetItem* treeItem;
	QHash<QStringList, QVector<QStringList> > leaves;

	ImportFileWidget* m_fileWidget;
	bool histselected = false;

private slots:
	/// Updates the selected data set of a ROOT file when a new item is selected
	void rootObjectSelectionChanged();
};

#endif
