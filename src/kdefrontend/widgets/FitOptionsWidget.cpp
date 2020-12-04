/***************************************************************************
    File                 : FitOptionsWidget.cc
    Project              : LabPlot
    Description          : widget for editing advanced fit options
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2020 Alexander Semke (alexander.semke@web.de)
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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

/*!
	\class FitOptionsWidget
	\brief Widget for editing advanced fit options.

	\ingroup kdefrontend
 */
FitOptionsWidget::FitOptionsWidget(QWidget* parent, XYFitCurve::FitData* fitData, XYFitCurve* fitCurve) : QWidget(parent),
	m_fitData(fitData), m_fitCurve(fitCurve) {
	ui.setupUi(this);
	ui.pbApply->setIcon(QIcon::fromTheme("dialog-ok-apply"));
	ui.pbCancel->setIcon(QIcon::fromTheme("dialog-cancel"));

	//TODO: show "robust" option when robust fitting is possible
// 	ui.cbRobust->addItem(i18n("on"));
// 	ui.cbRobust->addItem(i18n("off"));
	ui.lRobust->setVisible(false);
	ui.cbRobust->setVisible(false);

	ui.leMaxIterations->setValidator( new QIntValidator(ui.leMaxIterations) );
	ui.leEps->setValidator( new QDoubleValidator(ui.leEps) );
	ui.leEvaluatedPoints->setValidator( new QIntValidator(ui.leEvaluatedPoints) );

	SET_NUMBER_LOCALE
	ui.leMaxIterations->setText(numberLocale.toString(m_fitData->maxIterations));
	ui.leEps->setText(numberLocale.toString(m_fitData->eps));
	ui.leEvaluatedPoints->setText(numberLocale.toString(static_cast<qulonglong>(m_fitData->evaluatedPoints)));
	ui.sbConfidenceInterval->setLocale(numberLocale);

	//range widgets
	const auto* plot = static_cast<const CartesianPlot*>(fitCurve->parentAspect());
	m_dateTimeRange = (plot->xRangeFormat() != CartesianPlot::RangeFormat::Numeric);
	if (!m_dateTimeRange) {
		ui.leMin->setText(numberLocale.toString(m_fitData->fitRange.start()));
		ui.leMax->setText(numberLocale.toString(m_fitData->fitRange.end()));
		ui.leEvalMin->setText(numberLocale.toString(m_fitData->evalRange.start()));
		ui.leEvalMax->setText(numberLocale.toString(m_fitData->evalRange.end()));
	} else {
		ui.dateTimeEditMin->setDateTime( QDateTime::fromMSecsSinceEpoch(m_fitData->fitRange.start()) );
		ui.dateTimeEditMax->setDateTime( QDateTime::fromMSecsSinceEpoch(m_fitData->fitRange.end()) );
		ui.dateTimeEditEvalMin->setDateTime( QDateTime::fromMSecsSinceEpoch(m_fitData->evalRange.start()) );
		ui.dateTimeEditEvalMax->setDateTime( QDateTime::fromMSecsSinceEpoch(m_fitData->evalRange.end()) );
	}

	ui.leMin->setVisible(!m_dateTimeRange);
	ui.leMax->setVisible(!m_dateTimeRange);
	ui.lXRange->setVisible(!m_dateTimeRange);
	ui.leEvalMin->setVisible(!m_dateTimeRange);
	ui.leEvalMax->setVisible(!m_dateTimeRange);
	ui.lEvalRange->setVisible(!m_dateTimeRange);
	ui.dateTimeEditMin->setVisible(m_dateTimeRange);
	ui.dateTimeEditMax->setVisible(m_dateTimeRange);
	ui.lXRangeDateTime->setVisible(m_dateTimeRange);
	ui.dateTimeEditEvalMin->setVisible(m_dateTimeRange);
	ui.dateTimeEditEvalMax->setVisible(m_dateTimeRange);
	ui.lEvalRangeDateTime->setVisible(m_dateTimeRange);

	//auto range
	ui.cbAutoRange->setChecked(m_fitData->autoRange);
	ui.cbAutoEvalRange->setChecked(m_fitData->autoEvalRange);
	this->autoRangeChanged();
	this->autoEvalRangeChanged();

	ui.cbUseDataErrors->setChecked(m_fitData->useDataErrors);
	ui.cbUseResults->setChecked(m_fitData->useResults);
	ui.cbPreview->setChecked(m_fitData->previewEnabled);
	ui.sbConfidenceInterval->setValue(m_fitData->confidenceInterval);

	//SLOTS
	connect(ui.leEps, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leMaxIterations, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leEvaluatedPoints, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.cbUseDataErrors, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbUseResults, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbPreview, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.sbConfidenceInterval, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FitOptionsWidget::changed);
	connect(ui.pbApply, &QPushButton::clicked, this, &FitOptionsWidget::applyClicked);
	connect(ui.pbCancel, &QPushButton::clicked, this, &FitOptionsWidget::finished);
	connect(ui.cbAutoRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoRangeChanged);
	connect(ui.cbAutoEvalRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoEvalRangeChanged);
	connect(ui.leMin, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMinChanged);
	connect(ui.leMax, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMaxChanged);
	connect(ui.dateTimeEditMin, &QDateTimeEdit::dateTimeChanged, this, &FitOptionsWidget::fitRangeMinDateTimeChanged);
	connect(ui.dateTimeEditMax, &QDateTimeEdit::dateTimeChanged, this, &FitOptionsWidget::fitRangeMaxDateTimeChanged);
	connect(ui.leEvalMin, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMinChanged);
	connect(ui.leEvalMax, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMaxChanged);
	connect(ui.dateTimeEditEvalMin, &QDateTimeEdit::dateTimeChanged, this, &FitOptionsWidget::evalRangeMinDateTimeChanged);
	connect(ui.dateTimeEditEvalMax, &QDateTimeEdit::dateTimeChanged, this, &FitOptionsWidget::evalRangeMaxDateTimeChanged);
}

