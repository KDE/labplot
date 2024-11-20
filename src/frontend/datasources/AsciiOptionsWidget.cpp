/*
	File                 : AsciiOptionsWidget.h
	Project              : LabPlot
	Description          : widget providing options for the import of ascii data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2017 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2009-2019 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AsciiOptionsWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/macros.h"

#include <KConfigGroup>
#include <KLocalizedString>

/*!
\class AsciiOptionsWidget
\brief Widget providing options for the import of ascii data

\ingroup frontend
*/
AsciiOptionsWidget::AsciiOptionsWidget(QWidget* parent, bool liveData)
	: QWidget(parent), m_liveData(liveData) {
	ui.setupUi(parent);

	ui.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	ui.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	ui.cbDecimalSeparator->addItem(i18n("Point '.'"));
	ui.cbDecimalSeparator->addItem(i18n("Comma ','"));
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	const QString textNumberFormatShort = i18n("This option determines how the imported strings have to be converted to numbers.");
	const QString textNumberFormat = textNumberFormatShort + QStringLiteral("<br><br>")
		+ i18n("When point character is used for the decimal separator, the valid number representations are:"
			   "<ul>"
			   "<li>1234.56</li>"
			   "<li>1,234.56</li>"
			   "<li>etc.</li>"
			   "</ul>"
			   "For comma as the decimal separator, the valid number representations are:"
			   "<ul>"
			   "<li>1234,56</li>"
			   "<li>1.234,56</li>"
			   "<li>etc.</li>"
			   "</ul>");

	ui.lDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.lDecimalSeparator->setWhatsThis(textNumberFormat);
	ui.cbDecimalSeparator->setToolTip(textNumberFormatShort);
	ui.cbDecimalSeparator->setWhatsThis(textNumberFormat);

	// only available for live data, will be activated explicitly
	ui.chbCreateTimestamp->setVisible(false);

	const QString textDateTimeFormatShort = i18n(
		"This option determines how the imported strings have to be converted to calendar date, i.e. year, month, and day numbers in the Gregorian calendar "
		"and to time.");
	const QString textDateTimeFormat = textDateTimeFormatShort + QStringLiteral("<br><br>")
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
			   "</table>"
			   "<br><br>"
			   "In case the provided expression is empty, the format will be auto-detected.");

	ui.lDateTimeFormat->setToolTip(textDateTimeFormatShort);
	ui.lDateTimeFormat->setWhatsThis(textDateTimeFormat);
	ui.cbDateTimeFormat->setToolTip(textDateTimeFormatShort);
	ui.cbDateTimeFormat->setWhatsThis(textDateTimeFormat);

	QString info = i18n("If checked, the specified line in the file will be used to determine the column names.");
	ui.chbHeader->setToolTip(info);

	info = i18n("Line in the file that should be used to determine the column names.");
	ui.sbHeaderLine->setToolTip(info);

	info = i18n("Custom column names, comma separated. E.g. \"x, y\"");
	ui.lVectorNames->setToolTip(info);
	ui.kleVectorNames->setToolTip(info);

	info = i18n("Custom column modes, comma separated");
	ui.lColumnMode->setToolTip(info);
	ui.kleColumnMode->setToolTip(info);

	// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
	// 15.955 digits
	info = i18n(
			   "If not empty, the number of columns must match the number of column names if provided and it must match the number of  columns in the imported file"
			   "Datatypes:"
			   "<table>"
			   "<tr><td>%1</td><td>Integer number with 32bit size (−2.147.483.648 .. 2.147.483.647).</td></tr>"
			   "<tr><td>%2</td><td>Integer number with 64bit size (−9.223.372.036.854.775.808 .. 9.223.372.036.854.775.807).</td></tr>"
			   "<tr><td>%3</td><td>Floating point number 64bit size. Resolution of around 16 digits.</td></tr>"
			   "<tr><td>%4</td><td>Datetime with the format from the datetime text box if not empty, otherwise automatically determined.</td></tr>"
			   "<tr><td>%5</td><td>A text.</td></tr>"
			   "</table>");
	using Mode = AbstractColumn::ColumnMode;
	const QVector<Mode> vec = {Mode::Integer, Mode::BigInt, Mode::Double, Mode::DateTime, Mode::Text};
	for (const auto m: vec) {
		const auto& s = AsciiFilter::dataTypeString(m);
		if (s.first == s.second)
			info = info.arg(s.first);
		else
			info = info.arg(i18n("%1 or %2, s.first, s.second));
	}
	ui.lColumnMode->setWhatsThis(info);
	ui.kleColumnMode->setWhatsThis(info);

	if (m_liveData) {
		headerChanged(false);
		ui.chbHeader->setVisible(false);
		ui.sbHeaderLine->setVisible(false);
		ui.lVectorNames->setVisible(true);
		ui.kleVectorNames->setVisible(true);
	}

	connect(ui.chbHeader, &QCheckBox::toggled, this, &AsciiOptionsWidget::headerChanged);
	connect(ui.sbHeaderLine, QOverload<int>::of(&QSpinBox::valueChanged), this, &AsciiOptionsWidget::headerLineChanged);
	connect(ui.kleColumnMode, &QLineEdit::textChanged, this, &AsciiOptionsWidget::columnModesChanged);
}

void AsciiOptionsWidget::showAsciiHeaderOptions(bool visible) {
	DEBUG(Q_FUNC_INFO);

	ui.chbHeader->setVisible(visible);
	ui.sbHeaderLine->setVisible(visible);
	if (visible) {
		ui.lVectorNames->setVisible(!ui.chbHeader->isChecked());
		ui.kleVectorNames->setVisible(!ui.chbHeader->isChecked());
	} else {
		ui.lVectorNames->setVisible(false);
		ui.kleVectorNames->setVisible(false);
	}
}

void AsciiOptionsWidget::showTimestampOptions(bool visible) {
	ui.chbCreateTimestamp->setVisible(visible);
	m_createTimeStampAvailable = visible;
}

/*!
  Shows a text field for the vector names if the option "Use the first row..." was not selected.
  Hides it otherwise.
*/
void AsciiOptionsWidget::headerChanged(bool state) const {
	ui.sbHeaderLine->setEnabled(state);
	ui.kleVectorNames->setVisible(!state);
	ui.lVectorNames->setVisible(!state);
}

void AsciiOptionsWidget::updateWidgets(const AsciiFilter::Properties& properties) {
	ui.cbCommentCharacter->setCurrentText(properties.commentCharacter);
	ui.cbSeparatingCharacter->setCurrentText(properties.separator);

	const auto decimalSeparator = properties.locale.decimalPoint();
	int index = (decimalSeparator == QLatin1Char('.')) ? 0 : 1;
	ui.cbDecimalSeparator->setCurrentIndex(index);

	ui.sbYearBase->setValue(properties.baseYear);
	ui.cbDateTimeFormat->setCurrentText(properties.dateTimeFormat);
	ui.chbCreateIndex->setChecked(properties.createIndex);
	ui.chbCreateTimestamp->setChecked(properties.createTimestamp);
	ui.chbSimplifyWhitespaces->setChecked(properties.simplifyWhitespaces);
	ui.chbConvertNaNToZero->setChecked(properties.nanValue == 0);
	ui.chbRemoveQuotes->setChecked(properties.removeQuotes);
	ui.chbSkipEmptyParts->setChecked(properties.skipEmptyParts);
	ui.chbHeader->setChecked(properties.headerEnabled);
	ui.sbHeaderLine->setValue(properties.headerLine);
	ui.kleVectorNames->setText(properties.columnNamesRaw);
	ui.kleColumnMode->setText(properties.columnModesString);
	ui.chbIntAsDouble->setChecked(properties.intAsDouble);
}

void AsciiOptionsWidget::applyFilterSettings(AsciiFilter::Properties& properties) const {
	DEBUG(Q_FUNC_INFO)

	properties.commentCharacter = ui.cbCommentCharacter->currentText();
	const auto v = ui.cbSeparatingCharacter->currentText();
	if (v.compare(AsciiFilter::autoSeparatorDetectionString(), Qt::CaseInsensitive) == 0) {
		properties.automaticSeparatorDetection = true;
		properties.separator.clear();
	} else {
		properties.automaticSeparatorDetection = false;
		properties.separator = v;
	}

	// TODO: use general setting for decimal separator?
	const auto lang = ui.cbDecimalSeparator->currentIndex() == 0 ? QLocale::Language::C : QLocale::Language::German;
	properties.locale = lang;
	properties.dateTimeFormat = ui.cbDateTimeFormat->currentText();
	properties.createIndex = ui.chbCreateIndex->isChecked();

	// save the timestamp option only if it's visible, i.e. live source is used.
	// use the default setting in the filter (false) otherwise for non-live source
	if (m_createTimeStampAvailable)
		properties.createTimestamp = ui.chbCreateTimestamp->isChecked();

	properties.simplifyWhitespaces = ui.chbSimplifyWhitespaces->isChecked();
	properties.nanValue = ui.chbConvertNaNToZero->isChecked() ? 0.0 : std::numeric_limits<double>::quiet_NaN();
	properties.removeQuotes = ui.chbRemoveQuotes->isChecked();
	properties.skipEmptyParts = ui.chbSkipEmptyParts->isChecked();
	properties.columnNamesRaw = ui.kleVectorNames->text();
	properties.headerEnabled = ui.chbHeader->isChecked();
	properties.headerLine = ui.sbHeaderLine->value();
	properties.baseYear = ui.sbYearBase->value();
	properties.columnModesString = ui.kleColumnMode->text();
	properties.intAsDouble = ui.chbIntAsDouble->isChecked();
}

void AsciiOptionsWidget::setSeparatingCharacter(QLatin1Char character) {
	ui.cbSeparatingCharacter->setCurrentItem(QString(character));
}

bool AsciiOptionsWidget::isValid(QString& errorMessage) {
	QString invalidString;
	QVector<AbstractColumn::ColumnMode> s;
	if (!AsciiFilter::determineColumnModes(ui.kleColumnMode->text(), s, invalidString)) {
		errorMessage = i18n("Datatype not found: %1").arg(invalidString);
		return false;
	}
	return true;
}

// ##############################################################################
// ########################## Template handling   ##############################
// ##############################################################################
void AsciiOptionsWidget::loadSettings() const {
	auto config = KConfig();
	loadConfigFromTemplate(config);
}

void AsciiOptionsWidget::saveSettings() const {
	auto config = KConfig();
	saveConfigAsTemplate(config);
}

void AsciiOptionsWidget::loadConfigFromTemplate(KConfig& config) const {
	auto group = config.group(QStringLiteral("ImportAscii"));

	ui.cbCommentCharacter->setCurrentText(group.readEntry("CommentCharacter", "#"));
	ui.cbSeparatingCharacter->setCurrentText(group.readEntry("SeparatingCharacter", "auto"));

	const auto decimalSeparator = QLocale().decimalPoint();
	int index = (decimalSeparator == QLatin1Char('.')) ? 0 : 1;
	ui.cbDecimalSeparator->setCurrentIndex(group.readEntry("DecimalSeparator", index));

	ui.sbYearBase->setValue(group.readEntry("YearBase", 1900));
	ui.cbDateTimeFormat->setCurrentText(group.readEntry("DateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(group.readEntry("CreateIndex", false));
	ui.chbCreateTimestamp->setChecked(group.readEntry("CreateTimestamp", true));
	ui.chbSimplifyWhitespaces->setChecked(group.readEntry("SimplifyWhitespaces", true));
	ui.chbConvertNaNToZero->setChecked(group.readEntry("ConvertNaNToZero", false));
	ui.chbRemoveQuotes->setChecked(group.readEntry("RemoveQuotes", false));
	ui.chbSkipEmptyParts->setChecked(group.readEntry("SkipEmptyParts", false));
	if (!m_liveData)
		ui.chbHeader->setChecked(group.readEntry("UseFirstRow", true)); // header enabled - yes/no
	else
		ui.chbHeader->setChecked(false);
	headerChanged(ui.chbHeader->isChecked()); // call this to update the status of the SpinBox for the header line
	ui.sbHeaderLine->setValue(group.readEntry(QLatin1String("HeaderLine"), 1));
	ui.kleVectorNames->setText(group.readEntry("Names", ""));
	ui.kleColumnMode->setText(group.readEntry("Modes", ""));
}

void AsciiOptionsWidget::saveConfigAsTemplate(KConfig& config) const {
	auto group = config.group(QStringLiteral("ImportAscii"));

	group.writeEntry("CommentCharacter", ui.cbCommentCharacter->currentText());
	group.writeEntry("SeparatingCharacter", ui.cbSeparatingCharacter->currentText());
	group.writeEntry("DecimalSeparator", ui.cbDecimalSeparator->currentIndex());
	group.writeEntry("YearBase", ui.sbYearBase->value());
	group.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	group.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
	group.writeEntry("CreateTimestamp", ui.chbCreateTimestamp->isChecked());
	group.writeEntry("SimplifyWhitespaces", ui.chbSimplifyWhitespaces->isChecked());
	group.writeEntry("ConvertNaNToZero", ui.chbConvertNaNToZero->isChecked());
	group.writeEntry("RemoveQuotes", ui.chbRemoveQuotes->isChecked());
	group.writeEntry("SkipEmptyParts", ui.chbSkipEmptyParts->isChecked());
	group.writeEntry("UseFirstRow", ui.chbHeader->isChecked()); // header enabled - yes/no
	group.writeEntry(QLatin1String("HeaderLine"), ui.sbHeaderLine->value());
	group.writeEntry("Names", ui.kleVectorNames->text());
	group.writeEntry("Modes", ui.kleColumnMode->text());
}
