/***************************************************************************
File                 : ROOTOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of ROOT data
--------------------------------------------------------------------
Copyright            : (C) 2018 Christoph Roick (chrisito@gmx.de)
**************************************************************************/

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
