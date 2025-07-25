/*
	File             : XYEquationCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of equation curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYEquationCurveDock.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "frontend/widgets/ConstantsWidget.h"
#include "frontend/widgets/FunctionsWidget.h"
#include "frontend/GuiTools.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QCompleter>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>

/*!
  \class XYEquationCurveDock
  \brief  Provides a widget for editing the properties of the XYEquationCurves
		(2D-curves defined by a mathematical equation) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYEquationCurveDock::XYEquationCurveDock(QWidget* parent)
	: XYCurveDock(parent) {
	// remove the tab "Error bars"
	ui.tabWidget->removeTab(5);

	installEventFilter(this);
}

/*!
 * 	// Tab "General"
 */
void XYEquationCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment);
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

	uiGeneralTab.tbConstants1->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	uiGeneralTab.tbFunctions1->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));

	uiGeneralTab.tbConstants2->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	uiGeneralTab.tbFunctions2->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));

	// TODO: move to retranslateUi()
	uiGeneralTab.cbType->addItem(i18n("Cartesian"));
	uiGeneralTab.cbType->addItem(i18n("Polar"));
	uiGeneralTab.cbType->addItem(i18n("Parametric"));
	// 	uiGeneralTab.cbType->addItem(i18n("Implicit"));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
	uiGeneralTab.pbRecalculate->setToolTip(i18n("Click this button or press Shift+Enter to recalculate the result."));

	uiGeneralTab.teEquation2->setExpressionType(XYEquationCurve::EquationType::Parametric);

	// 	uiGeneralTab.teEquation1->setMaximumHeight(uiGeneralTab.leName->sizeHint().height()*2);
	// 	uiGeneralTab.teEquation2->setMaximumHeight(uiGeneralTab.leName->sizeHint().height()*2);
	uiGeneralTab.teMin->setMaximumHeight(uiGeneralTab.leName->sizeHint().height());
	uiGeneralTab.teMax->setMaximumHeight(uiGeneralTab.leName->sizeHint().height());

	// Slots
	connect(uiGeneralTab.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYEquationCurveDock::typeChanged);
	connect(uiGeneralTab.teEquation1, &ExpressionTextEdit::expressionChanged, this, &XYEquationCurveDock::enableRecalculate);
	connect(uiGeneralTab.teEquation2, &ExpressionTextEdit::expressionChanged, this, &XYEquationCurveDock::enableRecalculate);
	connect(uiGeneralTab.tbConstants1, &QToolButton::clicked, this, &XYEquationCurveDock::showConstants);
	connect(uiGeneralTab.tbFunctions1, &QToolButton::clicked, this, &XYEquationCurveDock::showFunctions);
	connect(uiGeneralTab.tbConstants2, &QToolButton::clicked, this, &XYEquationCurveDock::showConstants);
	connect(uiGeneralTab.tbFunctions2, &QToolButton::clicked, this, &XYEquationCurveDock::showFunctions);
	connect(uiGeneralTab.pbLoadFunction, &QPushButton::clicked, this, &XYEquationCurveDock::loadFunction);
	connect(uiGeneralTab.pbSaveFunction, &QPushButton::clicked, this, &XYEquationCurveDock::saveFunction);
	connect(uiGeneralTab.teMin, &ExpressionTextEdit::expressionChanged, this, &XYEquationCurveDock::enableRecalculate);
	connect(uiGeneralTab.teMax, &ExpressionTextEdit::expressionChanged, this, &XYEquationCurveDock::enableRecalculate);
	connect(uiGeneralTab.sbCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYEquationCurveDock::enableRecalculate);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYEquationCurveDock::recalculateClicked);
}

