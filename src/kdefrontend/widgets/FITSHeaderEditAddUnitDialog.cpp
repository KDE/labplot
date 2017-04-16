/***************************************************************************
File                 : FITSHeaderEditAddUnitDialog.cpp
Project              : LabPlot
Description          : Widget for adding or modifying FITS header keyword units
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#include "FITSHeaderEditAddUnitDialog.h"
#include "backend/datasources/filters/FITSFilter.h"

FITSHeaderEditAddUnitDialog::FITSHeaderEditAddUnitDialog(const QString& unit, QWidget* parent) : KDialog(parent) {
	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget(mainWidget);

	setWindowTitle(i18n("Add New Unit"));
	setWindowIcon(KIcon("document-new"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setButtonText(KDialog::Ok, i18n("&Add"));
	enableButtonOk(false);

	KCompletion* keyCompletion = new KCompletion;
	keyCompletion->setItems(FITSFilter::units());
	ui.kleUnit->setCompletionObject(keyCompletion);
	ui.kleUnit->setAutoDeleteCompletionObject(true);
	ui.kleUnit->setPlaceholderText(i18n("Enter unit name here"));

	connect(ui.kleUnit, SIGNAL(textChanged(QString)), this, SLOT(unitChanged()));
	connect(ui.kleUnit, SIGNAL(clearButtonClicked()), this, SLOT(unitChanged()));

	ui.kleUnit->setText(unit);
}

QString FITSHeaderEditAddUnitDialog::unit() const {
	QString unit = ui.kleUnit->text();
	if (unit.contains(QLatin1Char('(')))
		unit = unit.left(unit.indexOf(QLatin1Char('('))-1);

	return unit;
}

void FITSHeaderEditAddUnitDialog::unitChanged() {
	enableButtonOk(!ui.kleUnit->text().isEmpty());
}
