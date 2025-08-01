/*
	File                 : HypothesisTestDock.cpp
	Project              : LabPlot
	Description          : Dock for Hypothesis Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal <agarwaldevanshu8@gmail.com>
	SPDX-FileCopyrightText: 2023-205 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <KLocalizedString>
#include <QAbstractButton>
#include <QRadioButton>

#include "HypothesisTestDock.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include "backend/core/column/Column.h"

/*!
  \class HypothesisTestDock
  \brief Provides a dock widget for statistical hypothesis tests.
  \ingroup frontend
*/
HypothesisTestDock::HypothesisTestDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	ui.lTestMean->setText(QStringLiteral("μ₀"));
	ui.lSignificanceLevel->setText(QStringLiteral("α"));

	ui.sbSignificanceLevel->setMinimumNotEqual(0);

	ui.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

	hideControls();

	retranslateUi();

	//**********************************  Slots **********************************************
	connect(ui.cbTest, &QComboBox::currentIndexChanged, this, &HypothesisTestDock::testChanged);
	connect(ui.pbRecalculate, &QPushButton::clicked, this, &HypothesisTestDock::recalculate);

	connect(ui.tbAddVariable, &QToolButton::clicked, this, &HypothesisTestDock::addVariable);
	connect(ui.tbRemoveVariable, &QToolButton::clicked, this, &HypothesisTestDock::removeVariable);

	connect(ui.rbNullTwoTailed, &QRadioButton::toggled, ui.rbAlternateTwoTailed, &QRadioButton::setChecked);
	connect(ui.rbNullOneTailedLeft, &QRadioButton::toggled, ui.rbAlternateOneTailedLeft, &QRadioButton::setChecked);
	connect(ui.rbNullOneTailedRight, &QRadioButton::toggled, ui.rbAlternateOneTailedRight, &QRadioButton::setChecked);
}

void HypothesisTestDock::setTest(HypothesisTest* test) {
	CONDITIONAL_LOCK_RETURN;

	// disconnect all connections to the previous test
	for (auto& conn : m_aspectConnections)
		disconnect(conn);

	m_aspectConnections.clear();

	m_test = test;
	setAspects(QList<AbstractAspect*>{m_test});

	// we want to restore the properties from the aspect to the ui

	// clear all variables
	for (int i = 0, count = ui.variablesVerticalLayout->count(); i < count; i++)
		removeVariable();

	// confirm that all variables are cleared
	Q_ASSERT(ui.variablesVerticalLayout->count() == 0);

	// restore variables from aspect columns
	for (auto* col : m_test->columns()) {
		addVariable();
		int varCount = ui.variablesVerticalLayout->count();
		auto* layout = ui.variablesVerticalLayout->itemAt(varCount - 1)->layout(); // layoutitem for the just added variable above
		auto* treeViewCb = static_cast<TreeViewComboBox*>(layout->itemAt(1)->widget()); // treeviewcombobox for the just added variable above
		treeViewCb->setAspect(col); // set the aspect for the treeviewcombobox to the column
	}

	// restore significance level
	ui.sbSignificanceLevel->setValue(m_test->significanceLevel());

	// restore test mean
	ui.sbTestMean->setValue(m_test->testMean());

	int hypothesisCount = HypothesisTest::hypothesisCount(m_test->test());

	// restore null hypothesis
	if (hypothesisCount == 3) {
		switch (m_test->tail()) {
		case nsl_stats_tail_type_two:
			ui.rbNullTwoTailed->setChecked(true);
			break;
		case nsl_stats_tail_type_positive:
			ui.rbNullOneTailedLeft->setChecked(true);
			break;
		case nsl_stats_tail_type_negative:
			ui.rbNullOneTailedRight->setChecked(true);
			break;
		}
	}

	// trigger testChanged() to update the ui
	int oldIndex = ui.cbTest->currentIndex();
	ui.cbTest->setCurrentIndex(ui.cbTest->findData(static_cast<int>(m_test->test()))); // auto trigger testChanged()
	int newIndex = ui.cbTest->currentIndex();
	if (oldIndex == newIndex)
		testChanged(); // manually trigger testChanged()

	// update the aspect properties when the ui properties change
	m_aspectConnections << connect(ui.sbSignificanceLevel, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setSignificanceLevel);
	m_aspectConnections << connect(ui.sbTestMean, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setTestMean);
	m_aspectConnections << connect(ui.rbNullTwoTailed, &QRadioButton::toggled, [this](bool checked) {
		if (checked)
			m_test->setTail(nsl_stats_tail_type_two);
	});
	m_aspectConnections << connect(ui.rbNullOneTailedLeft, &QRadioButton::toggled, [this](bool checked) {
		if (checked)
			m_test->setTail(nsl_stats_tail_type_positive);
	});
	m_aspectConnections << connect(ui.rbNullOneTailedRight, &QRadioButton::toggled, [this](bool checked) {
		if (checked)
			m_test->setTail(nsl_stats_tail_type_negative);
	});
}

