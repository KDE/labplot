/*
	File                 : HypothesisTestDock.cpp
	Project              : LabPlot
	Description          : Dock for One-Sample T-Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HypothesisTestDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QDoubleValidator>
#include <QIcon>
#include <QLocale>
#include <QPushButton>

/*!
  \class HypothesisTestDock
  \brief Provides a dock (widget) for One-Sample T-Test.
  \ingroup frontend
*/
HypothesisTestDock::HypothesisTestDock(QWidget* parent)
	: BaseDock(parent)
{
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	spreadsheetComboBox = new TreeViewComboBox;
	ui.gridLayout->addWidget(spreadsheetComboBox, 4, 3, 1, 3);

	ui.cbTest->addItem(i18n("One-Sample T-Test"));

	ui.lPopulationSigma->setText(UTF8_QSTRING("σ"));
	ui.chbCalculateStats->setChecked(true);

	ui.pbLeveneTest->hide();
	ui.lCategorical->hide();
	ui.chbCategorical->hide();
	ui.lEqualVariance->hide();
	ui.chbEqualVariance->hide();
	ui.chbEqualVariance->setChecked(true);
	ui.lPopulationSigma->hide();
	ui.chbPopulationSigma->hide();
	ui.lePopulationSigma->hide();
	ui.pbPerformTest->setEnabled(false);

	const QString mu = UTF8_QSTRING("μ");
	const QString mu0 = UTF8_QSTRING("μₒ");

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
	ui.pbPerformTest->setIcon(QIcon::fromTheme(QLatin1String("run-build")));

	ui.rbH1TwoTail->setEnabled(false);
	ui.rbH1OneTail1->setEnabled(false);
	ui.rbH1OneTail2->setEnabled(false);

	connect(ui.rbH0TwoTail, &QRadioButton::clicked, this, &HypothesisTestDock::onNullTwoTailClicked);
	connect(ui.rbH0OneTail1, &QRadioButton::clicked, this, &HypothesisTestDock::onNullOneTail1Clicked);
	connect(ui.rbH0OneTail2, &QRadioButton::clicked, this, &HypothesisTestDock::onNullOneTail2Clicked);

	connect(spreadsheetComboBox, &TreeViewComboBox::currentModelIndexChanged,
			this, &HypothesisTestDock::onSpreadsheetSelectionChanged);
	connect(ui.pbPerformTest, &QPushButton::clicked,
			this, &HypothesisTestDock::runHypothesisTest);

	ui.cbTest->setCurrentIndex(0);
	Q_EMIT ui.cbTest->currentIndexChanged(0);
}

void HypothesisTestDock::initializeTest(HypothesisTest* test) {
	CONDITIONAL_LOCK_RETURN
		m_test = test;

	m_aspectTreeModel = new AspectTreeModel(m_test->project());

	const QList<AspectType> topLevelAspects{AspectType::Folder, AspectType::Workbook,
											AspectType::Spreadsheet, AspectType::LiveDataSource};
	spreadsheetComboBox->setTopLevelClasses(topLevelAspects);

	const QList<AspectType> selectableAspects{AspectType::Spreadsheet, AspectType::LiveDataSource};
	m_aspectTreeModel->setSelectableAspects(selectableAspects);
	spreadsheetComboBox->setModel(m_aspectTreeModel);

	ui.leName->setText(m_test->name());
	ui.teComment->setText(m_test->comment());
	updateAspectComboBoxIndex(spreadsheetComboBox, m_test->getDataSourceSpreadsheet());
	refreshColumnComboBox(m_test->getDataSourceSpreadsheet());

	connect(m_test, &HypothesisTest::aspectDescriptionChanged,
			this, &HypothesisTestDock::aspectDescriptionChanged);
}

void HypothesisTestDock::updateAspectComboBoxIndex(TreeViewComboBox* comboBox, const AbstractAspect* aspect) {
	if (aspect)
		comboBox->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		comboBox->setCurrentModelIndex(QModelIndex());
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
		m_test->setNullHypothesis(HypothesisTest::NullEquality);
		m_test->setTail(HypothesisTest::TailTwo);
	} else if (ui.rbH0OneTail1->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullLessEqual);
		m_test->setTail(HypothesisTest::TailPositive);
	} else if (ui.rbH0OneTail2->isChecked()) {
		m_test->setNullHypothesis(HypothesisTest::NullGreaterEqual);
		m_test->setTail(HypothesisTest::TailNegative);
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

	if (ui.cbCol1->count() == 0)
		return;
	QVector<Column*> columns;
	columns << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());
	m_test->setColumns(columns);

	m_test->runTest();
}

void HypothesisTestDock::onSpreadsheetSelectionChanged(const QModelIndex& index) {
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	refreshColumnComboBox(spreadsheet);
	m_test->setDataSourceSpreadsheet(spreadsheet);
}

void HypothesisTestDock::refreshColumnComboBox(Spreadsheet* spreadsheet) {
	ui.cbCol1->clear();
	for (auto* col : spreadsheet->children<Column>()) {
		if (col->columnMode() == AbstractColumn::ColumnMode::Integer ||
			col->columnMode() == AbstractColumn::ColumnMode::Double)
			ui.cbCol1->addItem(col->name(), qint64(col));
	}
	ui.pbPerformTest->setEnabled(hasSelectedColumns());
}

bool HypothesisTestDock::hasSelectedColumns() const {
	return ui.cbCol1->isVisible() && ui.cbCol1->count() > 0;
}
