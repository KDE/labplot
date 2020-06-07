/***************************************************************************
    File                 : GoToDialog.cpp
    Project              : LabPlot
    Description          : Dialog to provide the cell coordinates to navigate to
    --------------------------------------------------------------------
    Copyright            : (C) 2020 by Alexander Semke (alexander.semke@web.de)

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
#include "GoToDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class GoToDialog
	\brief Dialog to provide the cell coordinates to navigate to

	\ingroup kdefrontend
 */
GoToDialog::GoToDialog(QWidget* parent) : QDialog(parent) {
	setWindowTitle(i18nc("@title:window", "Go to Cell"));

	auto* layout = new QGridLayout(this);

	//row
	auto* label = new QLabel(i18n("Row:"));
	layout->addWidget(label, 0, 0);

	leRow = new QLineEdit(this);
	leRow->setValidator(new QIntValidator(leRow));
	leRow->setText("1");
	layout->addWidget(leRow, 0, 1);

	//column
	label = new QLabel(i18n("Column:"));
	layout->addWidget(label, 1, 0);

	leColumn = new QLineEdit(this);
	leColumn->setValidator(new QIntValidator(leColumn));
	leColumn->setText("1");
	layout->addWidget(leColumn, 1, 1);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btnBox, &QDialogButtonBox::accepted, this, &GoToDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &GoToDialog::reject);
	layout->addWidget(btnBox, 2, 1);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GoToDialog"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(200, 0).expandedTo(minimumSize()));
}

GoToDialog::~GoToDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GoToDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

int GoToDialog::row() {
	return leRow->text().toInt();
}


int GoToDialog::column() {
	return leColumn->text().toInt();
}
