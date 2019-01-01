/***************************************************************************
File                 : ImageOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of image data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
