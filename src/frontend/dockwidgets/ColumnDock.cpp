/*
	File                 : ColumnDock.cpp
	Project              : LabPlot
	Description          : widget for column properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ColumnDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/AddValueLabelDialog.h"
#include "frontend/spreadsheet/BatchEditValueLabelsDialog.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/widgets/ConstantsWidget.h"
#include "frontend/widgets/FunctionsWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/GuiTools.h"

#include <KConfig>
#include <KConfigGroup>

#include <QMenu>
#include <QWidgetAction>

/*!
  \class ColumnDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup frontend
*/

ColumnDock::ColumnDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	// add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateTimeFormats())
		ui.cbDateTimeFormat->addItem(s, QVariant(s));

	ui.cbDateTimeFormat->setEditable(true);

	// plot designations
	ui.cbPlotDesignation->clear();
	for (int i = 0; i < ENUM_COUNT(AbstractColumn, PlotDesignation); i++)
		ui.cbPlotDesignation->addItem(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation(i), false));

	// labels
	ui.twLabels->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.twLabels->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	// formula
	ui.tbConstants->setIcon(QIcon::fromTheme(QStringLiteral("format-text-symbol")));
	ui.tbFunctions->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));
	ui.pbApplyFormula->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
	ui.bAddVariable->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::typeChanged);
	connect(ui.cbNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::numericFormatChanged);
	connect(ui.sbPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColumnDock::precisionChanged);
	connect(ui.cbDateTimeFormat, &QComboBox::currentTextChanged, this, &ColumnDock::dateTimeFormatChanged);
	connect(ui.cbPlotDesignation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::plotDesignationChanged);
	connect(ui.bAddLabel, &QPushButton::clicked, this, &ColumnDock::addLabel);
	connect(ui.bRemoveLabel, &QPushButton::clicked, this, &ColumnDock::removeLabel);
	connect(ui.bBatchEditLabels, &QPushButton::clicked, this, &ColumnDock::batchEditLabels);

	connect(ui.bAddVariable, &QPushButton::pressed, this, &ColumnDock::addVariable);
	connect(ui.teEquation, &ExpressionTextEdit::expressionChanged, this, &ColumnDock::checkValues);
	connect(ui.tbConstants, &QToolButton::clicked, this, &ColumnDock::showConstants);
	connect(ui.tbFunctions, &QToolButton::clicked, this, &ColumnDock::showFunctions);
	connect(ui.tbLoadFunction, &QPushButton::clicked, this, &ColumnDock::loadFunction);
	connect(ui.tbSaveFunction, &QPushButton::clicked, this, &ColumnDock::saveFunction);
	connect(ui.pbApplyFormula, &QPushButton::clicked, this, &ColumnDock::applyFormula);

	retranslateUi();
}

