﻿/***************************************************************************
	File                 : HypothesisTestDock.cpp
	Project              : LabPlot
	Description          : widget for hypothesis test properties
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "HypothesisTestDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/TemplateHandler.h"

#include <QAbstractItemModel>
#include <QDir>
#include <QStandardItemModel>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>

/*!
  \class HypothesisTestDock
  \brief Provides a dock (widget) for hypothesis testing:
  \ingroup kdefrontend
*/

//TODO: To add tooltips in docks for non obvious widgets.
//TODO: Add functionality for database along with spreadsheet.

HypothesisTestDock::HypothesisTestDock(QWidget* parent) : BaseDock(parent) {
	//QDEBUG("in hypothesis test constructor ");
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout->addWidget(cbSpreadsheet, 5, 4, 1, 3);

	// adding item to tests and testtype combo box;
	ui.cbTest->addItem( i18n("T Test"), HypothesisTest::TTest);
	ui.cbTest->addItem( i18n("Z Test"), HypothesisTest::ZTest);
	ui.cbTest->addItem( i18n("ANOVA"), HypothesisTest::Anova);

	ui.lPopulationSigma->setText( UTF8_QSTRING("σ"));

	ui.chbCalculateStats->setChecked(true);

	// making all test blocks invisible at starting.
	ui.pbLeveneTest->hide();
	ui.lCategorical->hide();
	ui.chbCategorical->hide();
	ui.lCalculateStats->hide();
	ui.chbCalculateStats->hide();
	ui.lCol1->hide();
	ui.cbCol1->hide();
	ui.lCol2->hide();
	ui.cbCol2->hide();
	ui.lCol3->hide();
	ui.cbCol3->hide();
	ui.lEqualVariance->hide();
	ui.chbEqualVariance->hide();
	ui.chbEqualVariance->setChecked(true);
	ui.lPopulationSigma->hide();
	ui.lPopulationSigma->setToolTip( i18n("Sigma of Population") + QLatin1String(" <br/> <br/> ") +
									 i18n("Hint: Z-Test if preffered over T-Test if this is known"));
	ui.chbPopulationSigma->hide();
	ui.lePopulationSigma->hide();
	ui.pbPerformTest->setEnabled(false);
	ui.rbH1OneTail2->hide();
	ui.rbH1OneTail1->hide();
	ui.rbH1TwoTail->hide();
	ui.rbH0OneTail1->hide();
	ui.rbH0OneTail2->hide();
	ui.rbH0TwoTail->hide();
	ui.lH0->hide();
	ui.lH1->hide();

	QString mu = UTF8_QSTRING("μ");
	QString mu0 = UTF8_QSTRING("μₒ");

	// radio button for null and alternate hypothesis
	// for alternative hypothesis (h1)
	// one_tail_1 is mu > mu0; one_tail_2 is mu < mu0; two_tail = mu != mu0;

	ui.rbH1OneTail1->setText( i18n("%1 %2 %3", mu, UTF8_QSTRING(">"), mu0));
	ui.rbH1OneTail2->setText( i18n("%1 %2 %3", mu, UTF8_QSTRING("<"), mu0));
	ui.rbH1TwoTail->setText( i18n("%1 %2 %3", mu, UTF8_QSTRING("≠"), mu0));

	ui.rbH0OneTail1->setText( i18n("%1 %2 %3",mu, UTF8_QSTRING("≤"), mu0));
	ui.rbH0OneTail2->setText( i18n("%1 %2 %3", mu, UTF8_QSTRING("≥"), mu0));
	ui.rbH0TwoTail->setText( i18n("%1 %2 %3", mu, UTF8_QSTRING("="), mu0));

	ui.rbH0TwoTail->setEnabled(false);
	ui.rbH0OneTail1->setEnabled(false);
	ui.rbH0OneTail2->setEnabled(false);


	// setting muo and alpha buttons
	ui.lMuo->setText( i18n("%1", mu0));
	ui.lAlpha->setText( i18n("%1", UTF8_QSTRING("α")));
	ui.leMuo->setText( i18n("%1", m_populationMean));
	ui.leAlpha->setText( i18n("%1", m_significanceLevel));

	ui.lMuo->hide();
	ui.lMuo->setToolTip( i18n("Population Mean"));
	ui.lAlpha->hide();
	ui.lAlpha->setToolTip( i18n("Significance Level"));
	ui.leMuo->hide();
	ui.leAlpha->hide();
	ui.pbPerformTest->setIcon(QIcon::fromTheme(QLatin1String("run-build")));

	ui.leMuo->setText( i18n("%1", m_populationMean));
	ui.leAlpha->setText( i18n("%1", m_significanceLevel));

	//    readConnections();

	//    auto* style = ui.bAddRow->style();
	//    ui.bAddRow->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	//    ui.bAddRow->setToolTip(i18n("Add the selected field to rows"));
	//    ui.bRemoveRow->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	//    ui.bRemoveRow->setToolTip(i18n("Remove the selected field from rows"));

	//    ui.bAddColumn->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
	//    ui.bAddColumn->setToolTip(i18n("Add the selected field to columns"));
	//    ui.bRemoveColumn->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	//    ui.bRemoveColumn->setToolTip(i18n("Remove the selected field from columns"));

	//    //add/remove buttons only enabled if something was selected
	//    ui.bAddRow->setEnabled(false);
	//    ui.bRemoveRow->setEnabled(false);
	//    ui.bAddColumn->setEnabled(false);
	//    ui.bRemoveColumn->setEnabled(false);

	//    connect(ui.leName, &QLineEdit::textChanged, this, &HypothesisTestDock::nameChanged);
	//    connect(ui.teComment, &QLineEdit::textChanged, this, &HypothesisTestDock::commentChanged);


	connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &HypothesisTestDock::spreadsheetChanged);

	//    connect(ui.bAddRow,  &QPushButton::clicked, this, &HypothesisTestDock::addRow);
	//    connect(ui.bRemoveRow, &QPushButton::clicked, this,&HypothesisTestDock::removeRow);
	//    connect(ui.bAddColumn,  &QPushButton::clicked, this, &HypothesisTestDock::addColumn);
	//    connect(ui.bRemoveColumn, &QPushButton::clicked, this,&HypothesisTestDock::removeColumn);

	//      connect(ui.cbCol1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::doTTest);
	//      connect(ui.cbCol2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::doTTest);

	//    connect(ui.lwFields, &QListWidget::itemSelectionChanged, this, [=]() {
	//        bool enabled = !ui.lwFields->selectedItems().isEmpty();
	//        ui.bAddRow->setEnabled(enabled);
	//        ui.bAddColumn->setEnabled(enabled);
	//    });

	//    connect(ui.lwRows, &QListWidget::doubleClicked, this,&HypothesisTestDock::removeRow);
	//    connect(ui.lwRows, &QListWidget::itemSelectionChanged, this, [=]() {
	//        ui.bRemoveRow->setEnabled(!ui.lwRows->selectedItems().isEmpty());
	//    });

	//    connect(ui.lwColumns, &QListWidget::doubleClicked, this,&HypothesisTestDock::removeColumn);
	//    connect(ui.lwColumns, &QListWidget::itemSelectionChanged, this, [=]() {
	//        ui.bRemoveColumn->setEnabled(!ui.lwColumns->selectedItems().isEmpty());
	//    });

	connect(ui.cbTest, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::showTestType);
	connect(ui.cbTestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::showHypothesisTest);
	//    connect(ui.cbTest, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::showHypothesisTest);
	//    connect(ui.cbTestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::showHypothesisTest);
	connect(ui.pbPerformTest, &QPushButton::clicked, this, &HypothesisTestDock::doHypothesisTest);
	connect(ui.pbLeveneTest, &QPushButton::clicked, this, &HypothesisTestDock::performLeveneTest);


	//connecting null hypothesis and alternate hypothesis radio button
	connect(ui.rbH1OneTail1, &QRadioButton::toggled, this, &HypothesisTestDock::onRbH1OneTail1Toggled);
	connect(ui.rbH1OneTail2, &QRadioButton::toggled, this, &HypothesisTestDock::onRbH1OneTail2Toggled);
	connect(ui.rbH1TwoTail, &QRadioButton::toggled, this, &HypothesisTestDock::onRbH1TwoTailToggled);

	connect(ui.cbCol1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &HypothesisTestDock::col1IndexChanged);
	connect(ui.chbCalculateStats, &QCheckBox::stateChanged, this, &HypothesisTestDock::chbCalculateStatsStateChanged);
	connect(ui.chbCategorical, &QCheckBox::stateChanged, this, &HypothesisTestDock::changeCbCol2Label);

	connect(ui.chbPopulationSigma, &QCheckBox::stateChanged, this, &HypothesisTestDock::chbPopulationSigmaStateChanged);

	ui.cbTest->setCurrentIndex(0);
	emit ui.cbTest->currentIndexChanged(0);
	ui.cbTestType->setCurrentIndex(0);
	emit ui.cbTestType->currentIndexChanged(0);
}

