/*
	File                 : HypothesisTestDock.cpp
	Project              : LabPlot
	Description          : Dock for Hypothesis Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HypothesisTestDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/column/Column.h"
#include "backend/statistics/HypothesisTest.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KLocalizedString>
#include <KMessageWidget>

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

	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	retranslateUi();

	//**********************************  Slots **********************************************
	connect(ui.cbTest, &QComboBox::currentIndexChanged, this, &HypothesisTestDock::testChanged);
	connect(ui.pbRecalculate, &QPushButton::clicked, this, &HypothesisTestDock::recalculate);

	connect(m_buttonNew, &QPushButton::clicked, this, &HypothesisTestDock::addVariable);

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

	if (m_dataComboBoxes.isEmpty())
		addVariable(); // create the first variable which is never deleted

	auto* firstTreeViewCb = static_cast<TreeViewComboBox*>(m_dataComboBoxes.at(0)); // treeviewcombobox for the first variable
	firstTreeViewCb->setModel(aspectModel());
	firstTreeViewCb->setAspect(nullptr); // clear the current aspect

	load(); // load the remaining properties
	showStatusError(QString()); // remove the message from the previous test, if available

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

	connect(m_test, &HypothesisTest::statusError, this, &HypothesisTestDock::showStatusError);
}

void HypothesisTestDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;
	ui.retranslateUi(this);

	ui.cbTest->clear();
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::t_test_one_sample), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::t_test_two_sample), static_cast<int>(HypothesisTest::Test::t_test_two_sample));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::t_test_two_sample_paired), static_cast<int>(HypothesisTest::Test::t_test_two_sample_paired));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::t_test_welch), static_cast<int>(HypothesisTest::Test::t_test_welch));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::one_way_anova), static_cast<int>(HypothesisTest::Test::one_way_anova));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::one_way_anova_repeated), static_cast<int>(HypothesisTest::Test::one_way_anova_repeated));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::mann_whitney_u_test), static_cast<int>(HypothesisTest::Test::mann_whitney_u_test));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::wilcoxon_test), static_cast<int>(HypothesisTest::Test::wilcoxon_test));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::kruskal_wallis_test), static_cast<int>(HypothesisTest::Test::kruskal_wallis_test));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::friedman_test), static_cast<int>(HypothesisTest::Test::friedman_test));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::log_rank_test), static_cast<int>(HypothesisTest::Test::log_rank_test));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::chisq_independence), static_cast<int>(HypothesisTest::Test::chisq_independence));
	ui.cbTest->addItem(HypothesisTest::testName(HypothesisTest::Test::chisq_goodness_of_fit), static_cast<int>(HypothesisTest::Test::chisq_goodness_of_fit));

	// tooltip texts
	QString info = i18n(
		"<ul>"
		"<li><b>%1</b> - tests if a sample mean differs significantly from a known population mean</li>"
		"<li><b>%2</b> - tests if two independent samples have the same mean</li>"
		"<li><b>%3</b> - tests if the mean difference between two related samples is zero</li>"
		"<li><b>%4</b> - tests if two independent samples with unequal variances have the same mean</li>"
		"<li><b>%5</b> - tests if three or more independent samples have the same mean</li>"
		"<li><b>%6</b> - tests if three or more related/repeated measurements have the same mean</li>",
		HypothesisTest::testName(HypothesisTest::Test::t_test_one_sample),
		HypothesisTest::testName(HypothesisTest::Test::t_test_two_sample),
		HypothesisTest::testName(HypothesisTest::Test::t_test_two_sample_paired),
		HypothesisTest::testName(HypothesisTest::Test::t_test_welch),
		HypothesisTest::testName(HypothesisTest::Test::one_way_anova),
		HypothesisTest::testName(HypothesisTest::Test::one_way_anova_repeated)
	) + i18n(
		"<li><b>%1</b> - tests differences in medians between two independent groups</li>"
		"<li><b>%2</b> - tests if the median difference between two related samples is zero</li>"
		"<li><b>%3</b> - tests differences in medians among three or more independent groups</li>"
		"<li><b>%4</b> - tests differences in medians among three or more related/repeated measurements</li>"
		"<li><b>%5</b> - tests differences in survival distributions between two or more groups</li>"
		"<li><b>%6</b> - tests if two categorical variables are independent of each other</li>"
		"<li><b>%7</b> - tests if observed data follows a specified theoretical distribution</li>"
		"</ul>",
		HypothesisTest::testName(HypothesisTest::Test::mann_whitney_u_test),
		HypothesisTest::testName(HypothesisTest::Test::wilcoxon_test),
		HypothesisTest::testName(HypothesisTest::Test::kruskal_wallis_test),
		HypothesisTest::testName(HypothesisTest::Test::friedman_test),
		HypothesisTest::testName(HypothesisTest::Test::log_rank_test),
		HypothesisTest::testName(HypothesisTest::Test::chisq_independence),
		HypothesisTest::testName(HypothesisTest::Test::chisq_goodness_of_fit)
	);

	ui.lTest->setToolTip(info);
	ui.cbTest->setToolTip(info);
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
/*!
* This slot is called when the user clicks the recalculate button.
* It triggers the recalculation of the hypothesis test with the current settings.
*/
void HypothesisTestDock::recalculate() {
	m_test->recalculate();
}

