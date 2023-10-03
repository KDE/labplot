/*
	File                 : XLSXOptionsWidget.h
	Project              : LabPlot
	Description          : Widget providing options for the import of XLSX (Excel) data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-FileCopyrightText: 2023 Stefan Gerlach (stefan.gerlach@uni.kn)
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLSXOPTIONSWIDGET_H
#define XLSXOPTIONSWIDGET_H

#include "ui_xlsxoptionswidget.h"

#include <QMap>
#include <QPair>

class XLSXFilter;
class ImportFileWidget;

class XLSXOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit XLSXOptionsWidget(QWidget*, ImportFileWidget*);
	~XLSXOptionsWidget();

	void updateContent(XLSXFilter* filter, const QString& fileName);
	QTableWidget* previewWidget() const {
		return ui.twPreview;
	}
	QStringList selectedXLSXRegionNames() const;
	QVector<QStringList> previewString() const;
Q_SIGNALS:
	void enableDataPortionSelection(bool enable);
	void dataRegionSelectionChangedSignal();

public Q_SLOTS:
	void dataRegionSelectionChanged();

private:
	Ui::XLSXOptionsWidget ui;
	ImportFileWidget* m_fileWidget{nullptr};
	// TODO: use std::unique_ptr<ImportFileWidget> m_fileWidget{nullptr};
	QMap<QPair<QString, int>, bool> m_regionIsPossibleToImportToMatrix;
	QVector<QStringList> m_previewString;
};

#endif // XLSXOPTIONSWIDGET_H
