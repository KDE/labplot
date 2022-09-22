/*
	File                 : HDF5OptionsWidget.h
	Project              : LabPlot
	Description          : widget providing options for the import of HDF5 data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MDFOPTIONSWIDGET_H
#define MDFOPTIONSWIDGET_H

#include "ui_mdfoptionswidget.h"

#include <QWidget>
#include <QStringList>

class MDFFilter;
class ImportFileWidget;
class QTableWidget;
class QString;

class MDFOptionsWidget : public QWidget {
	Q_OBJECT

public:
    explicit MDFOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
    void updateContent(MDFFilter*, const QString& fileName);
	const QStringList selectedNames() const;
	int lines() const {
		return ui.sbPreviewLines->value();
	}
	QTableWidget* previewWidget() const {
		return ui.twPreview;
	}

private:
    Ui::MDFOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private Q_SLOTS:
	void hdf5TreeWidgetSelectionChanged();
};

#endif // MDFOPTIONSWIDGET_H
