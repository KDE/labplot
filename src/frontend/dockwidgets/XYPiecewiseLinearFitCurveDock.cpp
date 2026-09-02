/*
	File                 : XYPiecewiseLinearFitCurveDock.cpp
	Project              : LabPlot
	Description          : widget for editing properties of piecewise linear fit curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYPiecewiseLinearFitCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYPiecewiseLinearFitCurve.h"
#include "frontend/widgets/TreeViewComboBox.h"

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
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 2);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	retranslateUi();

	// TODO: activate later, when the feature is implemented
	uiGeneralTab.chkChangepointLines->hide();

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYPiecewiseLinearFitCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYPiecewiseLinearFitCurveDock::methodChanged);
	connect(uiGeneralTab.cbConnection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYPiecewiseLinearFitCurveDock::connectionTypeChanged);
	connect(uiGeneralTab.sbPenalty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::penaltyChanged);
	connect(uiGeneralTab.sbMinSegmentSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::minSegmentSizeChanged);
	connect(uiGeneralTab.sbMaxChangepoints, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYPiecewiseLinearFitCurveDock::maxChangepointsChanged);
	connect(uiGeneralTab.chkChangepointLines, &QCheckBox::toggled, this, &XYPiecewiseLinearFitCurveDock::changepointLinesEnabledChanged);
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
	uiGeneralTab.cbConnection->setCurrentIndex(static_cast<int>(m_fitData.connectionType));
	uiGeneralTab.sbPenalty->setValue(m_fitData.penalty);
	uiGeneralTab.sbMinSegmentSize->setValue(m_fitData.minSegmentSize);
	uiGeneralTab.sbMaxChangepoints->setValue(m_fitData.maxChangepoints);
	uiGeneralTab.chkChangepointLines->setChecked(m_fitCurve->changepointLinesEnabled());

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	showFitResult();

	// Slots
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::fitDataChanged, this, &XYPiecewiseLinearFitCurveDock::curveFitDataChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::dataSourceTypeChanged, this, &XYPiecewiseLinearFitCurveDock::curveDataSourceTypeChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::dataSourceCurveChanged, this, &XYPiecewiseLinearFitCurveDock::curveDataSourceCurveChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::xDataColumnChanged, this, &XYPiecewiseLinearFitCurveDock::curveXDataColumnChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::yDataColumnChanged, this, &XYPiecewiseLinearFitCurveDock::curveYDataColumnChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::fitDataChanged, this, &XYPiecewiseLinearFitCurveDock::curveFitDataChanged);
	connect(m_fitCurve, &XYPiecewiseLinearFitCurve::sourceDataChanged, this, &XYPiecewiseLinearFitCurveDock::enableRecalculate);
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

	updatePlotRangeList();
}

void XYPiecewiseLinearFitCurveDock::showFitResult() {
	const auto& fitResult = m_fitCurve->fitResult();

	if (!fitResult.available) {
		uiGeneralTab.teResults->clear();
		return;
	}

	QString html;

	if (!fitResult.valid) {
		html = QStringLiteral("<p style='color:red;'>") + fitResult.status + QStringLiteral("</p>");
		uiGeneralTab.teResults->setHtml(html);
		return;
	}

	html += QStringLiteral("<table>");
	html += QStringLiteral("<tr><td><b>Status:</b></td><td>") + fitResult.status + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>Segments:</b></td><td>") + QString::number(fitResult.numSegments) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>Changepoints:</b></td><td>") + QString::number(fitResult.changepoints.size()) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>R²:</b></td><td>") + QString::number(fitResult.rsquare, 'f', 4) + QStringLiteral("</td></tr>");
	html += QStringLiteral("<tr><td><b>SSE:</b></td><td>") + QString::number(fitResult.sse, 'g', 4) + QStringLiteral("</td></tr>");
	html += QStringLiteral("</table>");

	if (!fitResult.changepoints.isEmpty()) {
		html += QStringLiteral("<h4>Changepoints</h4>");
		html += QStringLiteral("<table border='1' cellpadding='3'>");
		html += QStringLiteral("<tr><th>#</th><th>X-Position</th></tr>");

		for (int i = 0; i < fitResult.changepoints.size(); ++i) {
			double xPos = fitResult.changepoints[i];
			html += QStringLiteral("<tr>");
			html += QStringLiteral("<td>") + QString::number(i + 1) + QStringLiteral("</td>");
			html += QStringLiteral("<td>") + QString::number(xPos, 'g', 6) + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		}
		html += QStringLiteral("</table>");
	}

	if (fitResult.numSegments > 0) {
		html += QStringLiteral("<h4>Segment Parameters</h4>");
		html += QStringLiteral("<table border='1' cellpadding='3'>");
		html += QStringLiteral("<tr><th></th>");
		for (size_t i = 0; i < fitResult.numSegments; ++i) {
			html += QStringLiteral("<th>Segment %1</th>").arg(i + 1);
		}
		html += QStringLiteral("</tr>");

		auto addParameterRow = [&html, &fitResult](const QString& label, const auto& value, char format, int precision) {
			html += QStringLiteral("<tr><th>") + label + QStringLiteral("</th>");
			for (const auto& segResult : fitResult.segmentResults)
				html += QStringLiteral("<td>") + QString::number(static_cast<double>(value(segResult)), format, precision) + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		};
		addParameterRow(QStringLiteral("Slope"), [](const auto& result) { return result.paramValues.value(1); }, 'g', 4);
		addParameterRow(QStringLiteral("SE(Slope)"), [](const auto& result) { return result.errorValues.value(1); }, 'g', 3);
		addParameterRow(QStringLiteral("Intercept"), [](const auto& result) { return result.paramValues.value(0); }, 'g', 4);
		addParameterRow(QStringLiteral("SE(Intercept)"), [](const auto& result) { return result.errorValues.value(0); }, 'g', 3);
		addParameterRow(QStringLiteral("t (Slope)"), [](const auto& result) { return result.tdist_tValues.value(1); }, 'g', 3);

		html += QStringLiteral("<tr><th>p (Slope)</th>");
		for (const auto& segResult : fitResult.segmentResults) {
			const double p = segResult.tdist_pValues.value(1);
			const auto pColor = p > 0.05 ? QStringLiteral("color:gray;") : p > 0.01 ? QStringLiteral("color:darkgreen;")
				: p > 0.001 ? QStringLiteral("color:darkcyan;") : p > 0.0001 ? QStringLiteral("color:blue;") : QStringLiteral("color:red;");
			html += QStringLiteral("<td style='") + pColor + QStringLiteral("'>") + QString::number(p, 'g', 3) + QStringLiteral("</td>");
		}
		html += QStringLiteral("</tr>");
		html += QStringLiteral("</table>");

		html += QStringLiteral("<h4>Goodness of Fit</h4>");
		html += QStringLiteral("<table border='1' cellpadding='3'>");
		html += QStringLiteral("<tr><th></th>");
		for (size_t i = 0; i < fitResult.numSegments; ++i) {
			html += QStringLiteral("<th>Segment %1</th>").arg(i + 1);
		}
		html += QStringLiteral("</tr>");

		auto addGoodnessRow = [&html, &fitResult](const QString& label, const auto& value, char format, int precision) {
			html += QStringLiteral("<tr><th>") + label + QStringLiteral("</th>");
			for (const auto& segResult : fitResult.segmentResults)
				html += QStringLiteral("<td>") + QString::number(static_cast<double>(value(segResult)), format, precision) + QStringLiteral("</td>");
			html += QStringLiteral("</tr>");
		};
		addGoodnessRow(QStringLiteral("R²"), [](const auto& result) { return result.rsquare; }, 'f', 4);
		addGoodnessRow(QStringLiteral("Adj. R²"), [](const auto& result) { return result.rsquareAdj; }, 'f', 4);
		addGoodnessRow(QStringLiteral("SSE"), [](const auto& result) { return result.sse; }, 'g', 4);
		addGoodnessRow(QStringLiteral("RMS"), [](const auto& result) { return result.rms; }, 'g', 4);
		addGoodnessRow(QStringLiteral("MAE"), [](const auto& result) { return result.mae; }, 'g', 4);
		addGoodnessRow(QStringLiteral("DoF"), [](const auto& result) { return result.dof; }, 'g', 0);
		addGoodnessRow(QStringLiteral("χ² p"), [](const auto& result) { return result.chisq_p; }, 'g', 3);
		addGoodnessRow(QStringLiteral("F"), [](const auto& result) { return result.fdist_F; }, 'g', 4);
		addGoodnessRow(QStringLiteral("F p"), [](const auto& result) { return result.fdist_p; }, 'g', 3);
		addGoodnessRow(QStringLiteral("AIC"), [](const auto& result) { return result.aic; }, 'g', 4);
		addGoodnessRow(QStringLiteral("BIC"), [](const auto& result) { return result.bic; }, 'g', 4);
		html += QStringLiteral("</table>");
	}

	uiGeneralTab.teResults->setHtml(html);
}

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

void XYPiecewiseLinearFitCurveDock::connectionTypeChanged(int) {
	CONDITIONAL_LOCK_RETURN;
	m_fitData.connectionType = static_cast<XYPiecewiseLinearFitCurve::ConnectionType>(uiGeneralTab.cbConnection->currentData().toInt());
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

void XYPiecewiseLinearFitCurveDock::changepointLinesEnabledChanged() {
	CONDITIONAL_LOCK_RETURN;
	bool enabled = uiGeneralTab.chkChangepointLines->isChecked();
	for (auto* curve : m_curvesList)
		static_cast<XYPiecewiseLinearFitCurve*>(curve)->setChangepointLinesEnabled(enabled);
}

void XYPiecewiseLinearFitCurveDock::recalculateClicked() {
	m_fitCurve->setFitData(m_fitData);
	showFitResult();
}

void XYPiecewiseLinearFitCurveDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	XYAnalysisCurveDock::retranslateUi();

	uiGeneralTab.cbConnection->clear();
	uiGeneralTab.cbConnection->addItem(i18n("Continuous"), static_cast<int>(XYPiecewiseLinearFitCurve::ConnectionType::Continuous));
	uiGeneralTab.cbConnection->addItem(i18n("Discontinuous"), static_cast<int>(XYPiecewiseLinearFitCurve::ConnectionType::Discontinuous));

	// tooltips
	QString info = i18n("Method for detecting changepoints.");
	uiGeneralTab.cbMethod->setToolTip(info);

	info = i18n("Minimum number of data points in a segment. Segments with fewer data points are merged with neighboring segments.");
	uiGeneralTab.lMinSegmentSize->setToolTip(info);
	uiGeneralTab.sbMinSegmentSize->setToolTip(info);

	info = i18n("Cost for adding a changepoint. Must be exceeded by the fit improvement (SSE reduction) to justify splitting.\nLower values detect more changepoints, higher values detect fewer.\nScale with your data: ~1 for Y-values near 1, ~10-100 for large values, ~0.01-0.1 for small values.");
	uiGeneralTab.lPenalty->setToolTip(info);
	uiGeneralTab.sbPenalty->setToolTip(info);

	info = i18n("Determine if the fit should be continuous or discontinuous at the changepoints.");
	uiGeneralTab.lConnection->setToolTip(info);
	uiGeneralTab.cbConnection->setToolTip(info);
}

void XYPiecewiseLinearFitCurveDock::curveFitDataChanged(const XYPiecewiseLinearFitCurve::FitData& fitData) {
	CONDITIONAL_LOCK_RETURN;
	m_fitData = fitData;
}