void HypothesisTestDock::retranslateUi() {
	ui.retranslateUi(this);

	ui.cbTest->clear();
	ui.cbTest->addItem(i18n("One-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Independent Two-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_two_sample));
	ui.cbTest->addItem(i18n("Paired Two-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_two_sample_paired));
	// ui.cbTest->addItem(i18n("One-Way ANOVA"), static_cast<int>(HypothesisTest::Test::one_way_anova));
	// ui.cbTest->addItem(i18n("Mann-Whitney U Test"), static_cast<int>(HypothesisTest::Test::mann_whitney_u_test));
	// ui.cbTest->addItem(i18n("Kruskal-Wallis Test"), static_cast<int>(HypothesisTest::Test::kruskal_wallis_test));
	ui.cbTest->addItem(i18n("Logrank Test"), static_cast<int>(HypothesisTest::Test::log_rank_test));

	// tooltip texts
	QString info = i18n(
		"<ul>"
		"<li><b>One-Sample t-Test</b> - tests if a sample mean differs significantly from a known population mean</li>"
		"<li><b>Independent Two-Sample t-Test</b> - tests if two independent samples have the same mean</li>"
		"<li><b>Paired Two-Sample t-Test</b> - tests if the mean difference between two related samples is zero</li>"
		// 	"<li><b>One-Way ANOVA</b> - tests if three or more independent samples have the same mean</li>"
		// 	"<li><b>Mann-Whitney U Test</b> - tests differences in medians between two independent groups</li>"
		// 	"<li><b>Kruskal-Wallis Test</b> - tests differences in medians among three or more independent groups</li>"
		"<li><b>Logrank Test</b> - tests differences in survival distributions between two or more groups</li>"
		"</ul>"
	);

	ui.lTest->setToolTip(info);
	ui.cbTest->setToolTip(info);
}

void HypothesisTestDock::ensureHypothesis() {
	int hypothesisCount = HypothesisTest::hypothesisCount(m_test->test());
	QVector<QPair<QString, QString>> hypothesisTexts = HypothesisTest::hypothesisText(m_test->test());

	if (hypothesisCount == 3) {
		ui.lNullTwoTailed->setText(hypothesisTexts[0].first);
		ui.lAlternateTwoTailed->setText(hypothesisTexts[0].second);
		ui.lNullOneTailedLeft->setText(hypothesisTexts[1].first);
		ui.lAlternateOneTailedLeft->setText(hypothesisTexts[1].second);
		ui.lNullOneTailedRight->setText(hypothesisTexts[2].first);
		ui.lAlternateOneTailedRight->setText(hypothesisTexts[2].second);

		ui.lNullOneTailedLeft->show();
		ui.lAlternateOneTailedLeft->show();
		ui.lNullOneTailedRight->show();
		ui.lAlternateOneTailedRight->show();
		ui.rbNullOneTailedLeft->show();
		ui.rbAlternateOneTailedLeft->show();
		ui.rbNullOneTailedRight->show();
		ui.rbAlternateOneTailedRight->show();
	} else if (hypothesisCount == 1) {
		ui.lNullTwoTailed->setText(hypothesisTexts[0].first);
		ui.lAlternateTwoTailed->setText(hypothesisTexts[0].second);

		ui.lNullOneTailedLeft->hide();
		ui.lAlternateOneTailedLeft->hide();
		ui.lNullOneTailedRight->hide();
		ui.lAlternateOneTailedRight->hide();
		ui.rbNullOneTailedLeft->hide();
		ui.rbAlternateOneTailedLeft->hide();
		ui.rbNullOneTailedRight->hide();
		ui.rbAlternateOneTailedRight->hide();

		ui.rbNullTwoTailed->setChecked(true);
	}
}

void HypothesisTestDock::ensureVariableCount() {
	// get the min and max number of columns needed for the test
	const auto [min, max] = HypothesisTest::variableCount(m_test->test());

	// function pointer to add or remove variables
	void (HypothesisTestDock::*func)() = nullptr;

	// difference between the current number of columns and the min/max number of columns needed for the test
	int diff = 0;

	// get the current number of columns
	int varCount = ui.variablesVerticalLayout->count();

	if (varCount < min) {
		// if the current number of columns is less than the min number of columns needed for the test, add columns to reach min
		func = &HypothesisTestDock::addVariable;
		diff = qAbs(min - varCount);
	} else if (varCount > max) {
		// if the current number of columns is greater than the max number of columns needed for the test, remove columns to reach max
		func = &HypothesisTestDock::removeVariable;
		diff = qAbs(varCount - max);
	}

	// add or remove columns to reach the min/max number of columns needed for the test
	for (int i = diff; i > 0; --i) {
		(this->*func)();
	}

	// confirm that the number of columns is correct for the test
	int newVarCount = ui.variablesVerticalLayout->count();
	Q_ASSERT(newVarCount >= min && newVarCount <= max);

	// enable or disable the add/remove variables buttons
	manageAddRemoveVariable();

	// update the columns in the aspect
	updateColumns();
}

void HypothesisTestDock::manageAddRemoveVariable() {
	const auto [min, max] = HypothesisTest::variableCount(m_test->test());
	const int count = ui.variablesVerticalLayout->count();

	ui.tbRemoveVariable->setEnabled(count > min);
	ui.tbAddVariable->setEnabled(count < max);
}

// called:
// - when the user has selected a column in the treeviewcombobox
// - when the user has added or removed a column
// - in ensureVariableCount()
void HypothesisTestDock::updateColumns() {
	QVector<Column*> columns;
	int varCount = ui.variablesVerticalLayout->count();
	for (int i = 0; i < varCount; i++) {
		auto* layout = ui.variablesVerticalLayout->itemAt(i)->layout();
		auto* treeViewCb = static_cast<TreeViewComboBox*>(layout->itemAt(1)->widget());
		auto* col = dynamic_cast<Column*>(treeViewCb->currentAspect());
		if (col)
			columns << col;
	}

	m_test->setColumns(columns);
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void HypothesisTestDock::recalculate() {
	// m_test->setTest clears the columns, so we need to update them
	updateColumns();

	// every other property should already be set via the connections

	m_test->recalculate();
}

void HypothesisTestDock::manageRecalculate() {
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());

	// we don't need to check for null and alternate hypothesis since we have guarantees that they are selected

	// we don't need to check for significance level since we have guarantees that it is above 0

	// we don't need to check for test mean since it can be any double value

	// check variables
	const auto [min, max] = HypothesisTest::variableCount(test);
	if (m_test->columns().size() < min || m_test->columns().size() > max) {
		ui.pbRecalculate->setToolTip(i18n("Selected test requires at least %1 and at most %2 non-empty variables to enable \"Recalculate\".", QString::number(min), QString::number(max)));
		ui.pbRecalculate->setEnabled(false);
		return;
	}

	ui.pbRecalculate->setToolTip(QString());
	ui.pbRecalculate->setEnabled(true);
}

void HypothesisTestDock::hideControls() {
	ui.lTestMean->hide();
	ui.sbTestMean->hide();
}

void HypothesisTestDock::testChanged() {
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	m_test->setTest(test);

	// reset the ui to a default state
	hideControls();

	// set symbols for the null and alternate hypotheses for the test
	ensureHypothesis();

	// ensure that the number of variables is correct for the test
	ensureVariableCount();

	// show any specific controls for the test
	if (test == HypothesisTest::Test::t_test_one_sample) {
		ui.lTestMean->show();
		ui.sbTestMean->show();
	}

	// check if the recalculate button should be enabled or disabled
	manageRecalculate();
}

void HypothesisTestDock::addVariable() {
	auto* layout = new QHBoxLayout();

	// size of the items in layout
	auto* lColumn = new QLabel(i18n("Variable %1:", QString::number(ui.variablesVerticalLayout->count() + 1)), this);
	layout->addWidget(lColumn);

	auto* cbColumn = new TreeViewComboBox(this);
	layout->addWidget(cbColumn);
	cbColumn->setTopLevelClasses({AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::Notebook, AspectType::Column});
	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Column});
	cbColumn->setModel(model);
	connect(cbColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::updateColumns); // update columns in aspect whenever treeviewcombobox changes
	// only checked once after updating columns since treeviewcombobox doesn't allow unselecting items
	connect(cbColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::manageRecalculate, Qt::SingleShotConnection);

	ui.variablesVerticalLayout->addLayout(layout);

	manageAddRemoveVariable(); // disable or enable add/remove variables buttons

	updateColumns();
}

void HypothesisTestDock::removeVariable() {
	int count = ui.variablesVerticalLayout->count();
	if (count > 0) {
		auto* layoutItem = ui.variablesVerticalLayout->takeAt(count - 1);
		auto* layout = layoutItem->layout();
		QLayoutItem* child;
		while ((child = layout->takeAt(0)) != nullptr) {
			delete child->widget(); // delete the label and TreeViewComboBox
			delete child; // delete the label and TreeViewComboBox layout item
		}
		delete layout; // delete the horizontal layout
	}

	manageAddRemoveVariable(); // disable or enable add/remove variables buttons

	updateColumns();
}
