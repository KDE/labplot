/*
	File             : XYEquationCurve2Dock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of equation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYEquationCurve2Dock.h"
#include "backend/core/Project.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve2.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>

#include <KLocalizedString>

/*!
  \class XYEquationCurve2Dock
  \brief  Provides a widget for editing the properties of the XYEquationCurves
		(2D-curves defined by a mathematical equation) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYEquationCurve2Dock::XYEquationCurve2Dock(QWidget* parent)
	: XYAnalysisCurveDock(parent) {
	// remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYEquationCurve2Dock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment, uiGeneralTab.pbRecalculate);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2, 2, 2, 2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));
	uiGeneralTab.bAddVariable->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	// Slots
	connect(uiGeneralTab.teEquation, &ExpressionTextEdit::expressionChanged, this, &XYEquationCurve2Dock::enableRecalculate);
	connect(uiGeneralTab.tbConstants, &QToolButton::clicked, this, &XYEquationCurve2Dock::showConstants);
	connect(uiGeneralTab.tbFunctions, &QToolButton::clicked, this, &XYEquationCurve2Dock::showFunctions);
	connect(uiGeneralTab.pbRecalculate, &QToolButton::clicked, this, &XYEquationCurve2Dock::recalculateClicked);

	connect(uiGeneralTab.bAddVariable, &QPushButton::pressed, this, &XYEquationCurve2Dock::addVariable);
}

void XYEquationCurve2Dock::initGeneralTab() {
	// show the properties of the first curve
	const auto* equationCurve = static_cast<const XYEquationCurve2*>(m_curve);
	const auto* plot = m_curve->plot();
	Q_ASSERT(equationCurve);
	uiGeneralTab.teEquation->setText(equationCurve->equation());
	const auto& formulaData = equationCurve->equationData();
	removeAllVariableWidgets();
	if (formulaData.isEmpty()) { // no formula was used for this column -> add the first variable "x"
		addVariable();
		m_variableLineEdits[0]->setText(QStringLiteral("x"));
	} else { // formula and variables are available
		// add all available variables and select the corresponding columns
		const auto& curves = plot->children<XYCurve>(AbstractAspect::ChildIndexFlag::Recursive);
		for (int i = 0; i < formulaData.size(); ++i) {
			addVariable();
			m_variableLineEdits[i]->setText(formulaData.at(i).variableName());

			bool found = false;
			for (const auto* c : curves) {
				if (c != formulaData.at(i).curve())
					continue;

				const auto* curve = dynamic_cast<const XYCurve*>(c);
				if (curve)
					m_variableDataCurves[i]->setCurrentModelIndex(aspectModel()->modelIndexOfAspect(curve));
				else
					m_variableDataCurves[i]->setCurrentModelIndex(QModelIndex());

				m_variableDataCurves[i]->useCurrentIndexText(true);
				m_variableDataCurves[i]->setInvalid(false);

				found = true;
				break;
			}

			// for the current variable name no curve exists anymore (was deleted)
			//->highlight the combobox red
			if (!found) {
				m_variableDataCurves[i]->setCurrentModelIndex(QModelIndex());
				m_variableDataCurves[i]->useCurrentIndexText(false);
				m_variableDataCurves[i]->setInvalid(
					true,
					i18n("The curve \"%1\"\nis not available anymore. It will be automatically used once it is created again.", formulaData.at(i).curvePath()));
				m_variableDataCurves[i]->setText(formulaData.at(i).curvePath().split(QLatin1Char('/')).last());
			}
		}
	}

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_equationCurve, &XYEquationCurve2::equationDataChanged, this, &XYEquationCurve2Dock::curveEquationDataChanged);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYEquationCurve2Dock::setCurves(QList<XYCurve*> list) {
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
	m_equationCurve = dynamic_cast<XYEquationCurve2*>(m_curve);
	Q_ASSERT(m_equationCurve);
	XYCurveDock::setModel();
	initGeneralTab();
	initTabs();
	setSymbols(list);
	enableRecalculate();
	updatePlotRangeList();
}

//*************************************************************
//**** SLOTs for changes triggered in XYEquationCurve2Dock *****
//*************************************************************

// TODO Copied from FunctionValuesDialog. Try to reduce copy paste!!!!
void XYEquationCurve2Dock::addVariable() {
	auto* layout{uiGeneralTab.gridLayoutVariables};
	auto row{m_variableLineEdits.size()};
	// text field for the variable name
	auto* le{new QLineEdit};
	le->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
	auto* validator = new QRegularExpressionValidator(QRegularExpression(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*")), le);
	le->setValidator(validator);
	// hardcoding size is bad. 40 is enough for three letters
	le->setMaximumWidth(40);
	connect(le, &QLineEdit::textChanged, this, &XYEquationCurve2Dock::variableNameChanged);
	layout->addWidget(le, row, 0, 1, 1);
	m_variableLineEdits << le;
	auto* l{new QLabel(QStringLiteral("="))};
	layout->addWidget(l, row, 1, 1, 1);
	m_variableLabels << l;

	// combo box for the data column
	auto* cb{new TreeViewComboBox()};
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &XYEquationCurve2Dock::variableColumnChanged);
	layout->addWidget(cb, row, 2, 1, 1);
	m_variableDataCurves << cb;

	setModelCurve(cb);

	// don't allow to select columns to be calculated as variable columns (avoid circular dependencies)
	QList<const AbstractAspect*> aspects;
	for (auto* c : m_curvesList)
		aspects << c;
	cb->setHiddenAspects(aspects);

	const auto& plot = m_curve->plot();
	// for the variable curve select the first non-selected curve
	for (auto* c : plot->children<XYCurve>()) {
		if (m_curvesList.indexOf(c) == -1) {
			cb->setCurrentModelIndex(aspectModel()->modelIndexOfAspect(c));
			break;
		}
	}

	// move the add-button to the next row
	layout->removeWidget(uiGeneralTab.bAddVariable);
	layout->addWidget(uiGeneralTab.bAddVariable, row + 1, 3, 1, 1);

	// add delete-button for the just added variable
	if (row != 0) {
		auto* b{new QToolButton()};
		b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		b->setToolTip(i18n("Delete variable"));
		layout->addWidget(b, row, 3, 1, 1);
		m_variableDeleteButtons << b;
		connect(b, &QToolButton::pressed, this, &XYEquationCurve2Dock::deleteVariable);
	}

	uiGeneralTab.lVariable->setText(i18n("Variables:"));
}

void XYEquationCurve2Dock::removeAllVariableWidgets() {
	while (!m_variableLineEdits.isEmpty())
		delete m_variableLineEdits.takeFirst();

	while (!m_variableLabels.isEmpty())
		delete m_variableLabels.takeFirst();

	while (!m_variableDataCurves.isEmpty())
		delete m_variableDataCurves.takeFirst();

	while (!m_variableDeleteButtons.isEmpty())
		delete m_variableDeleteButtons.takeFirst();
}

void XYEquationCurve2Dock::deleteVariable() {
	QObject* ob{QObject::sender()};
	const auto index{m_variableDeleteButtons.indexOf(qobject_cast<QToolButton*>(ob))};

	delete m_variableLineEdits.takeAt(index + 1);
	delete m_variableLabels.takeAt(index + 1);
	delete m_variableDataCurves.takeAt(index + 1);
	delete m_variableDeleteButtons.takeAt(index);

	variableNameChanged();
	enableRecalculate();

	// adjust the layout
	resize(QSize(width(), 0).expandedTo(minimumSize()));

	m_variableLineEdits.size() > 1 ? uiGeneralTab.lVariable->setText(i18n("Variables:")) : uiGeneralTab.lVariable->setText(i18n("Variable:"));
}

// TODO Copied from FunctionValuesDialog. Try to reduce copy paste!!!!
void XYEquationCurve2Dock::variableNameChanged() {
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
	uiGeneralTab.teEquation->setVariables(vars);
	enableRecalculate();
}
// TODO Copied from FunctionValuesDialog. Try to reduce copy paste!!!!
void XYEquationCurve2Dock::variableColumnChanged(const QModelIndex& index) {
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

void XYEquationCurve2Dock::recalculateClicked() {
	const auto& equation = uiGeneralTab.teEquation->document()->toPlainText();

	// determine variable names and data vectors of the specified curves
	QStringList variableNames;
	QVector<XYCurve*> variableCurves;
	for (int i = 0; i < m_variableLineEdits.size(); ++i) {
		variableNames << m_variableLineEdits.at(i)->text().simplified();

		auto* aspect{static_cast<AbstractAspect*>(m_variableDataCurves.at(i)->currentModelIndex().internalPointer())};
		if (aspect) {
			auto* curve{dynamic_cast<XYCurve*>(aspect)};
			if (curve)
				variableCurves << curve;
		}
	}

	for (auto* curve : m_curvesList)
		static_cast<XYEquationCurve2*>(curve)->setEquation(equation, variableNames, variableCurves);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	updatePlotRangeList(); // axes range may change when range on auto scale
}

void XYEquationCurve2Dock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, &ConstantsWidget::constantSelected, this, &XYEquationCurve2Dock::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYEquationCurve2Dock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &XYEquationCurve2Dock::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbFunctions->mapToGlobal(pos));
}

void XYEquationCurve2Dock::insertFunction(const QString& functionName) {
	uiGeneralTab.teEquation->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Neutral));
}

void XYEquationCurve2Dock::insertConstant(const QString& constantsName) {
	uiGeneralTab.teEquation->insertPlainText(constantsName);
}

void XYEquationCurve2Dock::enableRecalculate() const {
	// check whether the formula expressions are correct
	bool valid = false;
	valid = uiGeneralTab.teEquation->isValid();

	uiGeneralTab.pbRecalculate->setEnabled(valid);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYEquationCurve2Dock::curveEquationDataChanged(const XYEquationCurve2::EquationData& data) {
	CONDITIONAL_LOCK_RETURN;
	// TODOOOOOOO!!!!
	// uiGeneralTab.teEquation->setText(data.expression);
}
