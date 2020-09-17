/***************************************************************************
    File                 : CorrelationCoefficientDock.cpp
    Project              : LabPlot
    Description          : widget for correlation test properties
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)

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

#include "CorrelationCoefficientDock.h"
#include "backend/generalTest/CorrelationCoefficient.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>

/*!
  \class CorrelationCoefficientDock
  \brief Provides a dock (widget) for correlation testing:
  \ingroup kdefrontend
*/
CorrelationCoefficientDock::CorrelationCoefficientDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout->addWidget(cbSpreadsheet, 4, 3, 1, 3);

	ui.cbTest->addItem( i18n("Pearson r"), CorrelationCoefficient::Pearson);
	ui.cbTest->addItem( i18n("Kendall"), CorrelationCoefficient::Kendall);
	ui.cbTest->addItem( i18n("Spearman"), CorrelationCoefficient::Spearman);
	ui.cbTest->addItem( i18n("Chi Square"), CorrelationCoefficient::ChiSquare);

	ui.leNRows->setText("2");
	ui.leNColumns->setText("2");

	ui.leNRows->setValidator(new QIntValidator(this));
	ui.leNColumns->setValidator(new QIntValidator(this));

	ui.lTestType->hide();
	ui.cbTestType->hide();
	ui.lCalculateStats->hide();
	ui.chbCalculateStats->hide();
	ui.lNRows->hide();
	ui.leNRows->hide();
	ui.lNColumns->hide();
	ui.leNColumns->hide();
	ui.lCategorical->hide();
	ui.chbCategorical->hide();
	ui.lCol1->hide();
	ui.cbCol1->hide();
	ui.lCol2->hide();
	ui.cbCol2->hide();
	ui.lCol3->hide();
	ui.cbCol3->hide();
	ui.lAlpha->hide();
	ui.leAlpha->hide();
	ui.pbPerformTest->setEnabled(false);
	ui.pbPerformTest->setIcon(QIcon::fromTheme("run-build"));

	connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &CorrelationCoefficientDock::spreadsheetChanged);
	connect(ui.cbTest, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::showTestType);
	connect(ui.cbTestType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::showCorrelationCoefficient);
	connect(ui.chbCategorical, &QCheckBox::stateChanged, this, &CorrelationCoefficientDock::changeCbCol2Label);
	connect(ui.chbCalculateStats, &QCheckBox::stateChanged, this, &CorrelationCoefficientDock::chbColumnStatsStateChanged);
	connect(ui.leNRows, &QLineEdit::textChanged, this, &CorrelationCoefficientDock::leNRowsChanged);
	connect(ui.leNColumns, &QLineEdit::textChanged, this, &CorrelationCoefficientDock::leNColumnsChanged);
	connect(ui.pbExportToSpreadsheet, &QPushButton::clicked, this, &CorrelationCoefficientDock::exportStatsTableToSpreadsheet);
	connect(ui.pbPerformTest, &QPushButton::clicked, this, &CorrelationCoefficientDock::findCorrelationCoefficient);
	connect(ui.cbCol1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CorrelationCoefficientDock::col1IndexChanged);

	showTestType();
	showCorrelationCoefficient();
}

void CorrelationCoefficientDock::setCorrelationCoefficient(CorrelationCoefficient* CorrelationCoefficient) {
	Lock lock(m_initializing);
	m_correlationCoefficient = CorrelationCoefficient;
	m_aspect = m_correlationCoefficient;

	//show all available spreadsheets in the combo box
	m_aspectTreeModel = new AspectTreeModel(m_correlationCoefficient->project());

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook,
	                       AspectType::Spreadsheet, AspectType::LiveDataSource};
	cbSpreadsheet->setTopLevelClasses(list);

	list = {AspectType::Spreadsheet, AspectType::LiveDataSource};
	m_aspectTreeModel->setSelectableAspects(list);

	cbSpreadsheet->setModel(m_aspectTreeModel);

	//show the properties of the correlation
	ui.leName->setText(m_correlationCoefficient->name());
	ui.leComment->setText(m_correlationCoefficient->comment());
	setModelIndexFromAspect(cbSpreadsheet, m_correlationCoefficient->dataSourceSpreadsheet());
	setColumnsComboBoxModel(m_correlationCoefficient->dataSourceSpreadsheet());

	connect(m_correlationCoefficient, &CorrelationCoefficient::aspectDescriptionChanged, this, &CorrelationCoefficientDock::correlationCoefficientDescriptionChanged);
}

