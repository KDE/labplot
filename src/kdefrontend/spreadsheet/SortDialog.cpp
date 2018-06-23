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
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QLayout>
#include <KLocalizedString>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>


/*!
	\class SortDialog
	\brief Dialog for sorting the columns in a spreadsheet.

	\ingroup kdefrontend
 */

SortDialog::SortDialog( QWidget* parent, Qt::WFlags fl ) : QDialog( parent, fl ){

	setWindowIcon(QIcon::fromTheme("view-sort-ascending"));
	setWindowTitle(i18nc("@title:window", "Sort Columns"));
	setSizeGripEnabled(true);
    setAttribute(Qt::WA_DeleteOnClose);

	QGroupBox* widget = new QGroupBox(i18n("Options"));
	QGridLayout* layout = new QGridLayout(widget);
	layout->setSpacing(4);
	layout->setContentsMargins(4,4,4,4);

	layout->addWidget( new QLabel( i18n("Order")), 0, 0 );
    m_cbOrdering = new QComboBox();
    m_cbOrdering->addItem(QIcon::fromTheme("view-sort-ascending"), i18n("Ascending"));
    m_cbOrdering->addItem(QIcon::fromTheme("view-sort-descending"), i18n("Descending"));
    layout->addWidget(m_cbOrdering, 0, 1 );

    m_lType = new QLabel(i18n("Sort columns"));
    layout->addWidget( m_lType, 1, 0 );
    m_cbType = new QComboBox();
    m_cbType->addItem(i18n("Separately"));
    m_cbType->addItem(i18n("Together"));
    layout->addWidget(m_cbType, 1, 1 );
    m_cbType->setCurrentIndex(Together);

    m_lColumns = new QLabel(i18n("Leading column"));
    layout->addWidget( m_lColumns, 2, 0 );
    m_cbColumns = new QComboBox();
    layout->addWidget(m_cbColumns, 2, 1);
	layout->setRowStretch(3, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Sort"));

	connect(buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::sortColumns);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &SortDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &SortDialog::accept);

	layout->addWidget(buttonBox);

	setLayout(layout);

    connect(m_cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeType(int)));

	this->resize(400,0);
}

void SortDialog::sortColumns(){
	Column* leading;
    if(m_cbType->currentIndex() == Together)
        leading = m_columns.at(m_cbColumns->currentIndex());
	else
		leading = 0;

    emit sort(leading, m_columns, m_cbOrdering->currentIndex() == Ascending );
}

void SortDialog::setColumns(QVector<Column*> columns){
	m_columns = columns;

	for(int i=0; i<m_columns.size(); i++)
        m_cbColumns->addItem( m_columns.at(i)->name() );

    m_cbColumns->setCurrentIndex(0);

	if (m_columns.size() == 1){
        m_lType->hide();
        m_cbType->hide();
        m_lColumns->hide();
        m_cbColumns->hide();
	}
}

void SortDialog::changeType(int Type){
	if(Type == Together)
        m_cbColumns->setEnabled(true);
	else
        m_cbColumns->setEnabled(false);
}
