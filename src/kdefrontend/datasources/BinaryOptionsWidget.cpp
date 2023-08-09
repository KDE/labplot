/*
	File                 : BinaryOptionsWidget.cpp
	Project              : LabPlot
	Description          : widget providing options for the import of binary data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BinaryOptionsWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/BinaryFilter.h"

#include <KConfigGroup>

/*!
	\class BinaryOptionsWidget
	\brief Widget providing options for the import of binary data

	\ingroup kdefrontend
*/

BinaryOptionsWidget::BinaryOptionsWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(parent);

	ui.cbDataType->addItems(BinaryFilter::dataTypes());
	ui.cbByteOrder->addItem(i18n("Little endian"), QDataStream::LittleEndian);
	ui.cbByteOrder->addItem(i18n("Big endian"), QDataStream::BigEndian);

	const QString textDataTypeShort = i18n("This option determines the data type that the imported data while converting to numbers.");

	ui.lDataType->setToolTip(textDataTypeShort);
	ui.lDataType->setWhatsThis(textDataTypeShort);
	ui.cbDataType->setToolTip(textDataTypeShort);
	ui.cbDataType->setWhatsThis(textDataTypeShort);

	const QString textByteOrderShort = i18n("This option determines the byte order of the imported data when converting to numbers.");
	const QString textByteOrder = textByteOrderShort + QStringLiteral("<br><br>")
		+ i18n("<table>"
			   "<tr><td>little endian</td><td>typical byte order (endianness) on Intel x86 processors.</td></tr>"
			   "<tr><td>big endian</td><td>typical byte order on Mainframes (IBM) and SPARC/PowerPC/Motorola processors.</td></tr>"
			   "</table>");

	ui.lByteOrder->setToolTip(textByteOrderShort);
	ui.lByteOrder->setWhatsThis(textByteOrder);
	ui.cbByteOrder->setToolTip(textByteOrderShort);
	ui.cbByteOrder->setWhatsThis(textByteOrder);
}

void BinaryOptionsWidget::applyFilterSettings(BinaryFilter* filter) const {
	Q_ASSERT(filter);

	filter->setVectors(ui.niVectors->value());
	filter->setDataType((BinaryFilter::DataType)ui.cbDataType->currentIndex());
	filter->setByteOrder(static_cast<QDataStream::ByteOrder>(ui.cbByteOrder->currentData().toInt()));
	filter->setSkipBytes(ui.sbSkipBytes->value());
	filter->setSkipStartBytes(ui.sbSkipStartBytes->value());
	filter->setCreateIndexEnabled(ui.chbCreateIndex->isChecked());
}

void BinaryOptionsWidget::loadSettings() const {
	KConfigGroup conf(Settings::config(), "ImportBinary");

	ui.niVectors->setValue(conf.readEntry("Vectors", "2").toInt());
	ui.cbDataType->setCurrentIndex(conf.readEntry("DataType", 0));
	ui.cbByteOrder->setCurrentIndex(conf.readEntry("ByteOrder", 0));
	ui.sbSkipStartBytes->setValue(conf.readEntry("SkipStartBytes", 0));
	ui.sbSkipBytes->setValue(conf.readEntry("SkipBytes", 0));
	ui.chbCreateIndex->setChecked(conf.readEntry("CreateIndex", false));
}

void BinaryOptionsWidget::saveSettings() {
	KConfigGroup conf(Settings::config(), "ImportBinary");

	conf.writeEntry("Vectors", ui.niVectors->value());
	conf.writeEntry("ByteOrder", ui.cbByteOrder->currentIndex());
	conf.writeEntry("DataType", ui.cbDataType->currentIndex());
	conf.writeEntry("SkipStartBytes", ui.sbSkipStartBytes->value());
	conf.writeEntry("SkipBytes", ui.sbSkipBytes->value());
	conf.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
}