void ColumnDock::setColumns(QList<Column*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_columns = list;
	m_column = list.first();
	setAspects(list);

	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first column
	updateTypeWidgets(m_column->columnMode());
	ui.cbPlotDesignation->setCurrentIndex(int(m_column->plotDesignation()));

	// check whether we have non-editable columns:
	// 1. columns in read-only spreadsheets (LiveDataSource, Datapicker curve results, etc.)
	// 2. columns for residuals calculated in XYFitCurve (don't have Spreadsheet as the parent)
	bool nonEditable = false;
	for (auto* col : m_columns) {
		auto* s = dynamic_cast<Spreadsheet*>(col->parentAspect());
		if (s) {
			if (s->readOnly()) {
				nonEditable = true;
				break;
			}
		} else {
			nonEditable = true;
			break;
		}
	}

	// check if we need to hide widgets for value labels. no value labels for
	// * columns in statistics spreadsheets
	// * columns in the result spreadsheet in SeasonalDecomposition
	bool enableLabels = true;
	for (auto* col : m_columns) {
		auto* s = dynamic_cast<Spreadsheet*>(col->parentAspect());
		if (s) {
			if (s->type() == AspectType::StatisticsSpreadsheet || s->parentAspect()->type() == AspectType::SeasonalDecomposition) {
				enableLabels = false;
				break;
			}
		}
	}

	// if columns of different modes are selected, change of the mode is not possible
	bool sameMode = true;

	if (list.size() == 1) {
		// names and comments of non-editable columns in a file data source cannot be changed.
		if (!nonEditable && m_column->parentAspect()->type() == AspectType::LiveDataSource) {
			ui.leName->setEnabled(false);
			ui.teComment->setEnabled(false);
		}
	} else {
		auto mode = m_column->columnMode();
		for (auto* col : m_columns) {
			if (col->columnMode() != mode) {
				sameMode = false;
				break;
			}
		}
	}

	// type and formatting
	ui.lType->setEnabled(sameMode);
	ui.cbType->setEnabled(sameMode && !nonEditable); // don't allow to change the column type if there is at least one non-editable column
	ui.lNumericFormat->setEnabled(sameMode);
	ui.cbNumericFormat->setEnabled(sameMode);
	ui.lPrecision->setEnabled(sameMode);
	ui.sbPrecision->setEnabled(sameMode);
	ui.lDateTimeFormat->setEnabled(sameMode);
	ui.cbDateTimeFormat->setEnabled(sameMode);

	// value labels
	ui.twLabels->setEnabled(sameMode);
	ui.frameLabels->setEnabled(sameMode);
	ui.twLabels->setVisible(enableLabels);
	ui.frameLabels->setVisible(enableLabels);

	// show value labels of the first column if all selected columns have the same mode
	if (sameMode)
		showValueLabels();
	else {
		for (int i = 0; i < ui.twLabels->rowCount(); ++i)
			ui.twLabels->removeRow(0);
	}

	// formula, available only for columns in a spreadsheet
	m_spreadsheet = dynamic_cast<Spreadsheet*>(m_column->parentAspect());
	const bool isSpreadsheet = (m_spreadsheet != nullptr);
	ui.lFormula->setVisible(isSpreadsheet);
	if (isSpreadsheet)
		loadFormula();

	// slots
	connect(m_column, &AbstractColumn::modeChanged, this, &ColumnDock::columnModeChanged);
	connect(m_column->outputFilter(), &AbstractSimpleFilter::formatChanged, this, &ColumnDock::columnFormatChanged);
	connect(m_column->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &ColumnDock::columnPrecisionChanged);
	connect(m_column, &AbstractColumn::plotDesignationChanged, this, &ColumnDock::columnPlotDesignationChanged);
	connect(m_column, &Column::formulaChanged, this, &ColumnDock::loadFormula);
}

/*!
  depending on the currently selected column type (column mode) updates the widgets for the column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the type (column mode) is changed.
*/
void ColumnDock::updateTypeWidgets(AbstractColumn::ColumnMode mode) {
	ui.cbType->setCurrentIndex(ui.cbType->findData(static_cast<int>(mode)));
	switch (mode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
		ui.cbNumericFormat->setCurrentIndex(ui.cbNumericFormat->findData(filter->numericFormat()));
		ui.sbPrecision->setValue(filter->numDigits());
		break;
	}
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
		// 			DEBUG("	set column format: " << STDSTRING(filter->format()));
		const auto& format = filter->format();
		const int index = ui.cbDateTimeFormat->findData(format);
		if (index != -1)
			ui.cbDateTimeFormat->setCurrentIndex(index);
		else
			ui.cbDateTimeFormat->setCurrentText(format); // custom format, not in the list of pre-defined formats
		break;
	}
	case AbstractColumn::ColumnMode::Integer: // nothing to set
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		break;
	}

	// hide all the format related widgets first and
	// then show only what is required depending of the column mode(s)
	ui.lNumericFormat->hide();
	ui.cbNumericFormat->hide();
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
	ui.lDateTimeFormat->hide();
	ui.cbDateTimeFormat->hide();

	if (mode == AbstractColumn::ColumnMode::Double) {
		ui.lNumericFormat->show();
		ui.cbNumericFormat->show();
		ui.lPrecision->show();
		ui.sbPrecision->show();
	}

	if (mode == AbstractColumn::ColumnMode::DateTime) {
		ui.lDateTimeFormat->show();
		ui.cbDateTimeFormat->show();
	}
}

