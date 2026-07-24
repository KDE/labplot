/*
	File                 : JsonOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of json data.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JsonOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/datasources/filters/QJsonModel.h"
#include "backend/lib/trace.h"

#include <KCompressionDevice>
#include <KConfigGroup>
#include <KLocalizedString>

/*!
\class JsonOptionsWidget
\brief Widget providing options for the import of json data

\ingroup frontend
*/
JsonOptionsWidget::JsonOptionsWidget(QWidget* parent)
	: QWidget(parent)
	, m_model(new QJsonModel()) {
	ui.setupUi(parent);

	ui.cbDecimalSeparator->addItem(i18n("Point '.'"));
	ui.cbDecimalSeparator->addItem(i18n("Comma ','"));
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	connect(m_model, &QJsonModel::error, this, &JsonOptionsWidget::error);

	setTooltips();
}

void JsonOptionsWidget::applyFilterSettings(JsonFilter* filter, const QModelIndex& index) const {
	Q_ASSERT(filter);

	filter->setModel(m_model);
	filter->setModelRows(getIndexRows(index));

	QLocale::Language lang;
	if (ui.cbDecimalSeparator->currentIndex() == 0)
		lang = QLocale::Language::C;
	else
		lang = QLocale::Language::German;
	filter->setNumberFormat(lang);

	filter->setDateTimeFormat(ui.cbDateTimeFormat->currentText());
	filter->setCreateIndexEnabled(ui.chbCreateIndex->isChecked());
	filter->setNaNValueToZero(ui.chbConvertNaNToZero->isChecked());
	filter->setImportObjectNames(ui.chbImportObjectNames->isChecked());

	if (!index.isValid())
		return;
	auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
	if (item->childCount() < 1)
		return;
	filter->setDataRowType(item->child(0)->type());
}

void JsonOptionsWidget::clearModel() {
	m_model->clear();
	m_filename.clear();
}

void JsonOptionsWidget::loadSettings() const {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportJson"));

	const auto decimalSeparator = QLocale().decimalPoint();
	const int index = (decimalSeparator == QLatin1Char('.')) ? 0 : 1;
	ui.cbDecimalSeparator->setCurrentIndex(conf.readEntry("DecimalSeparator", index));

	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("DateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(conf.readEntry("CreateIndex", false));
	ui.chbConvertNaNToZero->setChecked(conf.readEntry("ConvertNaNToZero", false));
	ui.chbImportObjectNames->setChecked(conf.readEntry("ParseRowsName", false));
}

void JsonOptionsWidget::saveSettings() {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportJson"));

	conf.writeEntry("DecimalSeparator", ui.cbDecimalSeparator->currentIndex());
	conf.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
	conf.writeEntry("ConvertNaNToZero", ui.chbConvertNaNToZero->isChecked());
	conf.writeEntry("ParseRowsName", ui.chbImportObjectNames->isChecked());
}

void JsonOptionsWidget::loadDocument(const QString& filename) {
	PERFTRACE(QStringLiteral("JsonOptionsWidget::loadDocument"));
	if (m_filename == filename)
		return;
	else
		m_filename = filename;

	KCompressionDevice device(m_filename);
	m_model->clear();
	if (!device.open(QIODevice::ReadOnly) || (device.atEnd() && !device.isSequential()) || // empty file
		!m_model->loadJson(device.readAll()))
		clearModel();
}

QAbstractItemModel* JsonOptionsWidget::model() {
	return m_model;
}

void JsonOptionsWidget::setTooltips() {
	const QString textNumberFormatShort = AbstractFileFilter::textNumberFormatShort();
	const QString textNumberFormat = AbstractFileFilter::textNumberFormat();
	ui.lDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.lDecimalSeparator->setWhatsThis(textNumberFormat);
	ui.cbDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.cbDecimalSeparator->setWhatsThis(textNumberFormat);

	const QString textDateTimeFormatShort = AbstractFileFilter::textDateTimeFormatShort();
	const QString textDateTimeFormat = AbstractFileFilter::textDateTimeFormat();
	ui.lDateTimeFormat->setToolTip(textDateTimeFormatShort);
	ui.lDateTimeFormat->setWhatsThis(textDateTimeFormat);
	ui.cbDateTimeFormat->setToolTip(textDateTimeFormatShort);
	ui.cbDateTimeFormat->setWhatsThis(textDateTimeFormat);
}

QVector<int> JsonOptionsWidget::getIndexRows(const QModelIndex& index) const {
	QVector<int> rows;
	QModelIndex current = index;
	while (current.isValid()) {
		rows.prepend(current.row());
		current = current.parent();
	}
	return rows;
}