void HypothesisTestDock::setHypothesisTest(HypothesisTest* HypothesisTest) {
	//QDEBUG("in set hypothesis test");
	m_initializing = true;
	m_hypothesisTest = HypothesisTest;

	m_aspectTreeModel = new AspectTreeModel(m_hypothesisTest->project());

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook,
				AspectType::Spreadsheet, AspectType::LiveDataSource};
	cbSpreadsheet->setTopLevelClasses(list);

	list = {AspectType::Spreadsheet, AspectType::LiveDataSource};
	m_aspectTreeModel->setSelectableAspects(list);

	cbSpreadsheet->setModel(m_aspectTreeModel);

	//show the properties
	ui.leName->setText(m_hypothesisTest->name());
	ui.teComment->setText(m_hypothesisTest->comment());
	setModelIndexFromAspect(cbSpreadsheet, m_hypothesisTest->dataSourceSpreadsheet());
	setColumnsComboBoxModel(m_hypothesisTest->dataSourceSpreadsheet());

	//setting rows and columns in combo box;

	//undo functions
	//	connect(m_hypothesisTest, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(hypothesisTestDescriptionChanged(const AbstractAspect*)));

	m_initializing = false;
}

void HypothesisTestDock::showTestType() {
	m_test = ui.cbTest->currentData().toInt();

	ui.cbTestType->clear();

	if (testType(m_test) == HypothesisTest::TTest ||
			testType(m_test) == HypothesisTest::ZTest) {
		ui.cbTestType->addItem( i18n("Two Sample Independent"), HypothesisTest::TwoSampleIndependent);
		ui.cbTestType->addItem( i18n("Two Sample Paired"), HypothesisTest::TwoSamplePaired);
		ui.cbTestType->addItem( i18n("One Sample"), HypothesisTest::OneSample);
	} else if (testType(m_test) == HypothesisTest::Anova) {
		ui.cbTestType->addItem( i18n("One Way"), HypothesisTest::OneWay);
		ui.cbTestType->addItem( i18n("Two Way"), HypothesisTest::TwoWay);
	}
}

