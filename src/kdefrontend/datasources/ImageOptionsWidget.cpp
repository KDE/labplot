/*
File                 : ImageOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of image data
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ImageOptionsWidget.h"

#include <KSharedConfig>
#include <KConfigGroup>

 /*!
	\class ImageOptionsWidget
	\brief Widget providing options for the import of image data

	\ingroup kdefrontend
 */

ImageOptionsWidget::ImageOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(parent);

	ui.cbImportFormat->addItems(ImageFilter::importFormats());

	const QString textImageFormatShort = i18n("This option determines how the image is converted when importing.");

	ui.lImportFormat->setToolTip(textImageFormatShort);
	ui.lImportFormat->setWhatsThis(textImageFormatShort);
	ui.cbImportFormat->setToolTip(textImageFormatShort);
	ui.cbImportFormat->setWhatsThis(textImageFormatShort);
}

void ImageOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	ui.cbImportFormat->setCurrentIndex(conf.readEntry("ImportFormat", 0));
}

void ImageOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	conf.writeEntry("ImportFormat", ui.cbImportFormat->currentIndex());
}
