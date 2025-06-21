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

#include "HypothesisTestDock.h"
#include "frontend/widgets/TreeViewComboBox.h"

/*!
  \class HypothesisTestDock
  \brief Provides a dock widget for statistical hypothesis tests.
  \ingroup frontend
*/
HypothesisTestDock::HypothesisTestDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	// combo boxes for sample columns
	cbColumn1 = new TreeViewComboBox(this);
	ui.gridLayout->addWidget(cbColumn1, 6, 3, 1, 3);
	cbColumn2 = new TreeViewComboBox(this);
	ui.gridLayout->addWidget(cbColumn2, 7, 3, 1, 3);
	cbColumn3 = new TreeViewComboBox(this);
	ui.gridLayout->addWidget(cbColumn3, 8, 3, 1, 3);

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::Notebook, AspectType::Column};
	cbColumn1->setTopLevelClasses(list);
	cbColumn2->setTopLevelClasses(list);
	cbColumn3->setTopLevelClasses(list);

	ui.pbLeveneTest->hide();
	ui.lCategorical->hide();
	ui.chbCategorical->hide();
	ui.lEqualVariance->hide();
	ui.chbEqualVariance->hide();
	ui.chbEqualVariance->setChecked(true);
	ui.lPopulationSigma->hide();
	ui.chbPopulationSigma->hide();
	ui.lePopulationSigma->hide();
	ui.pbRecalculate->setEnabled(false);

	const QString mu = UTF8_QSTRING("μ");
	const QString mu0 = UTF8_QSTRING("μₒ");
	ui.lPopulationSigma->setText(UTF8_QSTRING("σ"));
	ui.rbH1OneTail1->setText(i18n("%1 > %2", mu, mu0));
	ui.rbH1OneTail2->setText(i18n("%1 < %2", mu, mu0));
	ui.rbH1TwoTail->setText(i18n("%1 ≠ %2", mu, mu0));
	ui.rbH0OneTail1->setText(i18n("%1 ≤ %2", mu, mu0));
	ui.rbH0OneTail2->setText(i18n("%1 ≥ %2", mu, mu0));
	ui.rbH0TwoTail->setText(i18n("%1 = %2", mu, mu0));
	ui.lMuo->setText(QStringLiteral("μ₀"));
	ui.lAlpha->setText(QString::fromUtf8("α"));

	ui.leMuo->setText(QLocale().toString(0.0));
	ui.leAlpha->setText(QLocale().toString(0.05));
	ui.leMuo->setValidator(new QDoubleValidator(ui.leMuo));
	ui.leAlpha->setValidator(new QDoubleValidator(ui.leAlpha));

	ui.pbRecalculate->setIcon(QIcon::fromTheme(QLatin1String("run-build")));

	ui.rbH1TwoTail->setEnabled(false);
	ui.rbH1OneTail1->setEnabled(false);
	ui.rbH1OneTail2->setEnabled(false);

	retranslateUi();

	//**********************************  Slots **********************************************
	connect(cbColumn1, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::enableRecalculate);
	connect(cbColumn2, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::enableRecalculate);
	connect(cbColumn3, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::enableRecalculate);
	connect(ui.cbTest, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::testChanged);
	connect(ui.rbH0TwoTail, &QRadioButton::clicked, this, &HypothesisTestDock::onNullTwoTailClicked);
	connect(ui.rbH0OneTail1, &QRadioButton::clicked, this, &HypothesisTestDock::onNullOneTail1Clicked);
	connect(ui.rbH0OneTail2, &QRadioButton::clicked, this, &HypothesisTestDock::onNullOneTail2Clicked);
	connect(ui.pbRecalculate, &QPushButton::clicked, this, &HypothesisTestDock::runHypothesisTest);
}

void HypothesisTestDock::setTest(HypothesisTest* test) {
	CONDITIONAL_LOCK_RETURN
	m_test = test;
	setAspects(QList<AbstractAspect*>{m_test});

	// TODO load the settings
	testChanged(ui.cbTest->currentIndex());

	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Column});
	cbColumn1->setModel(model);
	cbColumn2->setModel(model);
	cbColumn3->setModel(model);
}

