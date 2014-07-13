/***************************************************************************
    File                 : ConstantsWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke*web.de)
    Description          : widget for selecting constants

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
#include "ConstantsWidget.h"
#include "backend/gsl/ExpressionParser.h"

/*!
	\class ConstantsWidget
	\brief Widget for selecting supported mathematical and physical constants
	that can be used in expressions in \c ExpressionTextEdit.

	\ingroup kdefrontend
 */
ConstantsWidget::ConstantsWidget(QWidget *parent): QWidget(parent) {
	ui.setupUi(this);
	ui.bInsert->setIcon(KIcon("edit-paste"));
	ui.bCancel->setIcon(KIcon("dialog-cancel"));
	m_expressionParser = ExpressionParser::getInstance();
	ui.cbGroup->addItems(m_expressionParser->constantsGroups());

	//SLOTS
	connect( ui.kleFilter, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)) );
	connect( ui.cbGroup, SIGNAL(currentIndexChanged(int)), this, SLOT(groupChanged(int)) );
	connect( ui.lwConstants, SIGNAL(currentTextChanged(QString)), this, SLOT(constantChanged(QString)) );
	connect( ui.bInsert, SIGNAL(clicked(bool)), this, SLOT(insertClicked()) );
	connect( ui.bCancel, SIGNAL(clicked(bool)), this, SIGNAL(canceled()) );
	connect( ui.lwConstants, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(insertClicked()) );

	this->groupChanged(0);
}

/*!
 * shows all constants of the selected group and selects the first one in the QStringList
 */
void ConstantsWidget::groupChanged(int index) {
	static const QStringList& constants = m_expressionParser->constants();
	static const QStringList& names = m_expressionParser->constantsNames();
	static const QVector<int>& indices = m_expressionParser->constantsGroupIndices();

	ui.lwConstants->clear();
	for (int i=0; i<names.size(); ++i) {
		if (indices.at(i) == index)
			ui.lwConstants->addItem( names.at(i) + " (" + constants.at(i) + ")" );
	}
	ui.lwConstants->setCurrentRow(0);
}

void ConstantsWidget::filterChanged(const QString& filter) {
	if ( !filter.isEmpty() ) {
		ui.cbGroup->setEnabled(false);

		static const QStringList& names = m_expressionParser->constantsNames();
		static const QStringList& constants = m_expressionParser->constants();
		ui.lwConstants->clear();
		for (int i=0; i<names.size(); ++i) {
			if (names.at(i).contains(filter, Qt::CaseInsensitive))
				ui.lwConstants->addItem( names.at(i) + " (" + constants.at(i) + ")" );
		}

		if (ui.lwConstants->count()) {
			ui.lwConstants->setCurrentRow(0);
			ui.bInsert->setEnabled(true);
		} else {
			ui.kleValue->setText("");
			ui.lUnit->setText("");
			ui.bInsert->setEnabled(false);
		}
	} else {
		ui.cbGroup->setEnabled(true);
		groupChanged(ui.cbGroup->currentIndex());
	}
}

void ConstantsWidget::constantChanged(const QString& text) {
	static const QStringList& names = m_expressionParser->constantsNames();
	static const QStringList& values = m_expressionParser->constantsValues();
	static const QStringList& units = m_expressionParser->constantsUnits();

	QString name = text.left( text.indexOf(" (") );
	int index = names.indexOf(name);
	if (index!=-1){
		ui.kleValue->setText(values.at(index));
		ui.kleValue->setCursorPosition(0);
		ui.lUnit->setText(units.at(index));
	}
}

void ConstantsWidget::insertClicked() {
	static const QStringList& constants = m_expressionParser->constants();
	static const QStringList& names = m_expressionParser->constantsNames();

	//determine the currently selected constant
	const QString& text = ui.lwConstants->currentItem()->text();
	const QString& name = text.left( text.indexOf(" (") );
	int index = names.indexOf(name);
	const QString& constant = constants.at(index);

	emit(constantSelected(constant));
}
