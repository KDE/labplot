/***************************************************************************
    File                 : MatrixDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Matrix properties dialog

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
#include "MatrixDialog.h"
#include "Matrix.h"

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QCloseEvent>

MatrixDialog::MatrixDialog( Matrix *matrix, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    m_matrix(matrix)
{
    setWindowTitle( tr( "Matrix Properties" ) );

	QGridLayout * topLayout = new QGridLayout();
	QHBoxLayout * bottomLayout = new QHBoxLayout();

	topLayout->addWidget( new QLabel(tr( "Cell Width" )), 0, 0 );
	boxColWidth = new QSpinBox();
	boxColWidth->setRange(0,1000);
	boxColWidth->setSingleStep(10);
	topLayout->addWidget( boxColWidth, 0, 1 );

	topLayout->addWidget( new QLabel(tr( "Data Format" )), 1, 0 );
	boxFormat = new QComboBox();
    boxFormat->addItem( tr( "Decimal: 1000" ) );
	boxFormat->addItem( tr( "Scientific: 1E3" ) );

	topLayout->addWidget( boxFormat, 1, 1 );

	topLayout->addWidget( new QLabel( tr( "Decimal digits" )), 2, 0 );
	boxPrecision = new QSpinBox();
	boxPrecision->setRange(0,16);
	topLayout->addWidget( boxPrecision, 2, 1 );

	buttonApply = new QPushButton(tr( "&Apply" ));
	buttonApply->setAutoDefault( true );
	bottomLayout->addWidget( buttonApply );

	buttonOk = new QPushButton(tr( "&OK" ));
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );
	bottomLayout->addWidget( buttonOk );

	buttonCancel = new QPushButton(tr( "&Cancel" ));
	buttonCancel->setAutoDefault( true );
	bottomLayout->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomLayout);

	m_initial_col_width = matrix->columnsWidth();
    boxColWidth->setValue(matrix->columnsWidth());

    if (matrix->textFormat() == 'f')
		boxFormat->setCurrentIndex(0);
	else
		boxFormat->setCurrentIndex(1);

	boxPrecision->setValue(matrix->precision());

    matrix->saveCellsToMemory();
	
	// signals and slots connections
    connect(boxColWidth, SIGNAL(valueChanged(int)), m_matrix, SLOT(setColumnsWidth(int)));
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( cancel() ) );
}

void MatrixDialog::changePrecision(int precision)
{
    if (boxFormat->currentIndex() == 1)
		m_matrix->setNumericFormat('e', precision);
	else
		m_matrix->setNumericFormat('f', precision);
}

void MatrixDialog::apply()
{
	m_matrix->setColumnsWidth(boxColWidth->value());
    changePrecision(boxPrecision->value());
	m_initial_col_width = m_matrix->columnsWidth();
}

void MatrixDialog::cancel()
{
    m_matrix->setColumnsWidth(m_initial_col_width);
	close();
}

void MatrixDialog::accept()
{
	apply();
	close();
}

void MatrixDialog::closeEvent(QCloseEvent* e)
{
    m_matrix->forgetSavedCells();
    e->accept();
}
