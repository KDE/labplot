/*
	File                 : ExcelOptionsWidget.h
	Project              : LabPlot
	Description          : Widget providing options for the import of Excel (xlsx) data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Fabian Kristof (fkristofszabolcs@gmail.com)
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXCELOPTIONSWIDGET_H
#define EXCELOPTIONSWIDGET_H

#include "ui_exceloptionswidget.h"

#include <QMap>
#include <QPair>
#include <QString>
#include <QVector>
#include <QWidget>

class ExcelFilter;
class ImportFileWidget;

class QStringList;

class ExcelOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ExcelOptionsWidget(QWidget*, ImportFileWidget*);
	~ExcelOptionsWidget();

	void updateContent(ExcelFilter* filter, const QString& fileName);
	QTableWidget* previewWidget() const {
		return ui.twPreview;
	}
	QStringList selectedExcelRegionNames() const;
	QVector<QStringList> previewString() const;
Q_SIGNALS:
	void enableDataPortionSelection(bool enable);
	void dataRegionSelectionChangedSignal();

private Q_SLOTS:
	void dataRegionSelectionChanged();

private:
	QMap<QPair<QString, int>, bool> m_regionIsPossibleToImportToMatrix;
	Ui::ExcelOptionsWidget ui;
	ImportFileWidget* m_fileWidget{nullptr};
	QVector<QStringList> m_previewString;
};

#endif // EXCELOPTIONSWIDGET_H
