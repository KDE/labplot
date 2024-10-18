/*
	File             : XYFunctionCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of function curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFunctionCurveDock.h"
#include "backend/core/Project.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/worksheet/plots/cartesian/XYFunctionCurve.h"
#include "frontend/widgets/ConstantsWidget.h"
#include "frontend/widgets/FunctionsWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>

#include <KLocalizedString>

/*!
  \class XYFunctionCurveDock
  \brief Provides a widget for editing the properties of the XYFunctionCurves
		(2D-curves defined by a mathematical function) currently selected in
		the project explorer.

  \ingroup frontend
*/

XYFunctionCurveDock::XYFunctionCurveDock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
	// remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYFunctionCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, uiGeneralTab.pbRecalculate);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	m_gridLayoutVariables = new QGridLayout(uiGeneralTab.frameVariables);
	m_gridLayoutVariables->setContentsMargins(0, 0, 0, 0);
	m_gridLayoutVariables->setHorizontalSpacing(2);
	m_gridLayoutVariables->setVerticalSpacing(2);
	uiGeneralTab.frameVariables->setLayout(m_gridLayoutVariables);

	m_gridLayoutCurves = new QGridLayout(uiGeneralTab.frameCurves);
	m_gridLayoutCurves->setContentsMargins(0, 0, 0, 0);
	m_gridLayoutCurves->setHorizontalSpacing(2);
	m_gridLayoutCurves->setVerticalSpacing(2);
	uiGeneralTab.frameCurves->setLayout(m_gridLayoutCurves);

	auto* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2, 2, 2, 2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	uiGeneralTab.bAddVariable->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));

	// Slots
	connect(uiGeneralTab.bAddVariable, &QPushButton::clicked, this, &XYFunctionCurveDock::addVariable);
	connect(uiGeneralTab.teFunction, &ExpressionTextEdit::expressionChanged, this, &XYFunctionCurveDock::enableRecalculate);
	connect(uiGeneralTab.tbConstants, &QToolButton::clicked, this, &XYFunctionCurveDock::showConstants);
	connect(uiGeneralTab.tbFunctions, &QToolButton::clicked, this, &XYFunctionCurveDock::showFunctions);
	connect(uiGeneralTab.pbRecalculate, &QToolButton::clicked, this, &XYFunctionCurveDock::recalculateClicked);
}

void XYFunctionCurveDock::initGeneralTab() {
	// show the properties of the first curve
	const auto* functionCurve = static_cast<const XYFunctionCurve*>(m_curve);
	const auto* plot = m_curve->plot();
	Q_ASSERT(functionCurve);
	uiGeneralTab.teFunction->setText(functionCurve->function());

	removeAllVariableWidgets();
	const auto& formulaData = functionCurve->functionData();

	if (formulaData.isEmpty()) { // no formula was used for this column -> add the first variable "x"
		addVariable();
		m_variableLineEdits.constFirst()->setText(QStringLiteral("x"));
	} else { // formula and variables are available
		// add all available variables and select the corresponding columns
		const auto& curves = plot->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive);
		for (int i = 0; i < formulaData.size(); ++i) {
			addVariable();
			m_variableLineEdits.at(i)->setText(formulaData.at(i).variableName());
			auto* cb = m_variableComboBoxes.at(i);

			bool found = false;
			for (const auto* curve : curves) {
				if (curve != formulaData.at(i).curve())
					continue;

				if (curve)
					cb->setCurrentModelIndex(aspectModel()->modelIndexOfAspect(curve));
				else
					cb->setCurrentModelIndex(QModelIndex());

				cb->useCurrentIndexText(true);
				cb->setInvalid(false);

				found = true;
				break;
			}

			// for the current variable name no curve exists anymore (was deleted)
			//->highlight the combobox red
			if (!found) {
				cb->setCurrentModelIndex(QModelIndex());
				cb->useCurrentIndexText(false);
				cb->setInvalid(
					true,
					i18n("The curve \"%1\"\nis not available anymore. It will be automatically used once it is created again.", formulaData.at(i).curvePath()));
				cb->setText(formulaData.at(i).curvePath().split(QLatin1Char('/')).last());
			}
		}
	}

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_functionCurve, &XYFunctionCurve::functionDataChanged, this, &XYFunctionCurveDock::curveFunctionDataChanged);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFunctionCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curve = list.first();
	const auto* plot = m_curve->plot();
	for (const auto* c : list) {
		if (c->plot() != plot) {
			// It is not allowed to select curves from different plots
			setEnabled(false);
			setToolTip(QLatin1String("Curves from different Plots are not allowed"));
			return;
		}
	}
	setToolTip(QLatin1String(""));

	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);

	aspectModel()->setRoot(m_curve->project());
	m_functionCurve = dynamic_cast<XYFunctionCurve*>(m_curve);
	Q_ASSERT(m_functionCurve);
	XYCurveDock::setModel();

	initGeneralTab();
	initTabs();

	setSymbols(list);
	enableRecalculate();
	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFunctionCurveDock ****