void ColumnDock::showValueLabels() {
	while (ui.twLabels->rowCount() > 0)
		ui.twLabels->removeRow(0);

	if (m_column->valueLabelsInitialized()) {
		auto mode = m_column->labelsMode();
		int i = 0;

		switch (mode) {
		case AbstractColumn::ColumnMode::Double: {
			const auto* labels = m_column->valueLabels();
			if (!labels)
				return;
			ui.twLabels->setRowCount(labels->size());
			auto it = labels->constBegin();
			while (it != labels->constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it->value)));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it->label));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			const auto* labels = m_column->intValueLabels();
			if (!labels)
				return;
			ui.twLabels->setRowCount(labels->size());
			auto it = labels->constBegin();
			while (it != labels->constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it->value)));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it->label));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			const auto* labels = m_column->bigIntValueLabels();
			if (!labels)
				return;
			ui.twLabels->setRowCount(labels->size());
			auto it = labels->constBegin();
			while (it != labels->constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it->value)));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it->label));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			const auto* labels = m_column->textValueLabels();
			if (!labels)
				return;
			ui.twLabels->setRowCount(labels->size());
			auto it = labels->constBegin();
			while (it != labels->constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(it->value));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it->label));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			const auto* labels = m_column->dateTimeValueLabels();
			if (!labels)
				return;
			ui.twLabels->setRowCount(labels->size());
			auto it = labels->constBegin();
			const QString& format = ui.cbDateTimeFormat->currentText();
			while (it != labels->constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(it->value.toString(format)));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it->label));
				++it;
				++i;
			}
			break;
		}
		}
	}
}

//*************************************************************
//******** SLOTs for changes triggered in ColumnDock **********
//*************************************************************
void ColumnDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// data type
	ui.cbType->clear();
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Double), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Double)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Integer), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Integer)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::BigInt), QVariant(static_cast<int>(AbstractColumn::ColumnMode::BigInt)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Text), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Text)));
	// TODO: activate month and day names once supported
	// ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Month), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Month)));
	// ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Day), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Day)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::DateTime),
					   QVariant(static_cast<int>(AbstractColumn::ColumnMode::DateTime)));

	// formats for numeric values
	ui.cbNumericFormat->clear();
	ui.cbNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	ui.lVariable->setText(m_variableLineEdits.size() > 1 ? i18n("Variables:") : i18n("Variable:"));

	// tooltip texts
	QString info = i18n(
		"Specifies how numeric values are formatted in the spreadsheet:"
		"<ul>"
		"<li>Decimal - format as [-]9.9</li>"
		"<li>Scientific (e) - format as [-]9.9e[+|-]999</li>"
		"<li>Scientific (E) - format as [-]9.9E[+|-]999</li>"
		"<li>Automatic (e) - selects between 'Decimal' and 'Scientific (e)' to get the most concise format</li>"
		"<li>Automatic (E) - selects between 'Decimal' and 'Scientific (E)' to get the most concise format</li>"
		"</ul>"
	);
	ui.lNumericFormat->setToolTip(info);
	ui.cbNumericFormat->setToolTip(info);

	info = i18n("For the  'Decimal', 'Scientific (e)', and 'Scientific (E)' formats, the precision represents the number of digits after the decimal point.\n"
		"For the 'Automatic (e)' and 'Automatic (E)' formats, the precision represents the maximum number of significant digits (trailing zeroes are omitted).");
	ui.lPrecision->setToolTip(info);
	ui.sbPrecision->setToolTip(info);

	ui.bAddLabel->setToolTip(i18n("Add a new value label"));
	ui.bRemoveLabel->setToolTip(i18n("Remove the selected value label"));
	ui.bBatchEditLabels->setToolTip(i18n("Modify multiple values labels in a batch mode"));

	ui.bAddVariable->setToolTip(i18n("Add new variable"));
	ui.chkFormulaAutoUpdate->setToolTip(i18n("Automatically update the calculated values in the target column on changes in the variable columns"));
	ui.chkFormulaAutoResize->setToolTip(i18n("Automatically resize the target column to fit the size of the variable columns"));
}

