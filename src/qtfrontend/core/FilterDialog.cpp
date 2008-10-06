/***************************************************************************
    File                 : FilterDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Filter options dialog

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
#include "FilterDialog.h"
#include "MyParser.h"
#include "lib/ColorBox.h"
#include "analysis/FFTFilter.h"
#include "graph/Layer.h"

#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

FilterDialog::FilterDialog(int type, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	setWindowTitle(tr("Filter options"));
    filter_type = type;

    QGroupBox *gb1 = new QGroupBox();
    QGridLayout *gl1 = new QGridLayout(gb1);
	gl1->addWidget(new QLabel(tr("Filter curve: ")), 0, 0);

	boxName = new QComboBox();
	gl1->addWidget(boxName, 0, 1);

	if (type <= FFTFilter::HighPass)
		gl1->addWidget(new QLabel(tr("Frequency cutoff (Hz)")), 1, 0);
	else
		gl1->addWidget(new QLabel(tr("Low Frequency (Hz)")), 1, 0);

	boxStart = new QLineEdit();
	boxStart->setText(tr("0"));
	gl1->addWidget(boxStart, 1, 1);

	boxColor = new ColorBox(false);
	boxColor->setColor(QColor(Qt::red));
	if (type >= FFTFilter::BandPass)
		{
	    gl1->addWidget(new QLabel(tr("High Frequency (Hz)")), 2, 0);

		boxEnd = new QLineEdit();
		boxEnd->setText(tr("0"));
        gl1->addWidget(boxEnd, 2, 1);

		if (type == FFTFilter::BandPass)
		    gl1->addWidget(new QLabel(tr("Add DC Offset")), 3, 0);
		else
		    gl1->addWidget(new QLabel(tr("Substract DC Offset")), 3, 0);

		boxOffset = new QCheckBox();
		gl1->addWidget(boxOffset, 3, 1);

		gl1->addWidget(new QLabel(tr("Color")), 4, 0);
		gl1->addWidget(boxColor, 4, 1);
        gl1->setRowStretch(5, 1);
		}
    else
        {
        gl1->addWidget(new QLabel(tr("Color")), 2, 0);
		gl1->addWidget(boxColor, 2, 1);
        gl1->setRowStretch(3, 1);
        }

	buttonFilter = new QPushButton(tr( "&Filter" ));
    buttonFilter->setDefault( true );
    buttonCancel = new QPushButton(tr( "&Close" ));

    QVBoxLayout *vl = new QVBoxLayout();
 	vl->addWidget(buttonFilter);
	vl->addWidget(buttonCancel);
    vl->addStretch();

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->addWidget(gb1);
    hb->addLayout(vl);

	connect( buttonFilter, SIGNAL( clicked() ), this, SLOT( filter() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void FilterDialog::filter()
{
double from = 0.0, to = 0.0;
try
	{
	MyParser parser;
	parser.SetExpr(boxStart->text().replace(",", ".").toAscii().constData());
	from = parser.Eval();
	}
catch(mu::ParserError &e)
	{
	QMessageBox::critical(this, tr("Frequency input error"), QString::fromStdString(e.GetMsg()));
	boxStart->setFocus();
	return;
	}

if (from < 0)
		{
		QMessageBox::critical(this, tr("Frequency input error"),
				tr("Please enter positive frequency values!"));
		boxStart->setFocus();
		return;
		}

if (filter_type >= FFTFilter::BandPass)
	{
	try
		{
		MyParser parser;
		parser.SetExpr(boxEnd->text().replace(",", ".").toAscii().constData());
		to=parser.Eval();
		}
	catch(mu::ParserError &e)
		{
		QMessageBox::critical(this, tr("High Frequency input error"), QString::fromStdString(e.GetMsg()));
		boxEnd->setFocus();
		return;
		}

	if (to < 0)
		{
		QMessageBox::critical(this, tr("High Frequency input error"),
				tr("Please enter positive frequency values!"));
		boxEnd->setFocus();
		return;
		}

	if (from>=to)
		{
		QMessageBox::critical(this, tr("Frequency input error"),
				tr("Please enter frequency limits that satisfy: Low < High !"));
		boxEnd->setFocus();
		return;
		}
	}

FFTFilter *f = new FFTFilter((ApplicationWindow *)this->parent(), m_layer, boxName->currentText(), filter_type);
if (filter_type == FFTFilter::BandPass)
    {
    f->setBand(from, to);
    f->enableOffset(boxOffset->isChecked());
    }
else if (filter_type == FFTFilter::BandBlock)
    {
    f->setBand(from, to);
    f->enableOffset(!boxOffset->isChecked());
    }
else
    f->setCutoff(from);

f->setColor(boxColor->currentIndex());
f->run();
delete f;
}

void FilterDialog::setLayer(Layer *layer)
{
layer = layer;
boxName->addItems (layer->analysableCurvesList());
};