void HypothesisTestDock::showHypothesisTest() {
	//QDEBUG("in showHypothesisTest");

	if (ui.cbTestType->count() == 0)
		return;

	m_test = testType(m_test) | ui.cbTestType->currentData().toInt();

	ui.lCalculateStats->show();
	ui.chbCalculateStats->show();

	ui.lEqualVariance->setVisible(m_test == (HypothesisTest::TTest | HypothesisTest::TwoSampleIndependent));
	ui.chbEqualVariance->setVisible(m_test == (HypothesisTest::TTest | HypothesisTest::TwoSampleIndependent));

	ui.lPopulationSigma->setVisible(m_test == (HypothesisTest::TTest | HypothesisTest::OneSample) ||
									m_test == (HypothesisTest::ZTest | HypothesisTest::OneSample));
	ui.chbPopulationSigma->setVisible(m_test == (HypothesisTest::TTest | HypothesisTest::OneSample) ||
									  m_test == (HypothesisTest::ZTest | HypothesisTest::OneSample));
	ui.chbPopulationSigma->setChecked(false);

	ui.pbLeveneTest->setVisible(m_test == (HypothesisTest::Anova | HypothesisTest::OneWay) ||
								m_test == (HypothesisTest::TTest | HypothesisTest::TwoSampleIndependent));

	ui.lH1->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH1OneTail1->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH1OneTail2->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH1TwoTail->setVisible(testType(m_test) != HypothesisTest::Anova);

	ui.lH0->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH0OneTail1->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH0OneTail2->setVisible(testType(m_test) != HypothesisTest::Anova);
	ui.rbH0TwoTail->setVisible(testType(m_test) != HypothesisTest::Anova);

	ui.rbH1TwoTail->setChecked(true);

	ui.lMuo->setVisible(testSubtype(m_test) == HypothesisTest::OneSample);
	ui.leMuo->setVisible(testSubtype(m_test) == HypothesisTest::OneSample);

	ui.lAlpha->show();
	ui.leAlpha->show();

	setColumnsComboBoxView();
	chbCalculateStatsStateChanged();

	ui.pbPerformTest->setEnabled(nonEmptySelectedColumns());
	ui.pbLeveneTest->setEnabled(nonEmptySelectedColumns());
}

