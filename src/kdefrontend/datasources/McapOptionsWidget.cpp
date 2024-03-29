/*
	File                 : McapOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of json data.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "McapOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/McapFilter.h"
#include "backend/datasources/filters/QJsonModel.h"
#include "backend/lib/trace.h"

#include <KCompressionDevice>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QJsonDocument>

/*!
\class McapOptionsWidget
\brief Widget providing options for the import of json data

\ingroup kdefrontend
*/
McapOptionsWidget::McapOptionsWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(parent);
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());
}

void McapOptionsWidget::applyFilterSettings(McapFilter* filter) const {
	Q_ASSERT(filter);

	QLocale::Language lang;
	lang = QLocale::Language::C;
	filter->setNumberFormat(lang);

	filter->setDateTimeFormat(ui.cbDateTimeFormat->currentText());
	filter->setCreateIndexEnabled(ui.chbCreateIndex->isChecked());
	filter->setNaNValueToZero(ui.chbConvertNaNToZero->isChecked());

	// TODO: change this after implementation other row types
	filter->setDataRowType(QJsonValue::Object);
}

void McapOptionsWidget::loadSettings() const {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportMcap"));
	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("DateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(conf.readEntry("CreateIndex", false));
	ui.chbConvertNaNToZero->setChecked(conf.readEntry("ConvertNaNToZero", false));
}

void McapOptionsWidget::saveSettings() {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportMcap"));

	conf.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
	conf.writeEntry("ConvertNaNToZero", ui.chbConvertNaNToZero->isChecked());
}
