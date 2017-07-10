/***************************************************************************
    File                 : BinaryOptionsWidget.cpp
    Project              : LabPlot
    Description          : widget providing options for the import of binary data
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2009 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

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
#include "BinaryOptionsWidget.h"
#include "backend/datasources/filters/BinaryFilter.h"

#include <KSharedConfig>
#include <KConfigGroup>

 /*!
	\class BinaryOptionsWidget
	\brief Widget providing options for the import of binary data

	\ingroup kdefrontend
 */

BinaryOptionsWidget::BinaryOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(parent);

	ui.cbDataType->addItems(BinaryFilter::dataTypes());
	ui.cbByteOrder->addItems(BinaryFilter::byteOrders());
}

void BinaryOptionsWidget::applyFilterSettings(BinaryFilter* filter) const {
	Q_ASSERT(filter);

	filter->setVectors( ui.niVectors->value() );
	filter->setDataType( (BinaryFilter::DataType) ui.cbDataType->currentIndex() );
}

void BinaryOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	ui.niVectors->setValue(conf.readEntry("Vectors", "2").toInt());
	ui.cbDataType->setCurrentIndex(conf.readEntry("DataType", 0));
	ui.cbByteOrder->setCurrentIndex(conf.readEntry("ByteOrder", 0));
	ui.sbSkipStartBytes->setValue(conf.readEntry("SkipStartBytes", 0));
	ui.sbSkipBytes->setValue(conf.readEntry("SkipBytes", 0));
}

void BinaryOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	conf.writeEntry("Vectors", ui.niVectors->value());
	conf.writeEntry("ByteOrder", ui.cbByteOrder->currentIndex());
	conf.writeEntry("DataType", ui.cbDataType->currentIndex());
	conf.writeEntry("SkipStartBytes", ui.sbSkipStartBytes->value());
	conf.writeEntry("SkipBytes", ui.sbSkipBytes->value());
}