void HypothesisTestDock::doHypothesisTest()  {
	//QDEBUG("in doHypothesisTest");
	m_hypothesisTest->setTail(m_tail);
	m_hypothesisTest->setPopulationMean(ui.leMuo->text());
	m_hypothesisTest->setSignificanceLevel(ui.leAlpha->text());

	QVector<Column*> cols;

	if (ui.cbCol1->count() == 0)
		return;

	cols << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());

	if (testSubtype(m_test) == HypothesisTest::TwoWay)
		cols << reinterpret_cast<Column*>(ui.cbCol3->currentData().toLongLong());

	if (testSubtype(m_test) != HypothesisTest::OneSample)
		if (ui.cbCol2->count() > 0)
			cols << reinterpret_cast<Column*>(ui.cbCol2->currentData().toLongLong());

	m_hypothesisTest->setColumns(cols);

	m_hypothesisTest->performTest(m_test,
								  ui.chbCategorical->isChecked(),
								  ui.chbEqualVariance->isChecked(),
								  ui.chbCalculateStats->isChecked());
}

void HypothesisTestDock::performLeveneTest()  {
	QVector<Column*> cols;

	if (ui.cbCol1->count() == 0 || ui.cbCol2->count() == 0)
		return;

	cols << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());
	cols << reinterpret_cast<Column*>(ui.cbCol2->currentData().toLongLong());
	m_hypothesisTest->setColumns(cols);

	m_hypothesisTest->setSignificanceLevel(ui.leAlpha->text());
	m_hypothesisTest->performLeveneTest(ui.chbCategorical->isChecked());
}

void HypothesisTestDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

////*************************************************************
////****** SLOTs for changes triggered in HypothesisTestDock *******
////*************************************************************
void HypothesisTestDock::spreadsheetChanged(const QModelIndex& index) {
	//QDEBUG("in spreadsheetChanged");
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	setColumnsComboBoxModel(spreadsheet);
	m_hypothesisTest->setDataSourceSpreadsheet(spreadsheet);
}