//*************************************************************
void XYFunctionCurveDock::addVariable() {
	const auto row{m_variableLineEdits.size()};

	// text field for the variable name
	auto* le{new QLineEdit};
	le->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
	auto* validator = new QRegularExpressionValidator(QRegularExpression(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*")), le);
	le->setValidator(validator);
	// le->setMaximumWidth(40); // hardcoding size is bad. 40 is enough for three letters
	connect(le, &QLineEdit::textChanged, this, &XYFunctionCurveDock::variableNameChanged);
	m_gridLayoutVariables->addWidget(le, row, 0, 1, 1);
	m_variableLineEdits << le;

	// combo box for the data column
	auto* cb{new TreeViewComboBox()};
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &XYFunctionCurveDock::variableColumnChanged);
	m_gridLayoutCurves->addWidget(cb, row, 2, 1, 1);
	m_variableComboBoxes << cb;

	setModelCurve(cb);

	// don't allow to select columns to be calculated as variable columns (avoid circular dependencies)
	QList<const AbstractAspect*> aspects;
	for (auto* c : m_curvesList)
		aspects << c;
	cb->setHiddenAspects(aspects);

	// for the variable curve select the first non-selected curve
	const auto& plot = m_curve->plot();
	for (auto* c : plot->children<XYCurve>()) {
		if (m_curvesList.indexOf(c) == -1) {
			cb->setCurrentModelIndex(aspectModel()->modelIndexOfAspect(c));
			break;
		}
	}

	// add delete-button for the just added variable
	if (row != 0) {
		auto* b{new QToolButton()};
		b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		b->setToolTip(i18n("Delete variable"));
		m_gridLayoutCurves->addWidget(b, row, 3, 1, 1);
		m_variableDeleteButtons << b;
		connect(b, &QToolButton::pressed, this, &XYFunctionCurveDock::deleteVariable);
	}
}

void XYFunctionCurveDock::removeAllVariableWidgets() {
	while (!m_variableLineEdits.isEmpty())
		delete m_variableLineEdits.takeFirst();

	while (!m_variableComboBoxes.isEmpty())
		delete m_variableComboBoxes.takeFirst();

	while (!m_variableDeleteButtons.isEmpty())
		delete m_variableDeleteButtons.takeFirst();
}

void XYFunctionCurveDock::deleteVariable() {
	QObject* ob{QObject::sender()};
	const auto index{m_variableDeleteButtons.indexOf(qobject_cast<QToolButton*>(ob))};

	delete m_variableLineEdits.takeAt(index + 1);
	delete m_variableComboBoxes.takeAt(index + 1);
	delete m_variableDeleteButtons.takeAt(index);

	variableNameChanged();
	enableRecalculate();

	// adjust the layout
	resize(QSize(width(), 0).expandedTo(minimumSize()));

	m_variableLineEdits.size() > 1 ? uiGeneralTab.lVariable->setText(i18n("Variables:")) : uiGeneralTab.lVariable->setText(i18n("Variable:"));
}

// TODO Copied from FunctionValuesDialog. Try to reduce copy paste!!!!
void XYFunctionCurveDock::variableNameChanged() {
	QStringList vars;
	QString argText;
	for (auto* varName : m_variableLineEdits) {
		QString name = varName->text().simplified();
		if (!name.isEmpty()) {
			vars << name;

			if (argText.isEmpty())
				argText += name;
			else
				argText += QStringLiteral(", ") + name;
		}
	}

	QString funText = QStringLiteral("f = ");
	if (!argText.isEmpty())
		funText = QStringLiteral("f(") + argText + QStringLiteral(") = ");

	uiGeneralTab.lFunction->setText(funText);
	uiGeneralTab.teFunction->setVariables(vars);
	enableRecalculate();
}

// TODO Copied from FunctionValuesDialog. Try to reduce copy paste!!!!
void XYFunctionCurveDock::variableColumnChanged(const QModelIndex& index) {
	// combobox was potentially red-highlighted because of a missing column
	// remove the highlighting when we have a valid selection now
	auto* aspect{static_cast<AbstractAspect*>(index.internalPointer())};
	if (aspect) {
		auto* cb{dynamic_cast<TreeViewComboBox*>(QObject::sender())};
		if (cb)
			cb->setStyleSheet(QString());
	}

	enableRecalculate();
}

void XYFunctionCurveDock::recalculateClicked() {
	const auto& function = uiGeneralTab.teFunction->document()->toPlainText();

	// determine variable names and data vectors of the specified curves
	QStringList variableNames;
	QVector<const XYCurve*> variableCurves;
	for (int i = 0; i < m_variableLineEdits.size(); ++i) {
		variableNames << m_variableLineEdits.at(i)->text().simplified();

		auto* aspect{static_cast<AbstractAspect*>(m_variableComboBoxes.at(i)->currentModelIndex().internalPointer())};
		if (aspect) {
			auto* curve{dynamic_cast<const XYCurve*>(aspect)};
			if (curve)
				variableCurves << curve;
		}
	}

	for (auto* curve : m_curvesList)
		static_cast<XYFunctionCurve*>(curve)->setFunction(function, variableNames, variableCurves);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	updatePlotRangeList(); // axes range may change when range on auto scale
}

void XYFunctionCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, &ConstantsWidget::constantSelected, this, &XYFunctionCurveDock::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYFunctionCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &XYFunctionCurveDock::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbFunctions->mapToGlobal(pos));
}

void XYFunctionCurveDock::insertFunction(const QString& functionName) {
	uiGeneralTab.teFunction->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Neutral));
}

void XYFunctionCurveDock::insertConstant(const QString& constantsName) {
	uiGeneralTab.teFunction->insertPlainText(constantsName);
}

void XYFunctionCurveDock::enableRecalculate() const {
	// check whether the formula expressions are correct
	const bool valid = uiGeneralTab.teFunction->isValid();
	uiGeneralTab.pbRecalculate->setEnabled(valid);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYFunctionCurveDock::curveFunctionDataChanged(const XYFunctionCurve::FunctionData&) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.teFunction->setText(m_functionCurve->function());
}
