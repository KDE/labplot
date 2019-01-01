/***************************************************************************
    File                 : FitOptionsWidget.cc
    Project              : LabPlot
    Description          : widget for editing advanced fit options
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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
FitOptionsWidget::FitOptionsWidget(QWidget *parent, XYFitCurve::FitData* fitData, XYFitCurve* fitCurve) : QWidget(parent),
		m_fitData(fitData), m_fitCurve(fitCurve) {
	ui.setupUi(this);
	ui.pbApply->setIcon(QIcon::fromTheme("dialog-ok-apply"));
	ui.pbCancel->setIcon(QIcon::fromTheme("dialog-cancel"));

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
	ui.cbAutoRange->setChecked(m_fitData->autoRange);
	ui.cbAutoEvalRange->setChecked(m_fitData->autoEvalRange);
	ui.leMin->setText(QString::number(m_fitData->fitRange.first()));
	ui.leMax->setText(QString::number(m_fitData->fitRange.last()));
	ui.leEvalMin->setText(QString::number(m_fitData->evalRange.first()));
	ui.leEvalMax->setText(QString::number(m_fitData->evalRange.last()));
	this->autoRangeChanged();
	this->autoEvalRangeChanged();

	ui.cbUseDataErrors->setChecked(m_fitData->useDataErrors);
	ui.cbUseResults->setChecked(m_fitData->useResults);
	ui.cbPreview->setChecked(m_fitData->previewEnabled);

	//SLOTS
	connect(ui.leEps, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leMaxIterations, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leEvaluatedPoints, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.cbUseDataErrors, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbUseResults, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbPreview, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.pbApply, &QPushButton::clicked, this, &FitOptionsWidget::applyClicked);
	connect(ui.pbCancel, &QPushButton::clicked, this, &FitOptionsWidget::finished);
	connect(ui.cbAutoRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoRangeChanged);
	connect(ui.cbAutoEvalRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoEvalRangeChanged);
	connect(ui.leMin, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMinChanged);
	connect(ui.leMax, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMaxChanged);
	connect(ui.leEvalMin, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMinChanged);
	connect(ui.leEvalMax, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMaxChanged);
}

void FitOptionsWidget::autoRangeChanged() {
	const bool autoRange = ui.cbAutoRange->isChecked();
	m_fitData->autoRange = autoRange;

	if (autoRange) {
		ui.leMin->setEnabled(false);
		ui.lXRange->setEnabled(false);
		ui.leMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_fitCurve->xDataColumn();
		else {
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			ui.leMin->setText(QString::number(xDataColumn->minimum()));
			ui.leMax->setText(QString::number(xDataColumn->maximum()));
		}
	} else {
		ui.leMin->setEnabled(true);
		ui.lXRange->setEnabled(true);
		ui.leMax->setEnabled(true);
	}
}

void FitOptionsWidget::autoEvalRangeChanged() {
	const bool autoRange = ui.cbAutoEvalRange->isChecked();
	m_fitData->autoEvalRange = autoRange;

	if (autoRange) {
		ui.leEvalMin->setEnabled(false);
		ui.lEvalRange->setEnabled(false);
		ui.leEvalMax->setEnabled(false);

		const AbstractColumn* xDataColumn = nullptr;
		if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet)
			xDataColumn = m_fitCurve->xDataColumn();
		else {
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			ui.leEvalMin->setText(QString::number(xDataColumn->minimum()));
			ui.leEvalMax->setText(QString::number(xDataColumn->maximum()));
		}
	} else {
		ui.leEvalMin->setEnabled(true);
		ui.lEvalRange->setEnabled(true);
		ui.leEvalMax->setEnabled(true);
	}

}

void FitOptionsWidget::fitRangeMinChanged() {
	const double xMin = ui.leMin->text().toDouble();

	m_fitData->fitRange.first() = xMin;
	changed();
}
void FitOptionsWidget::fitRangeMaxChanged() {
	const double xMax = ui.leMax->text().toDouble();

	m_fitData->fitRange.last() = xMax;
	changed();
}

void FitOptionsWidget::evalRangeMinChanged() {
	const double xMin = ui.leEvalMin->text().toDouble();

	m_fitData->evalRange.first() = xMin;
	changed();
}
void FitOptionsWidget::evalRangeMaxChanged() {
	const double xMax = ui.leEvalMax->text().toDouble();

	m_fitData->evalRange.last() = xMax;
	changed();
}

void FitOptionsWidget::applyClicked() {
	m_fitData->maxIterations = ui.leMaxIterations->text().toFloat();
	m_fitData->eps = ui.leEps->text().toFloat();
	m_fitData->evaluatedPoints = ui.leEvaluatedPoints->text().toInt();
	m_fitData->useDataErrors = ui.cbUseDataErrors->isChecked();
	m_fitData->useResults = ui.cbUseResults->isChecked();
	m_fitData->previewEnabled = ui.cbPreview->isChecked();

	if (m_changed)
		emit optionsChanged();

	emit finished();
}

void FitOptionsWidget::changed() {
	m_changed = true;
}
