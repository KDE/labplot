/***************************************************************************
File                 : FITSHeaderEditAddUnitDialog.cpp
Project              : LabPlot
Description          : Widget for adding or modifying FITS header keyword units
--------------------------------------------------------------------
Copyright            : (C) 2016-2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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

#include <QDialogButtonBox>
#include <QCompleter>
#include <QPushButton>

FITSHeaderEditAddUnitDialog::FITSHeaderEditAddUnitDialog(const QString& unit, QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);
	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.horizontalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_okButton->setText(i18n("&Add"));

	setWindowTitle(i18nc("@title:window", "Add New Unit"));
	setWindowIcon(QIcon::fromTheme("document-new"));
	m_okButton->setEnabled(false);

	QCompleter* keyCompleter = new QCompleter(FITSFilter::units(), this);
	ui.leUnit->setCompleter(keyCompleter);
	ui.leUnit->setPlaceholderText(i18n("Enter unit name here"));

	connect(ui.leUnit, &QLineEdit::textChanged, this, &FITSHeaderEditAddUnitDialog::unitChanged);
	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FITSHeaderEditAddUnitDialog::close);

	connect(btnBox, &QDialogButtonBox::accepted, this, &FITSHeaderEditAddUnitDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FITSHeaderEditAddUnitDialog::reject);

	ui.leUnit->setText(unit);
}

QString FITSHeaderEditAddUnitDialog::unit() const {
	QString unit = ui.leUnit->text();
	if (unit.contains(QLatin1Char('(')))
		unit = unit.left(unit.indexOf(QLatin1Char('('))-1);

	return unit;
}

void FITSHeaderEditAddUnitDialog::unitChanged() {
	m_okButton->setEnabled(!ui.leUnit->text().isEmpty());
}
