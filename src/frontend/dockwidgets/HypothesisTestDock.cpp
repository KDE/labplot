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

	ui.cbTest->clear();
	ui.cbTest->addItem(i18n("One-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Independent Two-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_two_sample));
	ui.cbTest->addItem(i18n("Paired Two-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_two_sample_paired));
	// ui.cbTest->addItem(i18n("One-Way ANOVA"), static_cast<int>(HypothesisTest::Test::one_way_anova));
	// ui.cbTest->addItem(i18n("Mann-Whitney U Test"), static_cast<int>(HypothesisTest::Test::mann_whitney_u_test));
	// ui.cbTest->addItem(i18n("Kruskal-Wallis Test"), static_cast<int>(HypothesisTest::Test::kruskal_wallis_test));
	// ui.cbTest->addItem(i18n("Logrank Test"), static_cast<int>(HypothesisTest::Test::log_rank_test));

	ui.lTestMean->setText(QStringLiteral("μ₀"));
	ui.lSignificanceLevel->setText(QString::fromUtf8("α"));

	ui.sbSignificanceLevel->setMinimumNotEqual(0);

	ui.pbRecalculate->setIcon(QIcon::fromTheme(QLatin1String("run-build")));

	// tooltip texts
	QString info = i18n(
		"<ul>"
		"<li><b>One-Sample t-Test</b> - tests if a sample mean differs significantly from a known population mean</li>"
		"<li><b>Independent Two-Sample t-Test</b> - tests if two independent samples have the same mean</li>"
		"<li><b>Paired Two-Sample t-Test</b> - tests if the mean difference between two related samples is zero</li>"
		// 	"<li><b>One-Way ANOVA</b> - tests if three or more independent samples have the same mean</li>"
		// 	"<li><b>Mann-Whitney U Test</b> - tests differences in medians between two independent groups</li>"
		// 	"<li><b>Kruskal-Wallis Test</b> - tests differences in medians among three or more independent groups</li>"
		// 	"<li><b>Logrank Test</b> - tests differences in survival distributions between two or more groups</li>"
		"</ul>"
	);

	ui.lTest->setToolTip(info);
	ui.cbTest->setToolTip(info);

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

	if (m_test) {
		disconnect(ui.sbSignificanceLevel, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setSignificanceLevel);
		disconnect(ui.sbTestMean, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setTestMean);
	}
	disconnect(m_rbNullTwoTailedAspectConn);
	disconnect(m_rbNullOneTailedLeftAspectConn);
	disconnect(m_rbNullOneTailedRightAspectConn);
	disconnect(m_cbTestAspectConn);

	m_test = test;
	setAspects(QList<AbstractAspect*>{m_test});

	// clear past variables
	for (int i = 0, count = ui.variablesVerticalLayout->count(); i < count; i++)
		removeVariable(false);

	// confirm that all variables are cleared
	Q_ASSERT(ui.variablesVerticalLayout->count() == 0);

	// restore variables from aspect columns
	for (auto* col : m_test->columns()) {
		addVariable(false);
		int varCount = ui.variablesVerticalLayout->count(); // layoutitem for the just added variable above
		auto* layout = ui.variablesVerticalLayout->itemAt(varCount - 1)->layout();
		auto* treeViewCb = static_cast<TreeViewComboBox*>(layout->itemAt(1)->widget());
		treeViewCb->setAspect(col);
	}

	// restore significance level
	ui.sbSignificanceLevel->setValue(m_test->significanceLevel());

	// restore null hypothesis
	switch (m_test->nullHypothesis()) {
	case HypothesisTest::NullHypothesisType::NullEquality:
		ui.rbNullTwoTailed->setChecked(true);
		break;
	case HypothesisTest::NullHypothesisType::NullLessEqual:
		ui.rbNullOneTailedLeft->setChecked(true);
		break;
	case HypothesisTest::NullHypothesisType::NullGreaterEqual:
		ui.rbNullOneTailedRight->setChecked(true);
		break;
	}

	// restore test mean
	ui.sbTestMean->setValue(m_test->testMean());

	int oldIndex = ui.cbTest->currentIndex();
	ui.cbTest->setCurrentIndex(static_cast<int>(m_test->test())); // auto trigger testChanged(int)
	int newIndex = ui.cbTest->currentIndex();
	if (oldIndex == newIndex)
		testChanged(newIndex); // manually trigger testChanged(int)

	connect(ui.sbSignificanceLevel, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setSignificanceLevel);
	connect(ui.sbTestMean, qOverload<double>(&NumberSpinBox::valueChanged), m_test, &HypothesisTest::setTestMean);

	m_rbNullTwoTailedAspectConn = connect(ui.rbNullTwoTailed, &QRadioButton::toggled, [this] {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullEquality);
	});
	m_rbNullOneTailedLeftAspectConn = connect(ui.rbNullOneTailedLeft, &QRadioButton::toggled, [this] {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullLessEqual);
	});
	m_rbNullOneTailedRightAspectConn = connect(ui.rbNullOneTailedRight, &QRadioButton::toggled, [this] {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullGreaterEqual);
	});
	m_cbTestAspectConn = connect(ui.cbTest, &QComboBox::currentIndexChanged, [this](int i) {
		m_test->setTest(static_cast<HypothesisTest::Test>(i));
	});
}