void CorrelationCoefficientDock::showTestType() {
	if (ui.cbTest->count() == 0)
		return;

	m_test = ui.cbTest->currentData().toInt();

	ui.cbTestType->clear();
	switch (m_test) {
	case CorrelationCoefficient::ChiSquare:
		ui.lTestType->show();
		ui.cbTestType->show();
		ui.cbTestType->addItem( i18n("Test for Independence"), CorrelationCoefficient::IndependenceTest);
		break;
	case CorrelationCoefficient::Pearson:
	case CorrelationCoefficient::Spearman:
	case CorrelationCoefficient::Kendall:
	case CorrelationCoefficient::IndependenceTest:
		ui.lTestType->hide();
		ui.cbTestType->hide();
		showCorrelationCoefficient();
		break;
	}
}

void CorrelationCoefficientDock::showCorrelationCoefficient() {
	m_test = testType(m_test) | ui.cbTestType->currentData().toInt();

	ui.lCalculateStats->setVisible(testType(m_test) == CorrelationCoefficient::ChiSquare);
	ui.chbCalculateStats->setVisible(testType(m_test) == CorrelationCoefficient::ChiSquare);

	if (testType(m_test) != CorrelationCoefficient::ChiSquare)
		ui.chbCalculateStats->setChecked(true);

	chbColumnStatsStateChanged();
}

void CorrelationCoefficientDock::findCorrelationCoefficient()  {
	QVector<Column*> cols;

	if (ui.chbCategorical->isChecked() && ui.cbCol1->count() == 0)
		return;

	cols << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());
	cols << reinterpret_cast<Column*>(ui.cbCol2->currentData().toLongLong());

	if (testSubType(m_test) == CorrelationCoefficient::IndependenceTest)
		cols << reinterpret_cast<Column*>(ui.cbCol3->currentData().toLongLong());

	m_correlationCoefficient->setColumns(cols);
	m_correlationCoefficient->performTest(m_test, ui.chbCategorical->isChecked(), ui.chbCalculateStats->isChecked());
}

void CorrelationCoefficientDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

////*************************************************************
////****** SLOTs for changes triggered in CorrelationCoefficientDock *******
////*************************************************************
void CorrelationCoefficientDock::spreadsheetChanged(const QModelIndex& index) {
	//QDEBUG("in spreadsheetChanged");
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	setColumnsComboBoxModel(spreadsheet);
	m_correlationCoefficient->setDataSourceSpreadsheet(spreadsheet);
}

void CorrelationCoefficientDock::col1IndexChanged(int index) {
	if (index < 0) return;
	changeCbCol2Label();
}

