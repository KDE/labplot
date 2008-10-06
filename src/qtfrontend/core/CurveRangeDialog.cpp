/***************************************************************************
    File                 : CurveRangeDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Curve range dialog

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
#include "CurveRangeDialog.h"
#include "graph/PlotCurve.h"
#include "graph/Layer.h"
#include "table/Table.h"

#include <QGroupBox>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>

CurveRangeDialog::CurveRangeDialog(QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle(tr("Plot range"));

    QGroupBox *gb1 = new QGroupBox();
    QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Data set: ")), 0, 0);

	boxName = new QLabel();
	gl1->addWidget(boxName, 0, 1);

	gl1->addWidget(new QLabel(tr("From row number")), 1, 0);
	boxStart = new QSpinBox();
	boxStart->setMinimum(1);
	gl1->addWidget(boxStart, 1, 1);

	gl1->addWidget(new QLabel(tr("To row number")), 2, 0);
	boxEnd = new QSpinBox();
	boxEnd->setMinimum(1);
    gl1->addWidget(boxEnd, 2, 1);
    gl1->setRowStretch(3, 1);

	buttonOK = new QPushButton(tr( "&OK" ));
    buttonOK->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

    QHBoxLayout *hl = new QHBoxLayout();
	hl->addStretch();
 	hl->addWidget(buttonOK);
	hl->addWidget(buttonCancel);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addWidget(gb1);
    vb->addLayout(hl);

	connect( buttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void CurveRangeDialog::accept()
{
	if (!m_curve)
		return;

	int start = boxStart->value() - 1;
	int end = boxEnd->value() - 1;
	m_curve->setRowRange(QMIN(start, end), QMAX(start, end));
	m_layer->updatePlot();
	m_layer->notifyChanges();
	close();
}

void CurveRangeDialog::setCurveToModify(Layer *layer, int curve)
{
	if (!layer)
		return;

	m_layer = layer;
	m_curve = (DataCurve *)m_layer->curve(curve);
	if (!m_curve)
		return;

    Table *t = m_curve->table();
	if (t)
	{
		boxStart->setMaxValue(t->rowCount());
		boxEnd->setMaxValue(t->rowCount());
	}

	boxName->setText(m_curve->title().text());
	boxStart->setValue(m_curve->startRow() + 1);
	boxEnd->setValue(m_curve->endRow() + 1);
}
