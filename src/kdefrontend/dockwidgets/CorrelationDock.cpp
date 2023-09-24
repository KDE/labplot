/*
	File                 : CorrelationDock.cpp
	Project              : LabPlot
	Description          : Dock for Correlation Coefficients/Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CorrelationDock.h"
#include "backend/statistics/Correlation.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>

/*!
  \class CorrelationDock
  \brief Provides a dock (widget) for correlation testing:
  \ingroup kdefrontend
*/
CorrelationDock::CorrelationDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	cbSpreadsheet = new TreeViewComboBox;
	ui.gridLayout->addWidget(cbSpreadsheet, 4, 1, 1, 3);

	ui.cbMethod->addItem(i18n("Pearson r"), Correlation::Pearson);
	ui.cbMethod->addItem(i18n("Kendall"), Correlation::Kendall);
	ui.cbMethod->addItem(i18n("Spearman"), Correlation::Spearman);
	ui.cbMethod->addItem(i18n("Chi Square"), Correlation::ChiSquare);

	ui.leNRows->setText(QLatin1String("2"));
	ui.leNColumns->setText(QLatin1String("2"));

	ui.leNRows->setValidator(new QIntValidator(this));
	ui.leNColumns->setValidator(new QIntValidator(this));

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
	ui.pbPerformTest->setIcon(QIcon::fromTheme(QLatin1String("run-build")));

	connect(cbSpreadsheet, &TreeViewComboBox::currentModelIndexChanged, this, &CorrelationDock::spreadsheetChanged);
	connect(ui.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CorrelationDock::methodChanged);
	connect(ui.chbCategorical, &QCheckBox::stateChanged, this, &CorrelationDock::changeCbCol2Label);
	connect(ui.chbCalculateStats, &QCheckBox::stateChanged, this, &CorrelationDock::chbColumnStatsStateChanged);
	connect(ui.leNRows, &QLineEdit::textChanged, this, &CorrelationDock::leNRowsChanged);
	connect(ui.leNColumns, &QLineEdit::textChanged, this, &CorrelationDock::leNColumnsChanged);
	connect(ui.pbExportToSpreadsheet, &QPushButton::clicked, this, &CorrelationDock::exportStatsTableToSpreadsheet);
	connect(ui.pbPerformTest, &QPushButton::clicked, this, &CorrelationDock::recalculate);
	connect(ui.cbCol1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CorrelationDock::col1IndexChanged);

}

void CorrelationDock::setCorrelation(Correlation* coefficient) {
	Lock lock(m_initializing);
	m_correlation = coefficient;
	//setAspects(list);

	//show all available spreadsheets in the combo box
	m_aspectTreeModel = new AspectTreeModel(m_correlation->project());

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook,
	                       AspectType::Spreadsheet, AspectType::LiveDataSource};
	cbSpreadsheet->setTopLevelClasses(list);

	list = {AspectType::Spreadsheet, AspectType::LiveDataSource};
	m_aspectTreeModel->setSelectableAspects(list);

	cbSpreadsheet->setModel(m_aspectTreeModel);

	//show the properties of the correlation
	ui.leName->setText(m_correlation->name());
	ui.teComment->setText(m_correlation->comment());
	setModelIndexFromAspect(cbSpreadsheet, m_correlation->dataSourceSpreadsheet());
	setColumnsComboBoxModel(m_correlation->dataSourceSpreadsheet());

	methodChanged();

	connect(m_correlation, &Correlation::aspectDescriptionChanged, this, &CorrelationDock::aspectDescriptionChanged);
}

////*************************************************************
////****** SLOTs for changes triggered in CorrelationDock *******
////*************************************************************
void CorrelationDock::spreadsheetChanged(const QModelIndex& index) {
	//QDEBUG("in spreadsheetChanged");
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	setColumnsComboBoxModel(spreadsheet);
	m_correlation->setDataSourceSpreadsheet(spreadsheet);
}

void CorrelationDock::col1IndexChanged(int index) {
	if (index < 0) return;
	changeCbCol2Label();
}

void CorrelationDock::methodChanged() {
	const auto method = static_cast<Correlation::Method>(ui.cbMethod->currentData().toInt());

	ui.lCalculateStats->setVisible(method == Correlation::ChiSquare);
	ui.chbCalculateStats->setVisible(method == Correlation::ChiSquare);

	if (method != Correlation::ChiSquare)
		ui.chbCalculateStats->setChecked(true);

	chbColumnStatsStateChanged();
}

