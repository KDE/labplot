/*
	File                 : OdsOptionsWidget.h
	Project              : LabPlot
	Description          : Widget providing options for the import of Open Document Spreadsheet (ods) data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <steffan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ODSOPTIONSWIDGET_H
#define ODSOPTIONSWIDGET_H

#include "ui_odsoptionswidget.h"

#include <QMap>
#include <QPair>

class OdsFilter;
class ImportFileWidget;

class OdsOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit OdsOptionsWidget(QWidget*, ImportFileWidget*);
	~OdsOptionsWidget();

	void updateContent(OdsFilter* filter, const QString& fileName);
	QTableWidget* previewWidget() const {
		return ui.twPreview;
	}
	QStringList selectedOdsSheetNames() const;
	QVector<QStringList> previewString() const;
Q_SIGNALS:
	void enableDataPortionSelection(bool enable);

public Q_SLOTS:
	void sheetSelectionChanged();

private:
	Ui::OdsOptionsWidget ui;
	ImportFileWidget* m_fileWidget{nullptr};
	// TODO:	use std::unique_ptr<ImportFileWidget> m_fileWidget{nullptr};
	QVector<QStringList> m_previewString;
};

#endif // ODSOPTIONSWIDGET_H
