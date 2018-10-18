/***************************************************************************
    File                 : SortDialog.h
    Project              : LabPlot
    Description          : Sorting options dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke@web.de)

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
#include "SortDialog.h"

#include <QPushButton>


/*!
	\class SortDialog
	\brief Dialog for sorting the columns in a spreadsheet.

	\ingroup kdefrontend
 */

SortDialog::SortDialog( QWidget* parent ) : QDialog( parent ) {

	setWindowIcon(QIcon::fromTheme("view-sort-ascending"));
	setWindowTitle(i18nc("@title:window", "Sort Columns"));
	setSizeGripEnabled(true);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Sort"));

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::sortColumns);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &SortDialog::reject);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::accept);

	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeType(int)));

	resize(minimumSize());
}

void SortDialog::sortColumns() {
	Column* leading;
	if (ui.cbType->currentIndex() == Together)
		leading = m_columns.at(ui.cbColumns->currentIndex());
	else
		leading = nullptr;

	emit sort(leading, m_columns, ui.cbOrdering->currentIndex() == Ascending);
}

void SortDialog::setColumns(QVector<Column*> columns) {
	m_columns = columns;

	for (auto col: m_columns)
		ui.cbColumns->addItem(col->name());

	ui.cbColumns->setCurrentIndex(0);

	if (m_columns.size() == 1) {
		ui.lType->hide();
		ui.cbType->hide();
		ui.lColumns->hide();
		ui.cbColumns->hide();
	}
}

void SortDialog::changeType(int Type) {
	if (Type == Together)
		ui.cbColumns->setEnabled(true);
	else
		ui.cbColumns->setEnabled(false);
}