void CorrelationDock::recalculate()  {
	if (ui.chbCategorical->isChecked() && ui.cbCol1->count() == 0)
		return;

	QVector<Column*> cols;
	cols << reinterpret_cast<Column*>(ui.cbCol1->currentData().toLongLong());
	cols << reinterpret_cast<Column*>(ui.cbCol2->currentData().toLongLong());

	auto method = static_cast<Correlation::Method>(ui.cbMethod->currentData().toInt());

	if (method == Correlation::ChiSquare)
		cols << reinterpret_cast<Column*>(ui.cbCol3->currentData().toLongLong());

	m_correlation->setColumns(cols);
	m_correlation->performTest(method, ui.chbCategorical->isChecked(), ui.chbCalculateStats->isChecked());
}

void CorrelationDock::setModelIndexFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (aspect)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(aspect));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

void CorrelationDock::changeCbCol2Label() {
	if (ui.cbCol1->count() == 0) return;

	QString selected_text = ui.cbCol1->currentText();
	Column* col1 = m_correlation->dataSourceSpreadsheet()->column(selected_text);

	auto method = static_cast<Correlation::Method>(ui.cbMethod->currentData().toInt());

	if (method == (Correlation::Kendall | Correlation::Spearman) ||
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

void CorrelationDock::chbColumnStatsStateChanged() {
	bool chbChecked = ui.chbCalculateStats->isChecked();
	auto method = static_cast<Correlation::Method>(ui.cbMethod->currentData().toInt());

	ui.lVariables->setVisible(chbChecked);
	ui.lCol1->setVisible(chbChecked);
	ui.cbCol1->setVisible(chbChecked);
	ui.lCol2->setVisible(chbChecked);
	ui.cbCol2->setVisible(chbChecked);
	ui.lCol3->setVisible(chbChecked && method == Correlation::ChiSquare);
	ui.cbCol3->setVisible(chbChecked && method == Correlation::ChiSquare);

	ui.lCategorical->setVisible(chbChecked && method == Correlation::Pearson);
	ui.chbCategorical->setVisible(chbChecked && method == Correlation::Pearson);

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

	if (m_correlation != nullptr)
		m_correlation->initInputStatsTable(method, chbChecked, ui.leNRows->text().toInt(), ui.leNColumns->text().toInt());
}

void CorrelationDock::leNRowsChanged() {
	if (m_correlation != nullptr)
		m_correlation->setInputStatsTableNRows(ui.leNRows->text().toInt());
}

void CorrelationDock::leNColumnsChanged() {
	if (m_correlation != nullptr)
		m_correlation->setInputStatsTableNCols(ui.leNColumns->text().toInt());
}

void CorrelationDock::exportStatsTableToSpreadsheet() {
	if (ui.chbCalculateStats->isVisible() && !ui.chbCalculateStats->isChecked())
		m_correlation->exportStatTableToSpreadsheet();
}

void CorrelationDock::setColumnsComboBoxModel(Spreadsheet* spreadsheet) {
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
	methodChanged();
}

void CorrelationDock::setColumnsComboBoxView() {
	ui.cbCol1->clear();
	ui.cbCol2->clear();
	ui.cbCol3->clear();

	QList<Column*>::iterator i;
	const auto method = static_cast<Correlation::Method>(ui.cbMethod->currentData().toInt());
	switch (method) {
	case (Correlation::Pearson): {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case Correlation::Kendall: {
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
	case Correlation::Spearman: {
		for (i = m_onlyValuesCols.begin(); i != m_onlyValuesCols.end(); i++) {
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
			ui.cbCol2->addItem( (*i)->name(), qint64(*i));
		}
		for (i = m_twoCategoricalCols.begin(); i != m_twoCategoricalCols.end(); i++)
			ui.cbCol1->addItem( (*i)->name(), qint64(*i));
		break;
	}
	case Correlation::ChiSquare: {
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
		break;
	}
	}
}

bool CorrelationDock::nonEmptySelectedColumns() {
	if ((ui.cbCol1->isVisible() && ui.cbCol1->count() < 1) ||
			(ui.cbCol2->isVisible() && ui.cbCol2->count() < 1))
		return false;
	return true;
}

void CorrelationDock::countPartitions(Column *column, int &np, int &total_rows) {
	total_rows = column->rowCount();
	np = 0;
	QString cell_value;
	QMap<QString, bool> discovered_categorical_var;

	auto original_col_mode = column->columnMode();
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
