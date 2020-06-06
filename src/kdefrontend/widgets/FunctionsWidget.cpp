/***************************************************************************
    File                 : FunctionsWidget.cc
    Project              : LabPlot
    Description          : widget for selecting functions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)

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
#include "FunctionsWidget.h"
#include "backend/gsl/ExpressionParser.h"
#include <QTimer>

/*!
	\class FunctionsWidget
	\brief Widget for selecting supported mathematical functions
	that can be used in expressions in \c ExpressionTextEdit.

	\ingroup kdefrontend
 */
FunctionsWidget::FunctionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	ui.bInsert->setIcon(QIcon::fromTheme("edit-paste"));
	ui.bCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
	m_expressionParser = ExpressionParser::getInstance();
	ui.cbGroup->addItems(m_expressionParser->functionsGroups());

	//SLOTS
	connect(ui.leFilter, &QLineEdit::textChanged, this, &FunctionsWidget::filterChanged);
	connect(ui.cbGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionsWidget::groupChanged);
	connect(ui.bInsert, &QPushButton::clicked, this, &FunctionsWidget::insertClicked);
	connect(ui.bCancel, &QPushButton::clicked, this, &FunctionsWidget::canceled);
	connect(ui.lwFunctions, &QListWidget::itemDoubleClicked, this, &FunctionsWidget::insertClicked);

	//set the focus to the search field and select the first group after the widget is shown
	QTimer::singleShot(0, this, [=] () { ui.leFilter->setFocus(); this->groupChanged(0); });
}

/*!
 * shows all functions of the selected group and selects the first one in the QStringList
 */
void FunctionsWidget::groupChanged(int index) {
	static const QStringList& functions = m_expressionParser->functions();
	static const QStringList& names = m_expressionParser->functionsNames();
	static const QVector<int>& indices = m_expressionParser->functionsGroupIndices();

	ui.lwFunctions->clear();
	for (int i = 0; i < names.size(); ++i) {
		if (indices.at(i) == index)
			ui.lwFunctions->addItem( names.at(i) + " (" + functions.at(i) + ')' );
	}
	ui.lwFunctions->setCurrentRow(0);
}

void FunctionsWidget::filterChanged(const QString& filter) {
	if ( !filter.isEmpty() ) {
		ui.cbGroup->setEnabled(false);

		static const QStringList& names = m_expressionParser->functionsNames();
		static const QStringList& functions = m_expressionParser->functions();
		ui.lwFunctions->clear();
		for (int i = 0; i < names.size(); ++i) {
			if (names.at(i).contains(filter, Qt::CaseInsensitive) || functions.at(i).contains(filter, Qt::CaseInsensitive))
				ui.lwFunctions->addItem( names.at(i) + " (" + functions.at(i) + ')' );
		}

		if (ui.lwFunctions->count()) {
			ui.lwFunctions->setCurrentRow(0);
			ui.bInsert->setEnabled(true);
		} else {
			ui.bInsert->setEnabled(false);
		}
	} else {
		ui.cbGroup->setEnabled(true);
		groupChanged(ui.cbGroup->currentIndex());
	}
}

void FunctionsWidget::insertClicked() {
	static const QStringList& functions = m_expressionParser->functions();
	static const QStringList& names = m_expressionParser->functionsNames();

	//determine the currently selected constant
	const QString& text = ui.lwFunctions->currentItem()->text();
	const QString& name = text.left( text.indexOf(" (") );
	int index = names.indexOf(name);

	emit functionSelected(functions.at(index));
}