void FitOptionsWidget::autoRangeChanged() {
	const bool autoRange = ui.cbAutoRange->isChecked();
	m_fitData->autoRange = autoRange;

	ui.leMin->setEnabled(!autoRange);
	ui.lXRange->setEnabled(!autoRange);
	ui.leMax->setEnabled(!autoRange);
	ui.dateTimeEditMin->setEnabled(!autoRange);
	ui.lXRange->setEnabled(!autoRange);
	ui.dateTimeEditMax->setEnabled(!autoRange);

	if (autoRange) {
		const AbstractColumn* xDataColumn = nullptr;
		if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_fitCurve->xDataColumn();
		else {
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			const double xMin = xDataColumn->minimum();
			const double xMax = xDataColumn->maximum();
			m_fitData->fitRange.setRange(xMin, xMax);

			SET_NUMBER_LOCALE
			if (!m_dateTimeRange) {
				ui.leMin->setText(numberLocale.toString(xMin));
				ui.leMax->setText(numberLocale.toString(xMax));
			} else {
				ui.dateTimeEditMin->setDateTime(QDateTime::fromMSecsSinceEpoch(xMin));
				ui.dateTimeEditMax->setDateTime(QDateTime::fromMSecsSinceEpoch(xMax));
			}
		}
	}
}

void FitOptionsWidget::autoEvalRangeChanged() {
	const bool autoRange = ui.cbAutoEvalRange->isChecked();
	m_fitData->autoEvalRange = autoRange;

	ui.leEvalMin->setEnabled(!autoRange);
	ui.lEvalRange->setEnabled(!autoRange);
	ui.leEvalMax->setEnabled(!autoRange);
	ui.dateTimeEditEvalMin->setEnabled(!autoRange);
	ui.lEvalRange->setEnabled(!autoRange);
	ui.dateTimeEditEvalMax->setEnabled(!autoRange);

	if (autoRange) {
		const AbstractColumn* xDataColumn = nullptr;
		if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet)
			xDataColumn = m_fitCurve->xDataColumn();
		else {
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			const double xMin = xDataColumn->minimum();
			const double xMax = xDataColumn->maximum();
			m_fitData->evalRange.setRange(xMin, xMax);

			SET_NUMBER_LOCALE
			if (!m_dateTimeRange) {
				ui.leEvalMin->setText(numberLocale.toString(xMin));
				ui.leEvalMax->setText(numberLocale.toString(xMax));
			} else {
				ui.dateTimeEditEvalMin->setDateTime(QDateTime::fromMSecsSinceEpoch(xMin));
				ui.dateTimeEditEvalMax->setDateTime(QDateTime::fromMSecsSinceEpoch(xMax));
			}
		}
	}
}

void FitOptionsWidget::fitRangeMinChanged() {
	SET_DOUBLE_FROM_LE(m_fitData->fitRange.start(), ui.leMin);
	changed();
}
void FitOptionsWidget::fitRangeMaxChanged() {
	SET_DOUBLE_FROM_LE(m_fitData->fitRange.end(), ui.leMax);
	changed();
}

void FitOptionsWidget::fitRangeMinDateTimeChanged(const QDateTime& dateTime) {
	m_fitData->fitRange.setMin(dateTime.toMSecsSinceEpoch());
	changed();
}

void FitOptionsWidget::fitRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	m_fitData->fitRange.setMax(dateTime.toMSecsSinceEpoch());
	changed();
}

void FitOptionsWidget::evalRangeMinChanged() {
	SET_DOUBLE_FROM_LE(m_fitData->evalRange.start(), ui.leEvalMin);
	changed();
}
void FitOptionsWidget::evalRangeMaxChanged() {
	SET_DOUBLE_FROM_LE(m_fitData->evalRange.end(), ui.leEvalMax);
	changed();
}

void FitOptionsWidget::evalRangeMinDateTimeChanged(const QDateTime& dateTime) {
	m_fitData->evalRange.setMin(dateTime.toMSecsSinceEpoch());
	changed();
}

void FitOptionsWidget::evalRangeMaxDateTimeChanged(const QDateTime& dateTime) {
	m_fitData->evalRange.setMax(dateTime.toMSecsSinceEpoch());
	changed();
}

void FitOptionsWidget::applyClicked() {
	SET_INT_FROM_LE(m_fitData->maxIterations, ui.leMaxIterations);
	SET_DOUBLE_FROM_LE(m_fitData->eps, ui.leEps);
	SET_INT_FROM_LE(m_fitData->evaluatedPoints, ui.leEvaluatedPoints);

	m_fitData->useDataErrors = ui.cbUseDataErrors->isChecked();
	m_fitData->useResults = ui.cbUseResults->isChecked();
	m_fitData->previewEnabled = ui.cbPreview->isChecked();
	m_fitData->confidenceInterval = ui.sbConfidenceInterval->value();

	if (m_changed)
		emit optionsChanged();

	emit finished();
}

void FitOptionsWidget::changed() {
	m_changed = true;
}