/*!
  called when the type (column mode - numeric, text etc.) of the column was changed.
*/
void ColumnDock::typeChanged(int index) {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_RETURN_NO_LOCK; // TODO: lock needed?

	auto columnMode = static_cast<AbstractColumn::ColumnMode>(ui.cbType->itemData(index).toInt());
	const auto& columns = m_columns;

	switch (columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		int digits = ui.sbPrecision->value();
		for (auto* col : columns) {
			col->beginMacro(i18n("%1: change column type", col->name()));
			col->setColumnMode(columnMode);
			auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());

			// TODO: using
			// char format = ui.cbFormat->itemData(ui.cbFormat->currentIndex()).toChar().toLatin1();
			// outside of the for-loop and
			// filter->setNumericFormat(format);
			// inside the loop leads to wrong results when converting from integer to numeric -> 'f' is set instead of 'e'
			filter->setNumericFormat(ui.cbNumericFormat->itemData(ui.cbNumericFormat->currentIndex()).toChar().toLatin1());
			filter->setNumDigits(digits);
			col->endMacro();
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		for (auto* col : columns)
			col->setColumnMode(columnMode);
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		for (auto* col : columns) {
			col->beginMacro(i18n("%1: change column type", col->name()));
			// the format is saved as item data
			const QString& format = ui.cbDateTimeFormat->itemData(ui.cbDateTimeFormat->currentIndex()).toString();
			col->setColumnMode(columnMode);
			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			col->endMacro();
		}
		break;
	case AbstractColumn::ColumnMode::DateTime: // -> DateTime
		for (auto* col : columns) {
			// use standard format
			const QString& format(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
			QDEBUG(Q_FUNC_INFO << ", format = " << format)

			// enable if unit cat be used
			/*if (col->isNumeric()) {	// ask how to interpret numeric input value

				QStringList items;
				for (int i = 0; i < ENUM_COUNT(AbstractColumn, TimeUnit); i++)
					items << AbstractColumn::timeUnitString((AbstractColumn::TimeUnit)i);
				QDEBUG("ITEMS: " << items)

				bool ok;
				QString item = QInputDialog::getItem(this, i18n("DateTime Filter"), i18n("Unit:"), items, 0, false, &ok);
				if (ok) {
					int index = items.indexOf(item);

					DEBUG("Selected index: " << index)
					//TODO: use index
				}
				else	// Cancel
					return;
			}*/

			col->beginMacro(i18n("%1: change column type", col->name()));
			col->setColumnMode(columnMode); // TODO: timeUnit
			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			col->endMacro();
		}
		break;
	}

	updateTypeWidgets(columnMode);
}

void ColumnDock::numericFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	char format = ui.cbNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* col : std::as_const(m_columns)) {
		auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());
		filter->setNumericFormat(format);
	}
}

void ColumnDock::precisionChanged(int digits) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* col : std::as_const(m_columns)) {
		auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());
		filter->setNumDigits(digits);
	}
}

void ColumnDock::dateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* col : std::as_const(m_columns)) {
		auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		filter->setFormat(format);
	}
}

void ColumnDock::plotDesignationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto pd = AbstractColumn::PlotDesignation(index);
	for (auto* col : std::as_const(m_columns))
		col->setPlotDesignation(pd);
}

