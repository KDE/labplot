/*
    File                 : HDF5OptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of HDF5 data
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HDF5OPTIONSWIDGET_H
#define HDF5OPTIONSWIDGET_H

#include "ui_hdf5optionswidget.h"

class HDF5Filter;
class ImportFileWidget;

class HDF5OptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit HDF5OptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(HDF5Filter*, const QString &fileName);
	const QStringList selectedNames() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::HDF5OptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private Q_SLOTS:
	void hdf5TreeWidgetSelectionChanged();
};

#endif
