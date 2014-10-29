/***************************************************************************
    File                 : EquidistantNumbersDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating equidistant numbers
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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
#include "EquidistantNumbersDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

/*!
	\class EquidistantNumbersDialog
	\brief Dialog for equidistant numbers.

	\ingroup kdefrontend
 */

EquidistantNumbersDialog::EquidistantNumbersDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_spreadsheet(s) {

	setWindowTitle(i18n("Equidistant numbers"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	ui.cbType->addItem(i18n("Number"));
	ui.cbType->addItem(i18n("Increment"));

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonText(KDialog::Ok, i18n("&Generate"));
	setButtonToolTip(KDialog::Ok, i18n("Generate equidistant numbers"));

	ui.kleFrom->setClearButtonShown(true);
	ui.kleTo->setClearButtonShown(true);
	ui.kleIncrement->setClearButtonShown(true);
	ui.kleNumber->setClearButtonShown(true);

	ui.kleFrom->setValidator( new QDoubleValidator(ui.kleFrom) );
	ui.kleTo->setValidator( new QDoubleValidator(ui.kleTo) );
	ui.kleIncrement->setValidator( new QDoubleValidator(ui.kleIncrement) );
	ui.kleNumber->setValidator( new QIntValidator(ui.kleNumber) );

	ui.kleFrom->setText("1");
	ui.kleTo->setText("100");
	ui.kleIncrement->setText("1");

	connect( ui.cbType, SIGNAL(currentIndexChanged(int)), SLOT(typeChanged(int)) );
	connect( ui.kleFrom, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect( ui.kleTo, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect( ui.kleNumber, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect( ui.kleIncrement, SIGNAL(textChanged(QString)), this, SLOT(checkValues()) );
	connect(this, SIGNAL(okClicked()), this, SLOT(generate()));

	//generated data the  default
	this->typeChanged(0);

	resize( QSize(300,0).expandedTo(minimumSize()) );
}

void EquidistantNumbersDialog::setColumns(QList<Column*> list) {
	m_columns = list;
	ui.kleNumber->setText( QString::number(m_columns.first()->rowCount()) );
}

void EquidistantNumbersDialog::typeChanged(int index) {
	if (index==0) { //fixed number
		ui.lIncrement->hide();
		ui.kleIncrement->hide();
		ui.lNumber->show();
		ui.kleNumber->show();
	} else { //fixed increment
		ui.lIncrement->show();
		ui.kleIncrement->show();
		ui.lNumber->hide();
		ui.kleNumber->hide();
	}
}

void EquidistantNumbersDialog::checkValues() {
	if (ui.kleFrom->text().simplified().isEmpty()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	if (ui.kleTo->text().simplified().isEmpty()) {
		enableButton(KDialog::Ok, false);
		return;
	}

	if (ui.cbType->currentIndex() == 0) {
		if (ui.kleNumber->text().simplified().isEmpty() || ui.kleNumber->text().simplified().toInt()==0) {
			enableButton(KDialog::Ok, false);
			return;
		}
	} else {
		if (ui.kleIncrement->text().simplified().isEmpty() || qFuzzyIsNull(ui.kleIncrement->text().simplified().toDouble())) {
			enableButton(KDialog::Ok, false);
			return;
		}
	}

	enableButton(KDialog::Ok, true);
}

void EquidistantNumbersDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with equidistant numbers",
									"%1: fill columns with equidistant numbers",
									m_spreadsheet->name(),
									m_columns.size()));

	double start  = ui.kleFrom->text().toDouble();
	double end  = ui.kleTo->text().toDouble();
	int number;
	double dist;
	if (ui.cbType->currentIndex()==0) { //fixed number
		number = ui.kleNumber->text().toInt();
		if (number!=1)
			dist = (end - start)/ (number - 1);
		else
			dist = 0;
	} else { //fixed increment
		dist = ui.kleIncrement->text().toDouble();
		number = (end-start)/dist + 1;
	}

	if (m_spreadsheet->rowCount()<number)
		m_spreadsheet->setRowCount(number);

	foreach(Column* col, m_columns) {
		col->setSuppressDataChangedSignal(true);

		if (m_spreadsheet->rowCount()>number)
			col->clear();

		for (int i=0; i<number; ++i) {
			col->setValueAt(i, start + dist*i);
		}

		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