//*************************************************************
//****************** Formula handling *************************
//*************************************************************
void ColumnDock::loadFormula() {
	// clean variable list
	for (int i = 0; i < m_variableLineEdits.size(); i++)
		deleteVariable();

	// formula expression
	ui.teEquation->setPlainText(m_column->formula());

	// variables
	const auto& formulaData = m_column->formulaData();
	if (formulaData.isEmpty()) {
		// A formula without any column variable is also possible, for example when just using the rownumber: "i / 1000"
		// This is a workaround, because right now there is no way to determine which variable is used in the formula and which not
		// it makes no sense to add variables if they are not used (preventing cyclic dependency) Gitlab #1037

		// no formula was used for this column -> add the first variable "x"
		// addVariable();
		// m_variableLineEdits[0]->setText(QStringLiteral("x"));
	} else { // formula and variables are available
		// add all available variables and select the corresponding columns
		const auto& cols = m_spreadsheet->project()->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
		for (int i = 0; i < formulaData.size(); ++i) {
			addVariable();
			m_variableLineEdits[i]->setText(formulaData.at(i).variableName());

			bool found = false;
			for (const auto* col : cols) {
				if (col != formulaData.at(i).column())
					continue;

				const auto* column = dynamic_cast<const AbstractColumn*>(col);
				m_variableDataColumns.at(i)->setAspect(column);
				m_variableDataColumns.at(i)->setInvalid(false);

				found = true;
				break;
			}

			// for the current variable name no column exists anymore (was deleted)
			//->highlight the combobox red
			if (!found) {
				m_variableDataColumns.at(i)->setAspect(nullptr);
				m_variableDataColumns.at(i)->setInvalid(
					true,
					i18n("The column \"%1\"\nis not available anymore. It will be automatically used once it is created again.",
						 formulaData.at(i).columnName()));
				m_variableDataColumns.at(i)->setText(formulaData.at(i).columnName().split(QLatin1Char('/')).last());
			}
		}
	}

	// auto update
	// enable if linking is turned on, so the user has to explicit disable recalculation, so it cannot be forgotten
	ui.chkFormulaAutoUpdate->setChecked(m_column->formulaAutoUpdate() || m_spreadsheet->linking());

	// auto-resize
	if (!m_spreadsheet->linking())
		ui.chkFormulaAutoResize->setChecked(m_column->formulaAutoResize());
	else {
		// linking is active, deactivate this option since the size of the target spreadsheet is controlled by the linked spreadsheet
		ui.chkFormulaAutoResize->setChecked(false);
		ui.chkFormulaAutoResize->setEnabled(false);
		ui.chkFormulaAutoResize->setToolTip(i18n("Spreadsheet linking is active. The size of the target spreadsheet is controlled by the linked spreadsheet."));
	}

	checkValues();
}

 // check if variable is already defined as constant or function
bool ColumnDock::validVariableName(QLineEdit* le) {
	bool isValid = false;
	if (ExpressionParser::getInstance()->constants().indexOf(le->text()) != -1) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a constant. Please use another name."));
	} else if (ExpressionParser::getInstance()->functions().indexOf(le->text()) != -1) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a function. Please use another name."));
	} else if (le->text().compare(QLatin1String("i")) == 0) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("The variable name 'i' is reserved for the index of the column row."));
	} else if (le->text().contains(QRegularExpression(QLatin1String("^[0-9]|[^a-zA-Z0-9_]")))) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name starts with a digit or contains special character."));
	} else {
		le->setStyleSheet(QString());
		le->setToolTip(QString());
		isValid = true;
	}
	return isValid;
}