void HypothesisTestDock::changeCbCol2Label() {
	//QDEBUG("in changeCbCol2Label");
	if (testType(m_test) != HypothesisTest::Anova &&
			testSubtype(m_test) != HypothesisTest::TwoSampleIndependent) {
		ui.lCol2->setText( i18n("Independent Var. 2"));
		return;
	}

	if (ui.cbCol1->count() == 0) return;

	QString selected_text = ui.cbCol1->currentText();
	Column* col1 = m_hypothesisTest->dataSourceSpreadsheet()->column(selected_text);

	if (!ui.chbCategorical->isChecked()
		&& (col1->columnMode() == AbstractColumn::ColumnMode::Integer || col1->columnMode() == AbstractColumn::ColumnMode::Double)) {
		ui.lCol2->setText( i18n("Independent Var. 2"));
		ui.chbCategorical->setChecked(false);
		ui.chbCategorical->setEnabled(true);
	} else {
		ui.lCol2->setText( i18n("Dependent Var. 1"));
		if (!ui.chbCategorical->isChecked())
			ui.chbCategorical->setEnabled(false);
		else
			ui.chbCategorical->setEnabled(true);
		ui.chbCategorical->setChecked(true);
	}
}

void HypothesisTestDock::chbPopulationSigmaStateChanged() {
	if (ui.chbPopulationSigma->isVisible() && ui.chbPopulationSigma->isChecked())
		ui.lePopulationSigma->show();
	else
		ui.lePopulationSigma->hide();
}

void HypothesisTestDock::col1IndexChanged(int index) {
	if (index < 0) return;
	changeCbCol2Label();
}

void HypothesisTestDock::chbCalculateStatsStateChanged() {
	bool calculateStats = ui.chbCalculateStats->isChecked();

	ui.lVariables->setVisible(calculateStats);
	ui.lCol1->setVisible(calculateStats);
	ui.cbCol1->setVisible(calculateStats);

	ui.lCol2->setVisible(calculateStats && testSubtype(m_test) != HypothesisTest::OneSample);
	ui.cbCol2->setVisible(calculateStats && testSubtype(m_test) != HypothesisTest::OneSample);

	ui.lCol3->setVisible(calculateStats && m_test == (HypothesisTest::Anova | HypothesisTest::TwoWay));
	ui.cbCol3->setVisible(calculateStats && m_test == (HypothesisTest::Anova | HypothesisTest::TwoWay));

	ui.lCategorical->setVisible(calculateStats && m_test == (HypothesisTest::TTest | HypothesisTest::TwoSampleIndependent));
	ui.chbCategorical->setVisible(calculateStats && m_test == (HypothesisTest::TTest | HypothesisTest::TwoSampleIndependent));

	if (m_hypothesisTest != nullptr)
		m_hypothesisTest->initInputStatsTable(m_test, ui.chbCalculateStats->isChecked());
}

////*************************************************************
////******** SLOTs for changes triggered in Spreadsheet *********
////*************************************************************
void HypothesisTestDock::hypothesisTestDescriptionChanged(const AbstractAspect* aspect) {
	//QDEBUG("in hypothesisTestDescriptionChanged");

	if (m_hypothesisTest != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());

	m_initializing = false;
}

////*************************************************************
////******************** SETTINGS *******************************
////*************************************************************
//void HypothesisTestDock::load() {

//}

//void HypothesisTestDock::loadConfigFromTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    loads saved matrix properties from \c config.
// */
//void HypothesisTestDock::loadConfig(KConfig& config) {
//    Q_UNUSED(config);
//}

///*!
//    saves matrix properties to \c config.
// */
//void HypothesisTestDock::saveConfigAsTemplate(KConfig& config) {
//    Q_UNUSED(config);
//}

//TODO: Rather than inbuilt slots use own decided slots for checked rather than clicked

