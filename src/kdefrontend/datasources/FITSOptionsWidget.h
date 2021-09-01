/*
    File                 : FITSOptionsWidget.cpp
    Project              : LabPlot
    Description          : Widget providing options for the import of FITS data
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2017 Stefan Gerlach <stefan.gerlach@uni.kn>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FITSOPTIONSWIDGET_H
#define FITSOPTIONSWIDGET_H

#include "ui_fitsoptionswidget.h"

class FITSFilter;
class ImportFileWidget;

class FITSOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit FITSOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	QString currentExtensionName();
	void updateContent(FITSFilter*, const QString& fileName);
	const QStringList selectedExtensions() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }
	const QString extensionName(bool* ok);

private:
	Ui::FITSOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void fitsTreeWidgetSelectionChanged();
};

#endif // FITSOPTIONSWIDGET_H