void ColumnDock::checkValues() {
	if (ui.teEquation->toPlainText().simplified().isEmpty()) {
		ui.pbApplyFormula->setToolTip(i18n("Empty formula expression"));
		ui.pbApplyFormula->setEnabled(false);
		return;
	}

	// check whether the formula syntax is correct
	if (!ui.teEquation->isValid()) {
		ui.pbApplyFormula->setToolTip(i18n("Incorrect formula syntax: ") + ui.teEquation->errorMessage());
		ui.pbApplyFormula->setEnabled(false);
		return;
	}

	// check if expression uses variables
	if (ui.teEquation->expressionUsesVariables()) {
		// check the variables
		for (int i = 0; i < m_variableDataColumns.size(); ++i) {
			const auto& varName = m_variableLineEdits.at(i)->text();

			// ignore empty
			if (varName.isEmpty())
				continue;

			// check whether a valid column was provided for the variable
			auto* cb = m_variableDataColumns.at(i);
			auto* aspect = static_cast<AbstractAspect*>(cb->currentModelIndex().internalPointer());
			if (!aspect) {
				ui.pbApplyFormula->setToolTip(i18n("Select a valid column"));
				ui.pbApplyFormula->setEnabled(false);
				return;
			}

			// check whether the variable name is correct
			if (!validVariableName(m_variableLineEdits.at(i))) {
				ui.pbApplyFormula->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
				ui.pbApplyFormula->setEnabled(false);
				return;
			}
		}
	}

	ui.pbApplyFormula->setToolTip(i18n("Apply Formula"));
	ui.pbApplyFormula->setEnabled(true);
}

void ColumnDock::loadFunction() {
	auto fileName = GuiTools::loadFunction(ui.teEquation);
	if (fileName.isEmpty())
		return;

	// clean variable list
	for (int i = 0; i < m_variableLineEdits.size(); i++)
		deleteVariable();

	// load variables
	KConfig config(fileName);
	auto group = config.group(QLatin1String("Variables"));
	auto keys = group.keyList();
	int i = 0;
	for (const auto &name : keys) {
		addVariable();
		m_variableLineEdits[i]->setText(name);

		// restore path
		auto path = group.readEntry(name, {});
		QDEBUG(Q_FUNC_INFO << ", variable" << name << ":" << path)
		auto index = aspectModel()->modelIndexForPath(path);
		m_variableDataColumns[i++]->setCurrentModelIndex(index);
	}
}

void ColumnDock::saveFunction() {
	auto fileName = GuiTools::saveFunction(ui.teEquation);
	if (fileName.isEmpty())
		return;

	// save variables
	KConfig config(fileName);
	auto group = config.group(QLatin1String("Variables"));
	int i = 0;
	for (auto* varName : m_variableLineEdits) {
		QString name = varName->text().simplified();
		auto index = m_variableDataColumns.at(i++)->currentModelIndex();
		QString path = aspectModel()->path(index);
		group.writeEntry(name, path);
	}
}

void ColumnDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);
	connect(&constants, &ConstantsWidget::constantSelected, this, &ColumnDock::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction{new QWidgetAction(this)};
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + ui.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(ui.tbConstants->mapToGlobal(pos));
}

void ColumnDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &ColumnDock::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction{new QWidgetAction(this)};
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + ui.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(ui.tbFunctions->mapToGlobal(pos));
}

void ColumnDock::insertFunction(const QString& functionName) const {
	ui.teEquation->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Cartesian));
}

void ColumnDock::insertConstant(const QString& constantsName) const {
	ui.teEquation->insertPlainText(constantsName);
}

