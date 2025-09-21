/*
	File                 : FitOptionsWidget.cpp
	Project              : LabPlot
	Description          : widget for editing advanced fit options
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FitOptionsWidget.h"
#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"

/*!
	\class FitOptionsWidget
	\brief Widget for editing advanced fit options.

	\ingroup frontend
 */
FitOptionsWidget::FitOptionsWidget(QWidget* parent, XYFitCurve::FitData* fitData, XYFitCurve* fitCurve)
	: QWidget(parent)
	, m_fitData(fitData)
	, m_fitCurve(fitCurve) {
	ui.setupUi(this);
	ui.pbApply->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
	ui.pbCancel->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));

	// TODO: show "robust" option when robust fitting is possible
	// 	ui.cbRobust->addItem(i18n("on"));
	// 	ui.cbRobust->addItem(i18n("off"));
	ui.lRobust->setVisible(false);
	ui.cbRobust->setVisible(false);

	ui.leMaxIterations->setValidator(new QIntValidator(ui.leMaxIterations));
	ui.leEps->setValidator(new QDoubleValidator(ui.leEps));
	ui.leEvaluatedPoints->setValidator(new QIntValidator(ui.leEvaluatedPoints));

	const auto numberLocale = QLocale();
	ui.leMaxIterations->setText(numberLocale.toString(m_fitData->maxIterations));
	ui.leEps->setText(numberLocale.toString(m_fitData->eps));
	ui.leEvaluatedPoints->setText(numberLocale.toString(static_cast<qulonglong>(m_fitData->evaluatedPoints)));
	ui.sbConfidenceInterval->setLocale(numberLocale);

	// range widgets
	const auto* plot = static_cast<const CartesianPlot*>(fitCurve->parentAspect());
	const int xIndex = plot->coordinateSystem(m_fitCurve->coordinateSystemIndex())->index(CartesianCoordinateSystem::Dimension::X);
	m_dateTimeRange = (plot->xRangeFormat(xIndex) != RangeT::Format::Numeric);
	if (!m_dateTimeRange) {
		ui.leMin->setText(numberLocale.toString(m_fitData->fitRange.start()));
		ui.leMax->setText(numberLocale.toString(m_fitData->fitRange.end()));
		ui.leEvalMin->setText(numberLocale.toString(m_fitData->evalRange.start()));
		ui.leEvalMax->setText(numberLocale.toString(m_fitData->evalRange.end()));
	} else {
		ui.dateTimeEditMin->setMSecsSinceEpochUTC(m_fitData->fitRange.start());
		ui.dateTimeEditMax->setMSecsSinceEpochUTC(m_fitData->fitRange.end());
		ui.dateTimeEditEvalMin->setMSecsSinceEpochUTC(m_fitData->evalRange.start());
		ui.dateTimeEditEvalMax->setMSecsSinceEpochUTC(m_fitData->evalRange.end());
	}
	// changing data range not supported by ML
	if (fitData->algorithm == nsl_fit_algorithm_ml)
		ui.cbAutoRange->setEnabled(false);

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

	// auto range
	ui.cbAutoRange->setChecked(m_fitData->autoRange);
	ui.cbAutoEvalRange->setChecked(m_fitData->autoEvalRange);
	this->autoRangeChanged();
	this->autoEvalRangeChanged();

	ui.cbUseDataErrors->setChecked(m_fitData->useDataErrors);
	ui.cbUseResults->setChecked(m_fitData->useResults);
	ui.cbPreview->setChecked(m_fitData->previewEnabled);
	ui.sbConfidenceInterval->setValue(m_fitData->confidenceInterval);

	// SLOTS
	connect(ui.leEps, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leMaxIterations, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.leEvaluatedPoints, &QLineEdit::textChanged, this, &FitOptionsWidget::changed);
	connect(ui.cbUseDataErrors, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbUseResults, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.cbPreview, &QCheckBox::clicked, this, &FitOptionsWidget::changed);
	connect(ui.sbConfidenceInterval, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &FitOptionsWidget::changed);
	connect(ui.pbApply, &QPushButton::clicked, this, &FitOptionsWidget::applyClicked);
	connect(ui.pbCancel, &QPushButton::clicked, this, &FitOptionsWidget::finished);
	connect(ui.cbAutoRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoRangeChanged);
	connect(ui.cbAutoEvalRange, &QCheckBox::clicked, this, &FitOptionsWidget::autoEvalRangeChanged);
	connect(ui.leMin, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMinChanged);
	connect(ui.leMax, &QLineEdit::textChanged, this, &FitOptionsWidget::fitRangeMaxChanged);
	connect(ui.dateTimeEditMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &FitOptionsWidget::fitRangeMinDateTimeChanged);
	connect(ui.dateTimeEditMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &FitOptionsWidget::fitRangeMaxDateTimeChanged);
	connect(ui.leEvalMin, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMinChanged);
	connect(ui.leEvalMax, &QLineEdit::textChanged, this, &FitOptionsWidget::evalRangeMaxChanged);
	connect(ui.dateTimeEditEvalMin, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &FitOptionsWidget::evalRangeMinDateTimeChanged);
	connect(ui.dateTimeEditEvalMax, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &FitOptionsWidget::evalRangeMaxDateTimeChanged);
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

			const auto numberLocale = QLocale();
			if (!m_dateTimeRange) {
				ui.leMin->setText(numberLocale.toString(xMin));
				ui.leMax->setText(numberLocale.toString(xMax));
			} else {
				ui.dateTimeEditMin->setMSecsSinceEpochUTC(xMin);
				ui.dateTimeEditMax->setMSecsSinceEpochUTC(xMax);
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
		switch (m_fitCurve->dataSourceType()) {
		case XYAnalysisCurve::DataSourceType::Spreadsheet:
			xDataColumn = m_fitCurve->xDataColumn();
			break;
		case XYAnalysisCurve::DataSourceType::Curve:
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
			break;
		case XYAnalysisCurve::DataSourceType::Histogram:
			if (m_fitCurve->dataSourceHistogram())
				xDataColumn = m_fitCurve->dataSourceHistogram()->bins();
		}

		if (xDataColumn) {
			const double xMin = xDataColumn->minimum();
			const double xMax = xDataColumn->maximum();
			m_fitData->evalRange.setRange(xMin, xMax);

			const auto numberLocale = QLocale();
			if (!m_dateTimeRange) {
				ui.leEvalMin->setText(numberLocale.toString(xMin));
				ui.leEvalMax->setText(numberLocale.toString(xMax));
			} else {
				ui.dateTimeEditEvalMin->setMSecsSinceEpochUTC(xMin);
				ui.dateTimeEditEvalMax->setMSecsSinceEpochUTC(xMax);
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

void FitOptionsWidget::fitRangeMinDateTimeChanged(qint64 value) {
	m_fitData->fitRange.setStart(value);
	changed();
}

void FitOptionsWidget::fitRangeMaxDateTimeChanged(qint64 value) {
	m_fitData->fitRange.setEnd(value);
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

void FitOptionsWidget::evalRangeMinDateTimeChanged(qint64 value) {
	m_fitData->evalRange.setStart(value);
	changed();
}

void FitOptionsWidget::evalRangeMaxDateTimeChanged(qint64 value) {
	m_fitData->evalRange.setEnd(value);
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
		Q_EMIT optionsChanged();

	Q_EMIT finished();
}

void FitOptionsWidget::changed() {
	m_changed = true;
}
