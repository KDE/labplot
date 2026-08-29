/*
	File                 : XYPiecewiseLinearFitCurveDock.cpp
	Project              : LabPlot
	Description          : widget for editing properties of piecewise linear fit curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYPiecewiseLinearFitCurveDock.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QVBoxLayout>

XYPiecewiseLinearFitCurveDock::XYPiecewiseLinearFitCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
}

void XYPiecewiseLinearFitCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, uiGeneralTab.pbRecalculate, uiGeneralTab.cbAutoRecalculate, uiGeneralTab.cbDataSourceType);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 6, 2, 1, 2);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 7, 2, 1, 2);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 8, 2, 1, 2);

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	retranslateUi();

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYPiecewiseLinearFitCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYPiecewiseLinearFitCurveDock::methodChanged);
	connect(uiGeneralTab.sbPenalty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::penaltyChanged);
	connect(uiGeneralTab.sbMinSegmentSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::minSegmentSizeChanged);
	connect(uiGeneralTab.sbMaxChangepoints, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::maxChangepointsChanged);
	connect(uiGeneralTab.sbEvaluatedPoints, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::evaluatedPointsChanged);
	connect(uiGeneralTab.cbAutoRange, &QCheckBox::toggled, this, &XYPiecewiseLinearFitCurveDock::autoRangeChanged);
	connect(uiGeneralTab.leFitRangeMin, &QLineEdit::textChanged, this, &XYPiecewiseLinearFitCurveDock::fitRangeMinChanged);
	connect(uiGeneralTab.leFitRangeMax, &QLineEdit::textChanged, this, &XYPiecewiseLinearFitCurveDock::fitRangeMaxChanged);
	connect(uiGeneralTab.cbAutoEvalRange, &QCheckBox::toggled, this, &XYPiecewiseLinearFitCurveDock::autoEvalRangeChanged);
	connect(uiGeneralTab.leEvalRangeMin, &QLineEdit::textChanged, this, &XYPiecewiseLinearFitCurveDock::evalRangeMinChanged);
	connect(uiGeneralTab.leEvalRangeMax, &QLineEdit::textChanged, this, &XYPiecewiseLinearFitCurveDock::evalRangeMaxChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYPiecewiseLinearFitCurveDock::recalculateClicked);

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYPiecewiseLinearFitCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYPiecewiseLinearFitCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYPiecewiseLinearFitCurveDock::yDataColumnChanged);
}

void XYPiecewiseLinearFitCurveDock::initGeneralTab() {
	// show the properties of the first curve
	// data source
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_fitCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_fitCurve->dataSourceCurve());
	cbXDataColumn->setAspect(m_fitCurve->xDataColumn(), m_fitCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_fitCurve->yDataColumn(), m_fitCurve->yDataColumnPath());

	uiGeneralTab.cbMethod->setCurrentIndex(static_cast<int>(m_fitData.changepointMethod));
	uiGeneralTab.sbPenalty->setValue(m_fitData.penalty);
	uiGeneralTab.sbMinSegmentSize->setValue(m_fitData.minSegmentSize);
	uiGeneralTab.sbMaxChangepoints->setValue(m_fitData.maxChangepoints);
	uiGeneralTab.sbEvaluatedPoints->setValue(m_fitData.evaluatedPoints);
	uiGeneralTab.cbAutoRange->setChecked(m_fitData.autoRange);
	uiGeneralTab.leFitRangeMin->setText(QString::number(m_fitData.fitRange.start()));
	uiGeneralTab.leFitRangeMax->setText(QString::number(m_fitData.fitRange.end()));
	uiGeneralTab.cbAutoEvalRange->setChecked(m_fitData.autoEvalRange);
	uiGeneralTab.leEvalRangeMin->setText(QString::number(m_fitData.evalRange.start()));
	uiGeneralTab.leEvalRangeMax->setText(QString::number(m_fitData.evalRange.end()));

	uiGeneralTab.leFitRangeMin->setEnabled(!m_fitData.autoRange);
	uiGeneralTab.leFitRangeMax->setEnabled(!m_fitData.autoRange);
	uiGeneralTab.leEvalRangeMin->setEnabled(!m_fitData.autoEvalRange);
	uiGeneralTab.leEvalRangeMax->setEnabled(!m_fitData.autoEvalRange);

	showFitResult();
}

void XYPiecewiseLinearFitCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_fitCurve = static_cast<XYPiecewiseLinearFitCurve*>(m_curve);
	m_analysisCurve = m_fitCurve;
	setAnalysisCurves(list);
	m_fitData = m_fitCurve->fitData();

	initGeneralTab();
	initTabs();
	setSymbols(list);

	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::fitDataChanged, this, &XYPiecewiseLinearFitCurveDock::curveFitDataChanged);
}

void XYPiecewiseLinearFitCurveDock::showFitResult() {
	const auto& fitResult = m_fitCurve->fitResult();

	if (!fitResult.available) {
		uiGeneralTab.teResults->clear();
		return;
	}

	QString html = QStringLiteral("<h3>Piecewise Linear Fit Results</h3>");

	if (!fitResult.valid) {
		html += QStringLiteral("<p style='color:red;'>") + fitResult.status + QStringLiteral("</p>");
		uiGeneralTab.teResults->setHtml(html);
		return;
	}

	html += QStringLiteral("<table>");
	html += QStringLiteral("<tr><td><b>Status:</b></td><td>") + fitResult.status + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>Segments:</b></td><td>") + QString::number(fitResult.numSegments) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>Changepoints:</b></td><td>") + QString::number(fitResult.changepoints.size()) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>Overall R²:</b></td><td>") + QString::number(fitResult.overallRsquare, 'f', 4) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>SSE:</b></td><td>") + QString::number(fitResult.sse, 'g', 4) + QStringLiteral("</td></tr>");
	html += QStringLiteral("</table>");

	if (!fitResult.changepoints.isEmpty()) {
		html += QStringLiteral("<h4>Changepoints</h4>");
		html += QStringLiteral("<table border='1' cellpadding='3'>");
		html += QStringLiteral("<tr><th>Index</th><th>X-Position</th></tr>");

		const auto* xColumn = m_fitCurve->xDataColumn();
		for (int i = 0; i < fitResult.changepoints.size(); ++i) {
			size_t idx = fitResult.changepoints[i];
			html += QStringLiteral("<tr>");
			html += QStringLiteral("<td>") + QString::number(idx) + QStringLiteral("</td>");
			if (xColumn && idx < static_cast<size_t>(xColumn->rowCount()))
				html += QStringLiteral("<td>") + QString::number(xColumn->valueAt(idx), 'g', 6) + QStringLiteral("</td>");
			else
				html += QStringLiteral("<td>-</td>");
			html += QStringLiteral("</tr>");
		}
		html += QStringLiteral("</table>");
	}

	if (fitResult.numSegments > 0) {
		html += QStringLiteral("<h4>Segments</h4>");
		html += QStringLiteral("<table border='1' cellpadding='3'>");
		html += QStringLiteral("<tr><th>Segment</th><th>Slope</th><th>Intercept</th><th>R²</th></tr>");

		for (size_t i = 0; i < fitResult.numSegments; ++i) {
			if (i >= static_cast<size_t>(fitResult.slopes.size()) || i >= static_cast<size_t>(fitResult.intercepts.size()) 
				|| i >= static_cast<size_t>(fitResult.rsquares.size())) {
				DEBUG(Q_FUNC_INFO << ", invalid segment index: " << i);
				break;
			}
			html += QStringLiteral("<tr>");
			html += QStringLiteral("<td>") + QString::number(i + 1) + QStringLiteral("</td>");
			html += QStringLiteral("<td>") + QString::number(fitResult.slopes[i], 'g', 4) + QStringLiteral("</td>");
			html += QStringLiteral("<td>") + QString::number(fitResult.intercepts[i], 'g', 4) + QStringLiteral("</td>");
			html += QStringLiteral("<td>") + QString::number(fitResult.rsquares[i], 'f', 4) + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		}
		html += QStringLiteral("</table>");
	}

	uiGeneralTab.teResults->setHtml(html);
}
/*
bool XYPiecewiseLinearFitCurveDock::eventFilter(QObject* watched, QEvent* event) {
	return XYCurveDock::eventFilter(watched, event);
}
*/
// slots for changes triggered in XYPiecewiseLinearFitCurveDock
void XYPiecewiseLinearFitCurveDock::dataSourceTypeChanged(int index) {
	const auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYPiecewiseLinearFitCurve*>(curve)->setDataSourceType(type);

	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::methodChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.changepointMethod = static_cast<nsl_changepoint_method>(index);
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::penaltyChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.penalty = uiGeneralTab.sbPenalty->value();
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::minSegmentSizeChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.minSegmentSize = uiGeneralTab.sbMinSegmentSize->value();
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::maxChangepointsChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.maxChangepoints = uiGeneralTab.sbMaxChangepoints->value();
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::evaluatedPointsChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.evaluatedPoints = uiGeneralTab.sbEvaluatedPoints->value();
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_fitData.autoRange = autoRange;

	uiGeneralTab.leFitRangeMin->setEnabled(!autoRange);
	uiGeneralTab.leFitRangeMax->setEnabled(!autoRange);

	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::fitRangeMinChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.fitRange.setStart(uiGeneralTab.leFitRangeMin->text().toDouble());
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::fitRangeMaxChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.fitRange.setEnd(uiGeneralTab.leFitRangeMax->text().toDouble());
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::autoEvalRangeChanged() {
	bool autoEvalRange = uiGeneralTab.cbAutoEvalRange->isChecked();
	m_fitData.autoEvalRange = autoEvalRange;

	uiGeneralTab.leEvalRangeMin->setEnabled(!autoEvalRange);
	uiGeneralTab.leEvalRangeMax->setEnabled(!autoEvalRange);

	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::evalRangeMinChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.evalRange.setStart(uiGeneralTab.leEvalRangeMin->text().toDouble());
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::evalRangeMaxChanged() {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.evalRange.setEnd(uiGeneralTab.leEvalRangeMax->text().toDouble());
	enableRecalculate();
}

void XYPiecewiseLinearFitCurveDock::recalculateClicked() {
	m_fitCurve->setFitData(m_fitData);
	showFitResult();
}

void XYPiecewiseLinearFitCurveDock::retranslateUi() {
	XYAnalysisCurveDock::retranslateUi();

	// tooltips
	QString info = i18n("Method for detecting changepoints.");
	uiGeneralTab.cbMethod->setToolTip(info);

	info = i18n("Minimum number of data points in a segment. Segments with fewer data points are merged with neighboring segments.");
	uiGeneralTab.sbMinSegmentSize->setToolTip(info);

	info = i18n("Cost for adding a changepoint. Must be exceeded by the fit improvement (SSE reduction) to justify splitting.\nLower values detect more changepoints, higher values detect fewer.\nScale with your data: ~1 for Y-values near 1, ~10-100 for large values, ~0.01-0.1 for small values.");
	uiGeneralTab.sbPenalty->setToolTip(info);
}

void XYPiecewiseLinearFitCurveDock::showOptions(bool) {
	// All options always visible in this simple implementation
}

void XYPiecewiseLinearFitCurveDock::showResults(bool) {
	// Results always visible in this simple implementation
}

void XYPiecewiseLinearFitCurveDock::curveFitDataChanged(const XYPiecewiseLinearFitCurve::FitData& fitData) {
	CONDITIONAL_LOCK_RETURN;
	m_fitData = fitData;
}