void ColumnDock::addVariable() {
	auto* layout{ui.gridLayoutVariables};
	auto row{m_variableLineEdits.size()};

	// text field for the variable name
	auto* le{new QLineEdit};
	le->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
	auto* validator = new QRegularExpressionValidator(QRegularExpression(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*")), le);
	le->setValidator(validator);
	le->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
	connect(le, &QLineEdit::textChanged, this, &ColumnDock::variableNameChanged);
	layout->addWidget(le, row, 0, 1, 1);
	m_variableLineEdits << le;
	auto* l{new QLabel(QStringLiteral("="))};
	layout->addWidget(l, row, 1, 1, 1);
	m_variableLabels << l;

	// combo box for the data column
	auto* cb{new TreeViewComboBox()};
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &ColumnDock::variableColumnChanged);
	layout->addWidget(cb, row, 2, 1, 1);
	m_variableDataColumns << cb;

	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Column});
	model->enableNumericColumnsOnly(true);

	cb->setModel(model);

	// don't allow to select columns to be calculated as variable columns (avoid circular dependencies)
	QList<const AbstractAspect*> aspects;
	for (auto* col : m_columns)
		aspects << col;
	cb->setHiddenAspects(aspects);

	// for the variable column select the first non-selected column in the spreadsheet
	for (auto* col : m_spreadsheet->children<Column>()) {
		if (m_columns.indexOf(col) == -1) {
			cb->setAspect(col);
			break;
		}
	}

	// move the add-button to the next row
	layout->removeWidget(ui.bAddVariable);
	layout->addWidget(ui.bAddVariable, row + 1, 3, 1, 1);

	// add delete-button for the just added variable
	if (row != 0) {
		auto* b = new QToolButton();
		b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		b->setToolTip(i18n("Delete variable"));
		layout->addWidget(b, row, 3, 1, 1);
		m_variableDeleteButtons << b;
		connect(b, &QToolButton::pressed, this, &ColumnDock::deleteVariable);
	}

	ui.lVariable->setText(i18n("Variables:"));
	// TODO: adjust the tab-ordering after new widgets were added
}

void ColumnDock::deleteVariable() {
	auto* ob = qobject_cast<QToolButton*>(QObject::sender());
	if (ob) {
		const auto index = m_variableDeleteButtons.indexOf(ob);

		delete m_variableLineEdits.takeAt(index + 1);
		delete m_variableLabels.takeAt(index + 1);
		delete m_variableDataColumns.takeAt(index + 1);
		delete m_variableDeleteButtons.takeAt(index);
	} else {
		if (!m_variableLineEdits.isEmpty())
			delete m_variableLineEdits.takeLast();
		if (!m_variableLabels.isEmpty())
			delete m_variableLabels.takeLast();
		if (!m_variableDataColumns.isEmpty())
			delete m_variableDataColumns.takeLast();
		if (!m_variableDeleteButtons.isEmpty())
			delete m_variableDeleteButtons.takeLast();
	}

	variableNameChanged();
	checkValues();

	ui.lVariable->setText(m_variableLineEdits.size() > 1 ? i18n("Variables:") : i18n("Variable:"));
	// TODO: adjust the tab-ordering after some widgets were deleted
}

void ColumnDock::variableNameChanged() {
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

	ui.lFunction->setText(funText);
	ui.teEquation->setVariables(vars);
	checkValues();
}

void ColumnDock::variableColumnChanged(const QModelIndex& index) {
	// combobox was potentially red-highlighted because of a missing column
	// remove the highlighting when we have a valid selection now
	auto* aspect{static_cast<AbstractAspect*>(index.internalPointer())};
	if (aspect) {
		auto* cb{dynamic_cast<TreeViewComboBox*>(QObject::sender())};
		if (cb)
			cb->setStyleSheet(QString());
	}

	checkValues();
}

void ColumnDock::applyFormula() {
	WAIT_CURSOR_AUTO_RESET;
	m_spreadsheet->beginMacro(i18np("%1: apply column formula", "%1: apply column formula", m_spreadsheet->name(), m_columns.size()));

	// determine variable names and data vectors of the specified columns
	QStringList variableNames;
	QVector<Column*> variableColumns;
	for (int i = 0; i < m_variableLineEdits.size(); ++i) {
		variableNames << m_variableLineEdits.at(i)->text().simplified();

		auto* aspect{static_cast<AbstractAspect*>(m_variableDataColumns.at(i)->currentModelIndex().internalPointer())};
		if (aspect) {
			auto* column{dynamic_cast<Column*>(aspect)};
			if (column)
				variableColumns << column;
		}
	}

	// set the new values and store the expression, variable names and used data columns
	const QString& expression{ui.teEquation->toPlainText()};
	bool autoUpdate{(ui.chkFormulaAutoUpdate->checkState() == Qt::Checked)};
	bool autoResize{(ui.chkFormulaAutoResize->checkState() == Qt::Checked)};
	for (auto* col : m_columns) {
		col->setColumnMode(AbstractColumn::ColumnMode::Double);
		col->setFormula(expression, variableNames, variableColumns, autoUpdate, autoResize);
		col->updateFormula();
	}
	m_spreadsheet->endMacro();
}