// for alternate hypothesis
// one_tail_1 is mu > mu0; one_tail_2 is mu < mu0; two_tail = mu != mu0;
void HypothesisTestDock::onRbH1OneTail1Toggled(bool checked) {
	if (!checked) return;
	ui.rbH0OneTail1->setChecked(true);
	m_tail = HypothesisTest::Positive;
}

void HypothesisTestDock::onRbH1OneTail2Toggled(bool checked) {
	if (!checked) return;
	ui.rbH0OneTail2->setChecked(true);
	m_tail = HypothesisTest::Negative;
}

void HypothesisTestDock::onRbH1TwoTailToggled(bool checked) {
	if (!checked) return;
	ui.rbH0TwoTail->setChecked(true);
	m_tail = HypothesisTest::Two;
}

/**************************************Helper Functions********************************************/
void HypothesisTestDock::countPartitions(Column *column, int &np, int &total_rows) {
	total_rows = column->rowCount();
	np = 0;
	QString cell_value;
	QMap<QString, bool> discovered_categorical_var;

	AbstractColumn::ColumnMode original_col_mode = column->columnMode();
	column->setColumnMode(AbstractColumn::ColumnMode::Text);

	for (int i = 0; i < total_rows; i++) {
		cell_value = column->textAt(i);

		if (cell_value.isEmpty()) {
			total_rows = i;
			break;
		}

		if (discovered_categorical_var[cell_value])
			continue;

		discovered_categorical_var[cell_value] = true;
		np++;
	}
	column->setColumnMode(original_col_mode);
}

void HypothesisTestDock::setColumnsComboBoxModel(Spreadsheet* spreadsheet) {
	m_onlyValuesCols.clear();
	m_twoCategoricalCols.clear();
	m_multiCategoricalCols.clear();

	for (auto* col : spreadsheet->children<Column>()) {
		if (col->columnMode() == AbstractColumn::ColumnMode::Integer || col->columnMode() == AbstractColumn::ColumnMode::Double)
			m_onlyValuesCols.append(col);
		else {
			int np = 0, n_rows = 0;
			countPartitions(col, np, n_rows);
			if (np <= 1)
				continue;
			else if (np == 2)
				m_twoCategoricalCols.append(col);
			else
				m_multiCategoricalCols.append(col);
		}
	}
	setColumnsComboBoxView();
	showHypothesisTest();
}


//TODO: change from if else to switch case:
void HypothesisTestDock::setColumnsComboBoxView() {

	ui.cbCol1->clear();
	ui.cbCol2->clear();
	ui.cbCol3->clear();

	QList<Column*>::iterator i;

	switch (testType(m_test)) {
	case (HypothesisTest::ZTest):
	case (HypothesisTest::TTest): {
		switch (testSubtype(m_test)) {
		case (HypothesisTest::TwoSampleIndependent): {
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			break;
		}
		case (HypothesisTest::TwoSamplePaired): {
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			break;
		}
		case (HypothesisTest::OneSample): {
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++)
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			break;
		}
		}
		break;
	}
	case HypothesisTest::Anova: {
		switch (testSubtype(m_test)) {
		case HypothesisTest::OneWay: {
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++)
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++)
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			break;
		}
		case HypothesisTest::TwoWay: {
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++)
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));

			for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol3->addItem( (*i)->name(), qint64(*i));
			}
			for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol3->addItem( (*i)->name(), qint64(*i));
			}
			break;
		}
		}
		break;
	}
	}
}

bool HypothesisTestDock::nonEmptySelectedColumns() {
	if ((ui.cbCol1->isVisible() && ui.cbCol1->count() < 1) ||
			(ui.cbCol2->isVisible() && ui.cbCol2->count() < 1) ||
			(ui.cbCol3->isVisible() && ui.cbCol3->count() < 1))
		return false;
	return true;
}

int HypothesisTestDock::testType(int test) {
	return test & 0x0F;
}

int HypothesisTestDock::testSubtype(int test) {
	return test & 0xF0;
}