void HypothesisTestDock::retranslateUi() {
	ui.retranslateUi(this);
}

void HypothesisTestDock::setHypothesisText(HypothesisTest::Test test) {
	QString a;
	QString b;

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		a = UTF8_QSTRING("μ");
		b = UTF8_QSTRING("μₒ");
		break;
	case HypothesisTest::Test::t_test_two_sample:
	case HypothesisTest::Test::t_test_two_sample_paired:
		a = QStringLiteral("µ<sub>1</sub>");
		b = QStringLiteral("µ<sub>2</sub>");
		break;
	}

	ui.lNullOneTailedLeft->setText(a + QStringLiteral(" ≤ ") + b);
	ui.lNullOneTailedRight->setText(a + QStringLiteral(" ≥ ") + b);
	ui.lNullTwoTailed->setText(a + QStringLiteral(" = ") + b);
	ui.lAlternateOneTailedLeft->setText(a + QStringLiteral(" > ") + b);
	ui.lAlternateOneTailedRight->setText(a + QStringLiteral(" &lt; ") + b);
	ui.lAlternateTwoTailed->setText(a + QStringLiteral(" ≠ ") + b);
}

void HypothesisTestDock::ensureVariableCount(HypothesisTest::Test test) {
	const auto [min, max] = HypothesisTest::variableCount(test);

	void (HypothesisTestDock::*func)(bool) = nullptr;
	int diff = 0;
	int varCount = ui.variablesVerticalLayout->count();

	if (varCount < min) {
		func = &HypothesisTestDock::addVariable;
		diff = qAbs(min - varCount);
	} else if (varCount > max) {
		func = &HypothesisTestDock::removeVariable;
		diff = qAbs(varCount - max);
	}

	for (int i = diff; i > 0; --i) {
		(this->*func)(true);
	}

	int newVarCount = ui.variablesVerticalLayout->count();

	Q_ASSERT(newVarCount >= min && newVarCount <= max);

	manageAddRemoveVariable(test);
}

void HypothesisTestDock::manageAddRemoveVariable(HypothesisTest::Test test) {
	const auto [min, max] = HypothesisTest::variableCount(test);
	const int count = ui.variablesVerticalLayout->count();

	ui.tbRemoveVariable->setEnabled(count > min);
	ui.tbAddVariable->setEnabled(count < max);
}

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

	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	const auto [min, max] = HypothesisTest::variableCount(test);

	if (columns.size() >= min && columns.size() <= max)
		m_test->setColumns(columns);
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
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

void HypothesisTestDock::testChanged(int) {
	hideControls();

	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());

	setHypothesisText(test);
	ensureVariableCount(test);

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		ui.lTestMean->show();
		ui.sbTestMean->show();
		break;
	}

	manageRecalculate();
}

void HypothesisTestDock::recalculate() {
	if (ui.rbNullTwoTailed->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullEquality);
	} else if (ui.rbNullOneTailedLeft->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullLessEqual);
	} else if (ui.rbNullOneTailedRight->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullGreaterEqual);
	}

	m_test->setSignificanceLevel(ui.sbSignificanceLevel->value());

	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	m_test->setTest(test);

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		m_test->setTestMean(ui.sbTestMean->value());
		break;
	}

	updateColumns();

	m_test->recalculate();
}

void HypothesisTestDock::addVariable(bool updateCols) {
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

	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	manageAddRemoveVariable(test); // disable or enable add/remove variables buttons

	if (updateCols)
		updateColumns();
}

void HypothesisTestDock::removeVariable(bool updateCols) {
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

	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	manageAddRemoveVariable(test); // disable or enable add/remove variables buttons

	if (updateCols)
		updateColumns();
}