//*************************************************************
//*************** Value labels handling ***********************
//*************************************************************
void ColumnDock::addLabel() {
	m_column->setLabelsMode(m_column->columnMode());

	const auto mode = m_column->columnMode();
	AddValueLabelDialog dlg(this, m_column);

	if (mode == AbstractColumn::ColumnMode::Month || mode == AbstractColumn::ColumnMode::Day || mode == AbstractColumn::ColumnMode::DateTime)
		dlg.setDateTimeFormat(ui.cbDateTimeFormat->currentText());

	if (dlg.exec() == QDialog::Accepted) {
		const QString& label = dlg.label();
		const auto& columns = m_columns;
		QString valueStr;
		switch (mode) {
		case AbstractColumn::ColumnMode::Double: {
			double value = dlg.value();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			int value = dlg.valueInt();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			qint64 value = dlg.valueBigInt();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			valueStr = dlg.valueText();
			for (auto* col : columns)
				col->addValueLabel(valueStr, label);
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			const QDateTime& value = dlg.valueDateTime();
			valueStr = value.toString(ui.cbDateTimeFormat->currentText());
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		}
	}

	// reload all, because due to the migration the view
	// might be changed
	showValueLabels();
	m_column->setProjectChanged(true);
}

void ColumnDock::removeLabel() {
	auto* item = ui.twLabels->currentItem();
	if (!item)
		return;

	int row = ui.twLabels->currentRow();
	const auto& value = ui.twLabels->itemAt(row, 0)->text();
	for (auto* col : std::as_const(m_columns))
		col->removeValueLabel(value);

	ui.twLabels->removeRow(ui.twLabels->currentRow());
	m_column->setProjectChanged(true);
}

void ColumnDock::batchEditLabels() {
	auto* dlg = new BatchEditValueLabelsDialog(this);
	dlg->setColumns(m_columns);
	if (dlg->exec() == QDialog::Accepted)
		showValueLabels(); // new value labels were saved into the columns in the dialog, show them here

	delete dlg;
	m_column->setProjectChanged(true);
}

//*************************************************************
//********* SLOTs for changes triggered in Column *************
//*************************************************************
void ColumnDock::columnModeChanged(const AbstractAspect* aspect) {
	CONDITIONAL_LOCK_RETURN;
	if (m_column != aspect)
		return;

	updateTypeWidgets(m_column->columnMode());
	showValueLabels(); // Update value labels shown in the list, because due to the change they might be migrated
}

void ColumnDock::columnFormatChanged() {
	CONDITIONAL_LOCK_RETURN;
	auto columnMode = m_column->columnMode();
	switch (columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
		const int index = ui.cbNumericFormat->findData(filter->numericFormat());
		ui.cbNumericFormat->setCurrentIndex(index);
		break;
	}
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
		ui.cbDateTimeFormat->setCurrentText(filter->format());
		break;
	}
	}
}

void ColumnDock::columnPrecisionChanged() {
	CONDITIONAL_LOCK_RETURN;
	auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
	ui.sbPrecision->setValue(filter->numDigits());
}

void ColumnDock::columnPlotDesignationChanged(const AbstractColumn* col) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPlotDesignation->setCurrentIndex(int(col->plotDesignation()));
}
