/*
	File                 : JsonOptionsWidget.cpp
	Project              : LabPlot
	Description          : Widget providing options for the import of json data.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Andrey Cygankov <craftplace.ms@gmail.com>
	SPDX-FileCopyrightText: 2018-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JsonOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/JsonFilter.h"
#include "backend/datasources/filters/QJsonModel.h"
#include "backend/lib/trace.h"

#include <KConfigGroup>
#include <KFilterDev>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
\class JsonOptionsWidget
\brief Widget providing options for the import of json data

\ingroup kdefrontend
*/
JsonOptionsWidget::JsonOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
	: QWidget(parent)
	, m_fileWidget(fileWidget)
	, m_model(new QJsonModel()) {
	ui.setupUi(parent);

	ui.cbDecimalSeparator->addItem(i18n("Point '.'"));
	ui.cbDecimalSeparator->addItem(i18n("Comma ','"));
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

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

	// TODO: change this after implementation other row types
	filter->setDataRowType(QJsonValue::Array);
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
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportJson");

	const QChar decimalSeparator = QLocale().decimalPoint();
	int index = (decimalSeparator == '.') ? 0 : 1;
	ui.cbDecimalSeparator->setCurrentIndex(conf.readEntry("DecimalSeparator", index));

	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("DateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(conf.readEntry("CreateIndex", false));
	ui.chbConvertNaNToZero->setChecked(conf.readEntry("ConvertNaNToZero", false));
	ui.chbImportObjectNames->setChecked(conf.readEntry("ParseRowsName", false));
}

void JsonOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportJson");

	conf.writeEntry("DecimalSeparator", ui.cbDecimalSeparator->currentIndex());
	conf.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
	conf.writeEntry("ConvertNaNToZero", ui.chbConvertNaNToZero->isChecked());
	conf.writeEntry("ParseRowsName", ui.chbImportObjectNames->isChecked());
}

void JsonOptionsWidget::loadDocument(const QString& filename) {
	PERFTRACE("JsonOptionsWidget::loadDocument");
	if (m_filename == filename)
		return;
	else
		m_filename = filename;

	KFilterDev device(m_filename);
	m_model->clear();
	if (!device.open(QIODevice::ReadOnly) || (device.atEnd() && !device.isSequential()) || // empty file
		!m_model->loadJson(device.readAll()))
		clearModel();
}

QAbstractItemModel* JsonOptionsWidget::model() {
	return m_model;
}

void JsonOptionsWidget::setTooltips() {
	const QString textNumberFormatShort = i18n("This option determines how the imported strings have to be converted to numbers.");
	const QString textNumberFormat = textNumberFormatShort + "<br><br>"
		+ i18n("For 'C Format', a period is used for the decimal point character and comma is used for the thousands group separator. "
			   "Valid number representations are:"
			   "<ul>"
			   "<li>1234.56</li>"
			   "<li>1,234.56</li>"
			   "<li>etc.</li>"
			   "</ul>"
			   "When using 'System locale', the system settings will be used. "
			   "E.g., for the German local the valid number representations are:"
			   "<ul>"
			   "<li>1234,56</li>"
			   "<li>1.234,56</li>"
			   "<li>etc.</li>"
			   "</ul>");

	ui.lDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.lDecimalSeparator->setWhatsThis(textNumberFormat);
	ui.cbDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.cbDecimalSeparator->setWhatsThis(textNumberFormat);

	const QString textDateTimeFormatShort = i18n(
		"This option determines how the imported strings have to be converted to calendar date, i.e. year, month, and day numbers in the Gregorian calendar "
		"and to time.");
	const QString textDateTimeFormat = textDateTimeFormatShort + "<br><br>"
		+ i18n("Expressions that may be used for the date part of format string:"
			   "<table>"
			   "<tr><td>d</td><td>the day as number without a leading zero (1 to 31).</td></tr>"
			   "<tr><td>dd</td><td>the day as number with a leading zero (01 to 31).</td></tr>"
			   "<tr><td>ddd</td><td>the abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name.</td></tr>"
			   "<tr><td>dddd</td><td>the long localized day name (e.g. 'Monday' to 'Sunday'). Uses the system locale to localize the name.</td></tr>"
			   "<tr><td>M</td><td>the month as number without a leading zero (1 to 12).</td></tr>"
			   "<tr><td>MM</td><td>the month as number with a leading zero (01 to 12).</td></tr>"
			   "<tr><td>MMM</td><td>the abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name.</td></tr>"
			   "<tr><td>MMMM</td><td>the long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name.</td></tr>"
			   "<tr><td>yy</td><td>the year as two digit number (00 to 99).</td></tr>"
			   "<tr><td>yyyy</td><td>the year as four digit number. If the year is negative, a minus sign is prepended in addition.</td></tr>"
			   "</table><br><br>"
			   "Expressions that may be used for the time part of the format string:"
			   "<table>"
			   "<tr><td>h</td><td>the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)</td></tr>"
			   "<tr><td>hh</td><td>the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)</td></tr>"
			   "<tr><td>H</td><td>the hour without a leading zero (0 to 23, even with AM/PM display)</td></tr>"
			   "<tr><td>HH</td><td>the hour with a leading zero (00 to 23, even with AM/PM display)</td></tr>"
			   "<tr><td>m</td><td>the minute without a leading zero (0 to 59)</td></tr>"
			   "<tr><td>mm</td><td>the minute with a leading zero (00 to 59)</td></tr>"
			   "<tr><td>s</td><td>the second without a leading zero (0 to 59)</td></tr>"
			   "<tr><td>ss</td><td>the second with a leading zero (00 to 59)</td></tr>"
			   "<tr><td>z</td><td>the milliseconds without leading zeroes (0 to 999)</td></tr>"
			   "<tr><td>zzz</td><td>the milliseconds with leading zeroes (000 to 999)</td></tr>"
			   "<tr><td>AP or A</td><td>interpret as an AM/PM time. AP must be either 'AM' or 'PM'.</td></tr>"
			   "<tr><td>ap or a</td><td>Interpret as an AM/PM time. ap must be either 'am' or 'pm'.</td></tr>"
			   "</table><br><br>"
			   "Examples are:"
			   "<table>"
			   "<tr><td>dd.MM.yyyy</td><td>20.07.1969</td></tr>"
			   "<tr><td>ddd MMMM d yy</td><td>Sun July 20 69</td></tr>"
			   "<tr><td>'The day is' dddd</td><td>The day is Sunday</td></tr>"
			   "</table>");

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
