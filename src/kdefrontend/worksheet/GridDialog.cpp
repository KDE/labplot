/***************************************************************************
    File                 : GridDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2017 by Alexander Semke
    Email (use @ for *)  : alexander.semke@web.de
    Description          : dialog for editing the grid properties for the worksheet view

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

#include "GridDialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>
#include <QGridLayout>

#include <KLocalizedString>
#include <KColorButton>

//TODO:
//1. improve the layout and move the UI-part to a ui-file
//2. restore the dialog size
//3. restore the currently active grid settings

/**
 * @brief Provides a dialog for editing the grid properties for the worksheet view
 * \ingroup kdefrontend
 */
GridDialog::GridDialog(QWidget* parent) : QDialog(parent){
	setWindowTitle(i18nc("@title:window", "Custom Grid"));

	QWidget* widget = new QWidget;
	QGridLayout* layout = new QGridLayout(widget);

	QLabel* label = new QLabel(i18n("Style"), widget);
	layout->addWidget(label, 0, 0);

	cbStyle = new QComboBox(this);
	cbStyle->addItem(i18n("Lines"));
	cbStyle->addItem(i18n("Dots"));
	cbStyle->setCurrentIndex(0);
	layout->addWidget(cbStyle, 0, 1);

	label = new QLabel(i18n("Horizontal spacing"), widget);
	layout->addWidget(label, 1, 0);

	sbHorizontalSpacing = new QSpinBox(widget);
	sbHorizontalSpacing->setRange(1,100);
	sbHorizontalSpacing->setValue(10);
	layout->addWidget(sbHorizontalSpacing, 1, 1);

	label = new QLabel(i18n("Vertical spacing"), widget);
	layout->addWidget(label, 2, 0);

	sbVerticalSpacing = new QSpinBox(widget);
	sbVerticalSpacing->setRange(1,100);
	sbVerticalSpacing->setValue(10);
	layout->addWidget(sbVerticalSpacing, 2, 1);

	label = new QLabel(i18n("Color"), widget);
	layout->addWidget(label, 3, 0);

	kcbColor = new KColorButton(widget);
	kcbColor->setColor(Qt::gray);
	layout->addWidget(kcbColor , 3, 1);

	label = new QLabel(i18n("Opacity"), widget);
	layout->addWidget(label, 4, 0);

	sbOpacity = new QSpinBox(widget);
	sbOpacity->setRange(1,100);
	sbOpacity->setValue(100);
	layout->addWidget(sbOpacity, 4, 1);

	label = new QLabel("%", widget);
	layout->addWidget(label, 4, 2);


	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->addWidget(widget);
	vlayout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void GridDialog::save(WorksheetView::GridSettings& settings){
	if (cbStyle->currentIndex() == 0)
		settings.style  = WorksheetView::LineGrid;
	else
		settings.style  = WorksheetView::DotGrid;

	settings.horizontalSpacing = sbHorizontalSpacing->value();
	settings.verticalSpacing = sbVerticalSpacing->value();
	settings.color = kcbColor->color();
	settings.opacity = (float)sbOpacity->value()/100;
}
