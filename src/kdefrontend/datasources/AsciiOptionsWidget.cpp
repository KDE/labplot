/***************************************************************************
    File                 : AsciiOptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of ascii data
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)

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

#include "AsciiOptionsWidget.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

/*!
\class AsciiOptionsWidget
\brief Widget providing options for the import of ascii data

\ingroup kdefrontend
*/
AsciiOptionsWidget::AsciiOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(parent);

	ui.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	ui.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	ui.cbNumberFormat->addItems(AbstractFileFilter::numberFormats());
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());

	const QString textNumberFormatShort = i18n("This option determines how the imported strings have to be converted to numbers.");
	const QString textNumberFormat = textNumberFormatShort + "<br><br>" + i18n(
	                                     "For 'C Format', a period is used for the decimal point character and comma is used for the thousands group separator. "
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
	                                     "</ul>"
	                                 );

	ui.lNumberFormat->setToolTip(textNumberFormatShort);
	ui.lNumberFormat->setWhatsThis(textNumberFormat);
	ui.cbNumberFormat->setToolTip(textNumberFormatShort);
	ui.cbNumberFormat->setWhatsThis(textNumberFormat);

	const QString textDateTimeFormatShort = i18n("This option determines how the imported strings have to be converted to calendar date, i.e. year, month, and day numbers in the Gregorian calendar and to time.");
	const QString textDateTimeFormat = textDateTimeFormatShort + "<br><br>" + i18n(
	                                       "Expressions that may be used for the date part of format string:"
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

	connect(ui.chbHeader, &QCheckBox::stateChanged, this, &AsciiOptionsWidget::headerChanged);
}

void AsciiOptionsWidget::showAsciiHeaderOptions(bool b) {
	DEBUG("AsciiOptionsWidget::showAsciiHeaderOptions(" << b << ")");
	ui.chbHeader->setVisible(b);
	ui.lVectorNames->setVisible(b);
	ui.kleVectorNames->setVisible(b);
}

/*!
  enables a text field for the vector names if the option "Use the first row..." was not selected.
  Disables it otherwise.
*/
void AsciiOptionsWidget::headerChanged(int state) {
	DEBUG("AsciiOptionsWidget::headerChanged(" << state << ")");
	if (state == Qt::Checked) {
		ui.kleVectorNames->setEnabled(false);
		ui.lVectorNames->setEnabled(false);
	} else {
		ui.kleVectorNames->setEnabled(true);
		ui.lVectorNames->setEnabled(true);
	}
}

void AsciiOptionsWidget::applyFilterSettings(AsciiFilter* filter) const {
	Q_ASSERT(filter);

	filter->setCommentCharacter( ui.cbCommentCharacter->currentText() );
	filter->setSeparatingCharacter( ui.cbSeparatingCharacter->currentText() );
	filter->setNumberFormat( QLocale::Language(ui.cbNumberFormat->currentIndex()) );
	filter->setDateTimeFormat(ui.cbDateTimeFormat->currentText());
	filter->setCreateIndexEnabled( ui.chbCreateIndex->isChecked() );
	filter->setSimplifyWhitespacesEnabled( ui.chbSimplifyWhitespaces->isChecked() );
	filter->setNaNValueToZero( ui.chbConvertNaNToZero->isChecked() );
	filter->setRemoveQuotesEnabled( ui.chbRemoveQuotes->isChecked() );
	filter->setSkipEmptyParts( ui.chbSkipEmptyParts->isChecked() );
	filter->setVectorNames( ui.kleVectorNames->text() );
	filter->setHeaderEnabled( ui.chbHeader->isChecked() );
}


void AsciiOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportAscii");

	//TODO: check if this works (character gets currentItem?)
	ui.cbCommentCharacter->setCurrentItem(conf.readEntry("CommentCharacter", "#"));
	ui.cbSeparatingCharacter->setCurrentItem(conf.readEntry("SeparatingCharacter", "auto"));
	ui.cbNumberFormat->setCurrentIndex(conf.readEntry("NumberFormat", (int)QLocale::AnyLanguage));
	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("DateTimeFormat", "yyyy-MM-dd hh:mm:ss.zzz"));
	ui.chbCreateIndex->setChecked(conf.readEntry("CreateIndex", false));
	ui.chbSimplifyWhitespaces->setChecked(conf.readEntry("SimplifyWhitespaces", true));
	ui.chbConvertNaNToZero->setChecked(conf.readEntry("ConvertNaNToZero", false));
	ui.chbRemoveQuotes->setChecked(conf.readEntry("RemoveQuotes", false));
	ui.chbSkipEmptyParts->setChecked(conf.readEntry("SkipEmptyParts", false));
	ui.chbHeader->setChecked(conf.readEntry("UseFirstRow", true));
	ui.kleVectorNames->setText(conf.readEntry("Names", ""));
}

void AsciiOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportAscii");

	conf.writeEntry("CommentCharacter", ui.cbCommentCharacter->currentText());
	conf.writeEntry("SeparatingCharacter", ui.cbSeparatingCharacter->currentText());
	conf.writeEntry("NumberFormat", ui.cbNumberFormat->currentIndex());
	conf.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("CreateIndex", ui.chbCreateIndex->isChecked());
	conf.writeEntry("SimplifyWhitespaces", ui.chbSimplifyWhitespaces->isChecked());
	conf.writeEntry("ConvertNaNToZero", ui.chbConvertNaNToZero->isChecked());
	conf.writeEntry("RemoveQuotes", ui.chbRemoveQuotes->isChecked());
	conf.writeEntry("SkipEmptyParts", ui.chbSkipEmptyParts->isChecked());
	conf.writeEntry("UseFirstRow", ui.chbHeader->isChecked());
	conf.writeEntry("Names", ui.kleVectorNames->text());
}