////*************************************************************
////******** SLOTs for changes triggered in Spreadsheet *********
////*************************************************************
void CorrelationCoefficientDock::correlationCoefficientDescriptionChanged(const AbstractAspect* aspect) {
	if (m_correlationCoefficient != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

void CorrelationCoefficientDock::changeCbCol2Label() {
	if (ui.cbCol1->count() == 0) return;

	QString selected_text = ui.cbCol1->currentText();
	Column* col1 = m_correlationCoefficient->dataSourceSpreadsheet()->column(selected_text);

	if (m_test == (CorrelationCoefficient::Kendall | CorrelationCoefficient::Spearman) ||
			(!ui.chbCategorical->isChecked() && col1->isNumeric())) {
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

void CorrelationCoefficientDock::chbColumnStatsStateChanged() {
	bool chbChecked = ui.chbCalculateStats->isChecked();

	ui.lVariables->setVisible(chbChecked);
	ui.lCol1->setVisible(chbChecked);
	ui.cbCol1->setVisible(chbChecked);
	ui.lCol2->setVisible(chbChecked);
	ui.cbCol2->setVisible(chbChecked);
	ui.lCol3->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::ChiSquare);
	ui.cbCol3->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::ChiSquare);

	ui.lCategorical->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::Pearson);
	ui.chbCategorical->setVisible(chbChecked && testType(m_test) == CorrelationCoefficient::Pearson);

	ui.lNRows->setVisible(!chbChecked);
	ui.leNRows->setVisible(!chbChecked);

	ui.lNColumns->setVisible(!chbChecked);
	ui.leNColumns->setVisible(!chbChecked);
	ui.pbExportToSpreadsheet->setVisible(!chbChecked);

	if (chbChecked) {
		setColumnsComboBoxView();
		ui.pbPerformTest->setEnabled(nonEmptySelectedColumns());
	} else
		ui.pbPerformTest->setEnabled(true);

	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->initInputStatsTable(m_test, chbChecked, ui.leNRows->text().toInt(), ui.leNColumns->text().toInt());
}

void CorrelationCoefficientDock::leNRowsChanged() {
	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->setInputStatsTableNRows(ui.leNRows->text().toInt());
}

void CorrelationCoefficientDock::leNColumnsChanged() {
	if (m_correlationCoefficient != nullptr)
		m_correlationCoefficient->setInputStatsTableNCols(ui.leNColumns->text().toInt());
}

void CorrelationCoefficientDock::exportStatsTableToSpreadsheet() {
	if (ui.chbCalculateStats->isVisible() && !ui.chbCalculateStats->isChecked())
		m_correlationCoefficient->exportStatTableToSpreadsheet();
}

void CorrelationCoefficientDock::setColumnsComboBoxModel(Spreadsheet* spreadsheet) {
	m_onlyValuesCols.clear();
	m_twoCategoricalCols.clear();
	m_multiCategoricalCols.clear();

	for (auto* col : spreadsheet->children<Column>()) {
		if (col->isNumeric())
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
	showCorrelationCoefficient();
}

void CorrelationCoefficientDock::setColumnsComboBoxView() {
	ui.cbCol1->clear();
	ui.cbCol2->clear();
	ui.cbCol3->clear();

	QList<Column*>::iterator i;

	switch (testType(m_test)) {
	case (CorrelationCoefficient::Pearson): {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case CorrelationCoefficient::Kendall: {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		break;
	}
	case CorrelationCoefficient::Spearman: {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case CorrelationCoefficient::ChiSquare: {
		if (testSubType(m_test) == CorrelationCoefficient::IndependenceTest) {
			for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			for (i = m_multiCategoricalCols.begin(); i != m_multiCategoricalCols.end(); i++) {
				ui.cbCol1->addItem( (*i)->name(), qint64(*i));
				ui.cbCol2->addItem( (*i)->name(), qint64(*i));
			}
			for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
				ui.cbCol3->addItem( (*i)->name(), qint64(*i));
			}
		}
		break;
	}
	}
}

bool CorrelationCoefficientDock::nonEmptySelectedColumns() {
	if ((ui.cbCol1->isVisible() && ui.cbCol1->count() < 1) ||
			(ui.cbCol2->isVisible() && ui.cbCol2->count() < 1))
		return false;
	return true;
}

int CorrelationCoefficientDock::testType(int test) {
	return test & 0x0F;
}

int CorrelationCoefficientDock::testSubType(int test) {
	return test & 0xF0;
}

void CorrelationCoefficientDock::countPartitions(Column *column, int &np, int &total_rows) {
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
