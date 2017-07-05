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

#include <KLocale>
#include <KSharedConfig>
#include <KConfigGroup>

 /*!
	\class AsciiOptionsWidget
	\brief Widget providing options for the import of ascii data

	\ingroup kdefrontend
 */
AsciiOptionsWidget::AsciiOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.cbSeparatingCharacter->addItems(AsciiFilter::separatorCharacters());
	ui.cbCommentCharacter->addItems(AsciiFilter::commentCharacters());
	ui.cbNumbersFormat->addItems(AbstractFileFilter::numberFormats());
	ui.cbDateTimeFormat->addItems(AbstractColumn::dateTimeFormats());
	ui.chbTranspose->hide(); //TODO: enable later

	QString text = i18n(
		"<b> Number format: </b><br>"
		"This option determines how the imported strings has to be converted to numbers. "
		"For 'C Format', a period is used for the decimal point character and comma is used for the thousands group separator. <br>"
		"Valid number representations are: <br>"
		"<ul>"
		"<li>1234.56</li>"
		"<li>1,234.56</li>"
		"<li>etc.</li>"
		"</ul><br><br>"
		"When using 'System locale', the system settings will be used. "
		"E.g., for the German local the valid number representations are: <br>"
		"<ul>"
		"<li>1234,56</li>"
		"<li>1.234,56</li>"
		"<li>etc.</li>"
		"</ul><br><br>"
	);

	ui.lNumbersFormat->setToolTip(text);
	ui.cbNumbersFormat->setToolTip(text);

	connect(ui.chbHeader, SIGNAL(stateChanged(int)), SLOT(headerChanged(int)));
}

void AsciiOptionsWidget::showAsciiHeaderOptions(bool b) {
	ui.chbHeader->setVisible(b);
	ui.lVectorNames->setVisible(b);
	ui.kleVectorNames->setVisible(b);
}

void AsciiOptionsWidget::headerChanged(int state) {
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
	filter->setNumbersFormat( AbstractFileFilter::Locale(ui.cbNumbersFormat->currentIndex()) );
	filter->setDateTimeFormat(ui.cbDateTimeFormat->currentText());
	filter->setSimplifyWhitespacesEnabled( ui.chbSimplifyWhitespaces->isChecked() );
	filter->setSkipEmptyParts( ui.chbSkipEmptyParts->isChecked() );
	filter->setTransposed( ui.chbTranspose->isChecked() );
	filter->setVectorNames( ui.kleVectorNames->text() );
	filter->setHeaderEnabled( ui.chbHeader->isChecked() );
}


void AsciiOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");
	
	//TODO: check if this works (character gets currentItem?)
	ui.cbCommentCharacter->setCurrentItem(conf.readEntry("CommentCharacter", "#"));
	ui.cbSeparatingCharacter->setCurrentItem(conf.readEntry("SeparatingCharacter", "auto"));
	ui.cbNumbersFormat->setCurrentIndex(conf.readEntry("NumbersFormat", (int)AbstractFileFilter::LocaleSystem));
	ui.cbDateTimeFormat->setCurrentItem(conf.readEntry("DateTimeFormat", "hh:mm:ss"));
	ui.chbSimplifyWhitespaces->setChecked(conf.readEntry("SimplifyWhitespaces", true));
	ui.chbSkipEmptyParts->setChecked(conf.readEntry("SkipEmptyParts", false));
	ui.chbHeader->setChecked(conf.readEntry("UseFirstRow", true));
	ui.kleVectorNames->setText(conf.readEntry("Names", ""));
}

void AsciiOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	conf.writeEntry("CommentCharacter", ui.cbCommentCharacter->currentText());
	conf.writeEntry("SeparatingCharacter", ui.cbSeparatingCharacter->currentText());
	conf.writeEntry("NumbersFormat", ui.cbNumbersFormat->currentText());
	conf.writeEntry("DateTimeFormat", ui.cbDateTimeFormat->currentText());
	conf.writeEntry("SimplifyWhitespaces", ui.chbSimplifyWhitespaces->isChecked());
	conf.writeEntry("SkipEmptyParts", ui.chbSkipEmptyParts->isChecked());
	conf.writeEntry("UseFirstRow", ui.chbHeader->isChecked());
	conf.writeEntry("Names", ui.kleVectorNames->text());
}