/*! 
* This slot is called when the user changes the test type in the combobox.
* It updates the UI to reflect the selected test type, including showing/hiding controls and updating
* the number of variables needed for the test.
*/
void HypothesisTestDock::testChanged() {
	// set symbols for the null and alternate hypotheses for the test
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	ensureHypothesis(test);

	// ensure that the number of variables is correct for the test
	ensureVariableCount(test);

	// show any specific controls for the test
	const bool visible = (test == HypothesisTest::Test::t_test_one_sample);
	ui.lTestMean->setVisible(visible);
	ui.sbTestMean->setVisible(visible);

	// check if the recalculate button should be enabled or disabled
	manageRecalculate();

	if (m_initializing)
		return;

	m_test->setTest(test);

	// we need to call this here again for the case of when we has previously
	// selected variable columns and we change the selected hypothesis test. The aspect
	// clears its list of columns, while the gui still shows columns as selected.
	updateColumns();
}

// ##############################################################################
// #################################*  Helpers   ################################
// ##############################################################################
void HypothesisTestDock::ensureHypothesis(HypothesisTest::Test test) {
	int hypothesisCount = HypothesisTest::hypothesisCount(test);
	auto hypothesisTexts = HypothesisTest::hypothesisText(test);

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

void HypothesisTestDock::ensureVariableCount(HypothesisTest::Test test) {
	// get the min and max number of columns needed for the test
	const auto [min, max] = HypothesisTest::variableCount(test);

	// function pointer to add or remove variables
	void (HypothesisTestDock::*func)() = nullptr;

	// difference between the current number of columns and the min/max number of columns needed for the test
	int diff = 0;

	// get the current number of columns
	const int varCount = m_dataComboBoxes.size();

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
	for (int i = diff; i > 0; --i)
		(this->*func)();

	// confirm that the number of columns is correct for the test
	const int newVarCount = m_dataComboBoxes.size();
	Q_ASSERT(newVarCount >= min && newVarCount <= max);

	// enable or disable the add/remove variables buttons
	manageAddRemoveVariable();

	// update the columns in the aspect
	if (!m_initializing)
		updateColumns();
}

void HypothesisTestDock::manageAddRemoveVariable() {
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	const auto [min, max] = HypothesisTest::variableCount(test);
	const int count = m_dataComboBoxes.size();

	for (auto* btn : m_removeButtons)
		btn->setEnabled(count > min);

	m_buttonNew->setEnabled(count < max);
}

// called:
// - when the user has selected a column in the treeviewcombobox
// - when the user has added or removed a column
// - in ensureVariableCount()
void HypothesisTestDock::updateColumns() {
	if (m_initializing)
		return;

	QVector<const AbstractColumn*> columns;
	for (auto* treeViewCb : m_dataComboBoxes) {
		auto* col = dynamic_cast<AbstractColumn*>(treeViewCb->currentAspect());
		if (col)
			columns << col;
	}

	m_test->setDataColumns(columns);
}

void HypothesisTestDock::manageRecalculate() {
	// we don't need to check for null and alternate hypothesis since we have guarantees that they are selected
	// we don't need to check for significance level since we have guarantees that it is above 0
	// we don't need to check for test mean since it can be any double value

	// check variables
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());
	const auto [min, max] = HypothesisTest::variableCount(test);
	if (m_test->dataColumns().size() < min || m_test->dataColumns().size() > max) {
		ui.pbRecalculate->setToolTip(i18n("Selected test requires at least %1 and at most %2 non-empty variables.", QString::number(min), QString::number(max)));
		ui.pbRecalculate->setEnabled(false);
		return;
	}

	ui.pbRecalculate->setToolTip(QString());
	ui.pbRecalculate->setEnabled(true);
}

