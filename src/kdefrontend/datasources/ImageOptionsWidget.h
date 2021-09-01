/*
File                 : ImageOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of image data
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEOPTIONSWIDGET_H
#define IMAGEOPTIONSWIDGET_H

#include "ui_imageoptionswidget.h"
#include "backend/datasources/filters/ImageFilter.h"

class ImageOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImageOptionsWidget(QWidget*);
	void loadSettings() const;
	void saveSettings();
	ImageFilter::ImportFormat currentFormat() const {
		return (ImageFilter::ImportFormat)ui.cbImportFormat->currentIndex();
	}

private:
	Ui::ImageOptionsWidget ui;
};

#endif