void XYEquationCurveDock::initGeneralTab() {
	// show the properties of the first curve
	const auto* equationCurve = static_cast<const XYEquationCurve*>(m_curve);
	Q_ASSERT(equationCurve);
	const XYEquationCurve::EquationData& data = equationCurve->equationData();
	uiGeneralTab.cbType->setCurrentIndex(static_cast<int>(data.type));
	this->typeChanged(static_cast<int>(data.type));
	uiGeneralTab.teEquation1->setText(data.expression1);
	uiGeneralTab.teEquation2->setText(data.expression2);
	uiGeneralTab.teMin->setText(data.min);
	uiGeneralTab.teMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_equationCurve, &XYEquationCurve::equationDataChanged, this, &XYEquationCurveDock::curveEquationDataChanged);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYEquationCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_equationCurve = static_cast<XYEquationCurve*>(m_curve);
	Q_ASSERT(m_equationCurve);
	XYCurveDock::setModel();
	initGeneralTab();
	initTabs();
	setSymbols(list);

	updatePlotRangeList();

	uiGeneralTab.pbRecalculate->setEnabled(false);
}

bool XYEquationCurveDock::eventFilter(QObject* /* watched */, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		const auto* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Return && keyEvent->modifiers() == Qt::ShiftModifier && uiGeneralTab.pbRecalculate->isEnabled()) {
			recalculateClicked();
			return true;
		}
	}

	return false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYEquationCurveDock *****
//*************************************************************
void XYEquationCurveDock::typeChanged(int index) {
	const auto type{XYEquationCurve::EquationType(index)};
	if (type == XYEquationCurve::EquationType::Cartesian) {
		uiGeneralTab.lEquation1->setText(QStringLiteral("y=f(x)"));
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("x, min"));
		uiGeneralTab.lMax->setText(i18n("x, max"));
	} else if (type == XYEquationCurve::EquationType::Polar) {
		uiGeneralTab.lEquation1->setText(QString::fromUtf8("r(φ)"));
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("φ, min"));
		uiGeneralTab.lMax->setText(i18n("φ, max"));
	} else if (type == XYEquationCurve::EquationType::Parametric) {
		uiGeneralTab.lEquation1->setText(QStringLiteral("x=f(t)"));
		uiGeneralTab.lEquation2->setText(QStringLiteral("y=f(t)"));
		uiGeneralTab.lEquation2->show();
		uiGeneralTab.teEquation2->show();
		uiGeneralTab.tbFunctions2->show();
		uiGeneralTab.tbConstants2->show();
		uiGeneralTab.lMin->show();
		uiGeneralTab.lMax->show();
		uiGeneralTab.teMin->show();
		uiGeneralTab.teMax->show();
		uiGeneralTab.lMin->setText(i18n("t, min"));
		uiGeneralTab.lMax->setText(i18n("t, max"));
	} else if (type == XYEquationCurve::EquationType::Implicit) {
		uiGeneralTab.lEquation1->setText(QStringLiteral("f(x,y)"));
		uiGeneralTab.lEquation2->hide();
		uiGeneralTab.teEquation2->hide();
		uiGeneralTab.tbFunctions2->hide();
		uiGeneralTab.tbConstants2->hide();
		uiGeneralTab.lMin->hide();
		uiGeneralTab.lMax->hide();
		uiGeneralTab.teMin->hide();
		uiGeneralTab.teMax->hide();
	}

	uiGeneralTab.teEquation1->setExpressionType(type);
	this->enableRecalculate();
}

void XYEquationCurveDock::recalculateClicked() {
	XYEquationCurve::EquationData data;
	data.type = (XYEquationCurve::EquationType)uiGeneralTab.cbType->currentIndex();
	data.expression1 = uiGeneralTab.teEquation1->document()->toPlainText();
	data.expression2 = uiGeneralTab.teEquation2->document()->toPlainText();
	data.min = uiGeneralTab.teMin->document()->toPlainText();
	data.max = uiGeneralTab.teMax->document()->toPlainText();
	data.count = uiGeneralTab.sbCount->value();

	for (auto* curve : m_curvesList)
		static_cast<XYEquationCurve*>(curve)->setEquationData(data);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	updatePlotRangeList(); // axes range may change when range on auto scale
}

void XYEquationCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	if (QObject::sender() == uiGeneralTab.tbConstants1)
		connect(&constants, &ConstantsWidget::constantSelected, this, &XYEquationCurveDock::insertConstant1);
	else
		connect(&constants, &ConstantsWidget::constantSelected, this, &XYEquationCurveDock::insertConstant2);

	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	if (QObject::sender() == uiGeneralTab.tbConstants1) {
		QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants1->width(), -menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbConstants1->mapToGlobal(pos));
	} else {
		QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants2->width(), -menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbConstants2->mapToGlobal(pos));
	}
}

void XYEquationCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	if (QObject::sender() == uiGeneralTab.tbFunctions1)
		connect(&functions, &FunctionsWidget::functionSelected, this, &XYEquationCurveDock::insertFunction1);
	else
		connect(&functions, &FunctionsWidget::functionSelected, this, &XYEquationCurveDock::insertFunction2);

	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	if (QObject::sender() == uiGeneralTab.tbFunctions1) {
		QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions1->width(), -menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions1->mapToGlobal(pos));
	} else {
		QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions2->width(), -menu.sizeHint().height());
		menu.exec(uiGeneralTab.tbFunctions2->mapToGlobal(pos));
	}
}

void XYEquationCurveDock::loadFunction() {
	QString fileName = GuiTools::loadFunction(uiGeneralTab.teEquation1);
	if (fileName.isEmpty())
		return;

	// special options if accepted
	KConfig config(fileName);
	auto group = config.group(QLatin1String("EquationCurve"));
	auto xmin = group.readEntry("XMin", 0);
	auto xmax = group.readEntry("XMax", 1);
	auto npoints = group.readEntry("NPoints", 1000);

	const auto numberLocale = QLocale();
	uiGeneralTab.teMin->setText(numberLocale.toString(xmin));
	uiGeneralTab.teMax->setText(numberLocale.toString(xmax));
	uiGeneralTab.sbCount->setValue(npoints);
}

void XYEquationCurveDock::saveFunction() {
	QString fileName = GuiTools::saveFunction(uiGeneralTab.teEquation1);
	if (fileName.isEmpty())
		return;

	// special option if accepted
	KConfig config(fileName);
	auto group = config.group(QLatin1String("EquationCurve"));
	group.writeEntry("XMin", uiGeneralTab.teMin->document()->toPlainText());
	group.writeEntry("XMax", uiGeneralTab.teMax->document()->toPlainText());
	group.writeEntry("NPoints", uiGeneralTab.sbCount->value());
	config.sync();
	QDEBUG(Q_FUNC_INFO << ", saved function options to" << fileName)
}

void XYEquationCurveDock::insertFunction1(const QString& functionName) {
	const auto type{XYEquationCurve::EquationType(uiGeneralTab.cbType->currentIndex())};

	uiGeneralTab.teEquation1->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, type));
}

void XYEquationCurveDock::insertConstant1(const QString& constantsName) {
	uiGeneralTab.teEquation1->insertPlainText(constantsName);
}

void XYEquationCurveDock::insertFunction2(const QString& functionName) {
	uiGeneralTab.teEquation1->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Parametric));
}

void XYEquationCurveDock::insertConstant2(const QString& constantsName) {
	uiGeneralTab.teEquation2->insertPlainText(constantsName);
}

void XYEquationCurveDock::enableRecalculate() {
	CONDITIONAL_RETURN_NO_LOCK;

	// check whether the formula expressions are correct
	bool valid = false;
	const auto type = XYEquationCurve::EquationType(uiGeneralTab.cbType->currentIndex());
	if (type != XYEquationCurve::EquationType::Parametric)
		valid = uiGeneralTab.teEquation1->isValid();
	else
		valid = (uiGeneralTab.teEquation1->isValid() && uiGeneralTab.teEquation2->isValid());

	valid = (valid && uiGeneralTab.teMin->isValid() && uiGeneralTab.teMax->isValid());
	uiGeneralTab.pbRecalculate->setEnabled(valid);

	updatePlotRangeList();
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYEquationCurveDock::curveEquationDataChanged(const XYEquationCurve::EquationData& data) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.cbType->setCurrentIndex(static_cast<int>(data.type));
	uiGeneralTab.teEquation1->setText(data.expression1);
	uiGeneralTab.teEquation2->setText(data.expression2);
	uiGeneralTab.teMin->setText(data.min);
	uiGeneralTab.teMax->setText(data.max);
	uiGeneralTab.sbCount->setValue(data.count);
}