void HypothesisTestDock::addVariable() {
	auto* model = aspectModel();
	model->enableNonEmptyNumericColumnsOnly(true);
	model->setSelectableAspects({AspectType::Column});

	auto* cbColumn = new TreeViewComboBox(this);
	cbColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbColumn->setModel(model);
	connect(cbColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::updateColumns); // update columns in aspect whenever treeviewcombobox changes
	connect(cbColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::manageRecalculate, Qt::SingleShotConnection); // only checked once after updating columns since treeviewcombobox doesn't allow unselecting items

	const int index = m_dataComboBoxes.size();
	if (index == 0) {
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(cbColumn->sizePolicy().hasHeightForWidth());
		cbColumn->setSizePolicy(sizePolicy1);
	} else {
		auto* button = new QPushButton();
		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(button, &QPushButton::clicked, this, &HypothesisTestDock::removeVariable);
		ui.gridLayout_4->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	ui.gridLayout_4->addWidget(cbColumn, index, 0, 1, 1);
	ui.gridLayout_4->addWidget(m_buttonNew, index + 1, 1, 1, 1);
	m_dataComboBoxes << cbColumn;

	if (!m_removeButtons.isEmpty())
		ui.lVariables->setText(i18n("Columns:"));
	else
		ui.lVariables->setText(i18n("Column:"));

	manageAddRemoveVariable(); // disable or enable add/remove variables buttons

	if (!m_initializing)
		updateColumns();
}

void HypothesisTestDock::removeVariable() {
	auto* sender = dynamic_cast<QPushButton*>(QObject::sender());
	if (sender) {
		// remove button was clicked, determine which one and
		// delete it together with the corresponding combobox
		for (int i = 0; i < m_removeButtons.count(); ++i) {
			if (sender == m_removeButtons.at(i)) {
				delete m_dataComboBoxes.takeAt(i + 1);
				delete m_removeButtons.takeAt(i);
			}
		}
	} else {
		// no sender is available, the function is being called directly
		if (m_removeButtons.count() > 0) {
			delete m_dataComboBoxes.takeLast();
			delete m_removeButtons.takeLast();
		}
	}

	if (!m_removeButtons.isEmpty())
		ui.lVariables->setText(i18n("Columns:"));
	else
		ui.lVariables->setText(i18n("Column:"));

	manageAddRemoveVariable(); // disable or enable add/remove variables buttons

	if (!m_initializing)
		updateColumns();
}

void HypothesisTestDock::showStatusError(const QString& info) {
	if (info.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			ui.gridLayout->addWidget(m_messageWidget, 17, 0, 1, 3);
		}
		m_messageWidget->setText(info);
		m_messageWidget->animatedShow();
		QDEBUG(info);
	}
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void HypothesisTestDock::load() {
	// clear all variables except the first
	for (int i = 0, count = m_removeButtons.size(); i < count; i++)
		removeVariable();

	// restore variables from aspect columns
	for (int i = 0; i < m_test->dataColumns().size(); i++) {
		auto* col = m_test->dataColumns().at(i);

		if (i != 0) // no need to add for the first since it was never deleted
			addVariable();

		auto* treeViewCb = static_cast<TreeViewComboBox*>(m_dataComboBoxes.at(i)); // treeviewcombobox for the just added variable above
		treeViewCb->setModel(aspectModel());
		treeViewCb->setAspect(col, col->path()); // set the aspect for the treeviewcombobox to the column
	}

	// restore significance level
	ui.sbSignificanceLevel->setValue(m_test->significanceLevel());

	// restore test mean
	ui.sbTestMean->setValue(m_test->testMean());

	// restore null hypothesis
	const int hypothesisCount = HypothesisTest::hypothesisCount(m_test->test());
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
}