void HypothesisTestDock::retranslateUi() {
	ui.retranslateUi(this);

	ui.cbTest->clear();
	ui.cbTest->addItem(i18n("One-Sample t-Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Two-Sample t-Test (Independent)"), static_cast<int>(HypothesisTest::Test::t_test_two_sample));
	ui.cbTest->addItem(i18n("Paired t-Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("One-Way ANOVA"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Mann-Whitney U Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Kruskal-Wallis Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));
	ui.cbTest->addItem(i18n("Logrank Test"), static_cast<int>(HypothesisTest::Test::t_test_one_sample));

	// tooltip texts
	QString info = i18n(
		"<ul>"
		"<li><b>One-Sample t-Test</b> - tests whether the mean of a single sample differs significantly from a known or hypothesized population mean</li>"
		"<li><b>Two-Sample t-Test (Independent)</b> - compares the means of two independent groups to determine if they are significantly different</li>"
		"<li><b>Paired t-Test</b> - tests the difference between paired observations (e.g., before and after measurements) to see if the mean difference is significantly different from zero</li>"
		"<li><b>One-Way ANOVA</b> - tests whether there are significant differences among the means of three or more independent groups</li>"
		"<li><b>Mann-Whitney U Test</b> - </li>"
		"<li><b>Kruskal-Wallis Test</b> - </li>"
		"<li><b>Logrank Test</b> - </li>"
		"</ul>"
	);
	ui.lTest->setToolTip(info);
	ui.cbTest->setToolTip(info);
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void HypothesisTestDock::testChanged(int) {
	const auto test = static_cast<HypothesisTest::Test>(ui.cbTest->currentData().toInt());

	switch (test) {
	case HypothesisTest::Test::t_test_one_sample:
		ui.lColumn2->hide();
		cbColumn2->hide();
		ui.lColumn3->hide();
		cbColumn3->hide();
		break;
	case HypothesisTest::Test::t_test_two_sample:
		ui.lColumn2->show();
		cbColumn2->show();
		ui.lColumn3->hide();
		cbColumn3->hide();
		break;
	case HypothesisTest::Test::t_test_paired:

		break;
	case HypothesisTest::Test::one_way_anova:

		break;
	case HypothesisTest::Test::mann_whitney_u_test:

		break;
	case HypothesisTest::Test::kruskal_wallis_test:

		break;
	case HypothesisTest::Test::log_rank_test:

		break;
	default:

		break;
	}
}

void HypothesisTestDock::onNullTwoTailClicked() {
	ui.rbH1TwoTail->setChecked(true);
	ui.rbH1TwoTail->setEnabled(true);
	ui.rbH1OneTail1->setEnabled(false);
	ui.rbH1OneTail2->setEnabled(false);
}

void HypothesisTestDock::onNullOneTail1Clicked() {
	ui.rbH1OneTail1->setChecked(true);
	ui.rbH1OneTail1->setEnabled(true);
	ui.rbH1TwoTail->setEnabled(false);
	ui.rbH1OneTail2->setEnabled(false);
}

void HypothesisTestDock::onNullOneTail2Clicked() {
	ui.rbH1OneTail2->setChecked(true);
	ui.rbH1OneTail2->setEnabled(true);
	ui.rbH1TwoTail->setEnabled(false);
	ui.rbH1OneTail1->setEnabled(false);
}

void HypothesisTestDock::runHypothesisTest() {
	if (ui.rbH0TwoTail->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullEquality);
		m_test->setTail(nsl_stats_tail_type_two);
	} else if (ui.rbH0OneTail1->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullLessEqual);
		m_test->setTail(nsl_stats_tail_type_positive);
	} else if (ui.rbH0OneTail2->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullHypothesisType::NullGreaterEqual);
		m_test->setTail(nsl_stats_tail_type_negative);
	}

	bool ok = false;
	double popMean = QLocale().toDouble(ui.leMuo->text(), &ok);
	if (!ok)
		return; // TODO: Handle conversion error
	m_test->setPopulationMean(popMean);

	double alpha = QLocale().toDouble(ui.leAlpha->text(), &ok);
	if (!ok)
		return; // TODO: Handle conversion error
	m_test->setSignificanceLevel(alpha);

	QVector<Column*> columns;
	columns << reinterpret_cast<Column*>(cbColumn1->currentModelIndex().internalPointer());
	if (cbColumn2->isVisible())
		columns << reinterpret_cast<Column*>(cbColumn2->currentModelIndex().internalPointer());
	if (cbColumn2->isVisible())
		columns << reinterpret_cast<Column*>(cbColumn3->currentModelIndex().internalPointer());
	m_test->setColumns(columns);

	m_test->runTest();
}

void HypothesisTestDock::enableRecalculate() {
	bool ok = false;
	QLocale().toDouble(ui.leMuo->text(), &ok);
	if (!ok) {
		ui.pbRecalculate->setToolTip(i18n("Provide a numerical value for the test mean"));
		ui.pbRecalculate->setEnabled(false);
	}

	QLocale().toDouble(ui.leAlpha->text(), &ok);
	if (!ok) {
		ui.pbRecalculate->setToolTip(i18n("Provide a numerical value for the confidence level"));
		ui.pbRecalculate->setEnabled(false);
	}

	ui.pbRecalculate->setToolTip(QString());
	ui.pbRecalculate->setEnabled(true);
}
