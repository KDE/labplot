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

	\ingroup frontend
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
	auto config = KConfig();
	loadConfigFromTemplate(config);
}

void BinaryOptionsWidget::saveSettings() const {
	auto config = KConfig();
	saveConfigAsTemplate(config);
}

void BinaryOptionsWidget::loadConfigFromTemplate(KConfig& config) const {
	auto group = config.group(QStringLiteral("ImportBinary"));

	ui.niVectors->setValue(group.readEntry("Vectors", "2").toInt());
	ui.cbDataType->setCurrentIndex(group.readEntry("DataType", 0));
	ui.cbByteOrder->setCurrentIndex(group.readEntry("ByteOrder", 0));
	ui.sbSkipStartBytes->setValue(group.readEntry("SkipStartBytes", 0));
	ui.sbSkipBytes->setValue(group.readEntry("SkipBytes", 0));
	ui.chbCreateIndex->setChecked(group.readEntry("CreateIndex", false));
}

void BinaryOptionsWidget::saveConfigAsTemplate(KConfig& config) const {
	auto group = config.group(QStringLiteral("ImportBinary"));

	group.writeEntry("Vectors", ui.niVectors->value());
	group.writeEntry("ByteOrder", ui.cbByteOrder->currentIndex());
	group.writeEntry("DataType", ui.cbDataType->currentIndex());
	group.writeEntry("SkipStartBytes", ui.sbSkipStartBytes->value());
	group.writeEntry("SkipBytes", ui.sbSkipBytes->value());
	group.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
}
