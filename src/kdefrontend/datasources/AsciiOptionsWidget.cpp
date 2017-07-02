/***************************************************************************
    File                 : AsciiOptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of ascii data
    --------------------------------------------------------------------
    Copyright            : (C) 2009 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2009-2017 by Alexander Semke (alexander.semke@web.de)

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
#include <KLocale>

 /*!
	\class AsciiOptionsWidget
	\brief Widget providing options for the import of ascii data

	\ingroup kdefrontend
 */
AsciiOptionsWidget::AsciiOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

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
}
