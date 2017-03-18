/***************************************************************************
    File                 : FitOptionsWidget.cc
    Project              : LabPlot
    Description          : widget for editing advanced fit options
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
#include "FitOptionsWidget.h"

/*!
	\class FitOptionsWidget
	\brief Widget for editing advanced fit options.

	\ingroup kdefrontend
 */
FitOptionsWidget::FitOptionsWidget(QWidget *parent, XYFitCurve::FitData* fitData): QWidget(parent), m_fitData(fitData), m_changed(false) {
	ui.setupUi(this);
	ui.pbApply->setIcon(KIcon("dialog-ok-apply"));
	ui.pbCancel->setIcon(KIcon("dialog-cancel"));

	//TODO: show "robust" option when robust fitting is possible
// 	ui.cbRobust->addItem(i18n("on"));
// 	ui.cbRobust->addItem(i18n("off"));
	ui.lRobust->setVisible(false);
	ui.cbRobust->setVisible(false);

	ui.leEps->setValidator( new QDoubleValidator(ui.leEps) );
	ui.leMaxIterations->setValidator( new QIntValidator(ui.leMaxIterations) );
	ui.leEvaluatedPoints->setValidator( new QIntValidator(ui.leEvaluatedPoints) );

	ui.leEps->setText(QString::number(m_fitData->eps));
	ui.leMaxIterations->setText(QString::number(m_fitData->maxIterations));
	ui.leEvaluatedPoints->setText(QString::number(m_fitData->evaluatedPoints));
	ui.cbEvaluateFullRange->setChecked(m_fitData->evaluateFullRange);
	ui.cbUseResults->setChecked(m_fitData->useResults);

	//SLOTS
	connect( ui.leEps, SIGNAL(textChanged(QString)), this, SLOT(changed()) ) ;
	connect( ui.leMaxIterations, SIGNAL(textChanged(QString)), this, SLOT(changed()) ) ;
	connect( ui.leEvaluatedPoints, SIGNAL(textChanged(QString)), this, SLOT(changed()) ) ;
	connect( ui.cbEvaluateFullRange, SIGNAL(clicked(bool)), this, SLOT(changed()) ) ;
	connect( ui.cbUseResults, SIGNAL(clicked(bool)), this, SLOT(changed()) ) ;
	connect( ui.pbApply, SIGNAL(clicked()), this, SLOT(applyClicked()) );
	connect( ui.pbCancel, SIGNAL(clicked()), this, SIGNAL(finished()) );
}

void FitOptionsWidget::applyClicked() {
	m_fitData->maxIterations = ui.leMaxIterations->text().toFloat();
	m_fitData->eps = ui.leEps->text().toFloat();
	m_fitData->evaluatedPoints = ui.leEvaluatedPoints->text().toInt();
	m_fitData->evaluateFullRange = ui.cbEvaluateFullRange->isChecked();
	m_fitData->useResults = ui.cbUseResults->isChecked();

	if (m_changed)
		emit optionsChanged();

	emit finished();
}

void FitOptionsWidget::changed() {
	m_changed = true;
}
