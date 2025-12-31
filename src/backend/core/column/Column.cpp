/*
	File                 : Column.cpp
	Project              : LabPlot
	Description          : Aspect that manages a column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2013-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/column/Column.h"
#ifndef SDK
#include "backend/notebook/Notebook.h"
#endif
#include "backend/core/Project.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/column/columncommands.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Plot.h"
#include "frontend/spreadsheet/SpreadsheetView.h"

#include <QActionGroup>
#include <QClipboard>
#include <QIcon>
#include <QMenu>
#include <QMimeData>
#include <QThreadPool>

/**
 * \class Column
 * \brief Aspect that manages a column
 *
 * This class represents a column, i.e., (mathematically) a 1D vector of
 * values with a header. It provides a public reading and (undo aware) writing
 * interface as defined in AbstractColumn. A column
 * can have one of currently three data types: double, QString, or
 * QDateTime. The string representation of the values can differ depending
 * on the mode of the column.
 *
 * Column inherits from AbstractAspect and is intended to be a child
 * of the corresponding Spreadsheet in the aspect hierarchy. Columns don't
 * have a view as they are intended to be displayed inside a spreadsheet.
 */

Column::Column(const QString& name, ColumnMode mode)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, mode)) {
	init();
}

Column::Column(const QString& name, const QVector<double>& data)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, ColumnMode::Double, new QVector<double>(data))) {
	init();
}

Column::Column(const QString& name, const QVector<int>& data)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, ColumnMode::Integer, new QVector<int>(data))) {
	init();
}

Column::Column(const QString& name, const QVector<qint64>& data)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, ColumnMode::BigInt, new QVector<qint64>(data))) {
	init();
}

Column::Column(const QString& name, const QVector<QString>& data)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, ColumnMode::Text, new QVector<QString>(data))) {
	init();
}

Column::Column(const QString& name, const QVector<QDateTime>& data, ColumnMode mode)
	: AbstractColumn(name, AspectType::Column)
	, d(new ColumnPrivate(this, mode, new QVector<QDateTime>(data))) {
	init();
}

/**
 * \brief Common part of ctors
 */
void Column::init() {
	m_string_io = new ColumnStringIO(this);
	d->inputFilter()->input(0, m_string_io);
	d->outputFilter()->input(0, this);
	d->inputFilter()->setHidden(true);
	d->outputFilter()->setHidden(true);
	addChildFast(d->inputFilter());
	addChildFast(d->outputFilter());
}

Column::~Column() {
	delete m_string_io;
	delete d;
}

QMenu* Column::createContextMenu() {
	if (parentAspect()->type() == AspectType::StatisticsSpreadsheet)
		return nullptr;

	// initialize the actions if not done yet
	if (!m_copyDataAction) {
		m_copyDataAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy Data"), this);
		connect(m_copyDataAction, &QAction::triggered, this, &Column::copyData);

		m_pasteDataAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("Paste Data"), this);
		connect(m_pasteDataAction, &QAction::triggered, this, &Column::pasteData);

		m_usedInActionGroup = new QActionGroup(this);
		connect(m_usedInActionGroup, &QActionGroup::triggered, this, &Column::navigateTo);
		connect(this, &AbstractColumn::maskingChanged, this, [=] {
			d->invalidate();
		});
	}

	QMenu* menu = AbstractAspect::createContextMenu();
	QAction* firstAction{nullptr};

	// insert after "rename" and "delete" actions, if available.
	// MQTTTopic columns don't have these actions
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	// add actions available in SpreadsheetView
	// TODO: we don't need to add anything from the view for MQTTTopic columns.
	// at the moment it's ok to check to the null pointer for firstAction here.
	// later, once we have some actions in the menu also for MQTT topics we'll
	// need to explicitly to dynamic_cast for MQTTTopic
	if (firstAction) {
		if (auto* spreadsheet = parentAspect()->castTo<Spreadsheet>()) {
			spreadsheet->fillColumnContextMenu(menu, this);
		}
#if defined(HAVE_CANTOR_LIBS) && !defined(SDK)
		else if (auto* notebook = parentAspect()->castTo<Notebook>()) {
			notebook->fillColumnContextMenu(menu, this);
		}
#endif
	}

	//"Used in" menu containing all plots where the column is used
	QMenu* usedInMenu = new QMenu(i18n("Used in"));
	usedInMenu->setIcon(QIcon::fromTheme(QStringLiteral("go-next-view")));

	// remove previously added actions
	for (auto* action : m_usedInActionGroup->actions())
		m_usedInActionGroup->removeAction(action);

	auto* project = this->project();
	bool showIsUsed = false;

	// add curves where the column is currently in use
	bool sectionAdded = false;
	const auto& plots = project->children<Plot>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* plot : plots) {
		const bool used = plot->usingColumn(this, true);
		if (used) {
			if (!sectionAdded) {
				usedInMenu->addSection(i18n("Plots"));
				sectionAdded = true;
			}

			auto* action = new QAction(plot->icon(), plot->name(), m_usedInActionGroup);
			action->setData(plot->path());
			usedInMenu->addAction(action);
			showIsUsed = true;
		}
	}

	// add axes where the column is used as a custom column for ticks positions or labels
	sectionAdded = false;
	const auto& axes = project->children<Axis>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* axis : axes) {
		const bool used = (axis->majorTicksColumn() == this || axis->minorTicksColumn() == this || axis->labelsTextColumn() == this);
		if (used) {
			if (!sectionAdded) {
				usedInMenu->addSection(i18n("Axes"));
				sectionAdded = true;
			}

			auto* action = new QAction(axis->icon(), axis->name(), m_usedInActionGroup);
			action->setData(axis->path());
			usedInMenu->addAction(action);
			showIsUsed = true;
		}
	}

	// add calculated columns where the column is used in formula variables
	sectionAdded = false;
	const auto& columns = project->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
	const QString& path = this->path();
	for (const auto* column : columns) {
		int index = -1;
		for (int i = 0; i < column->formulaData().count(); i++) {
			if (path == column->formulaData().at(i).columnName()) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			if (!sectionAdded) {
				usedInMenu->addSection(i18n("Calculations"));
				sectionAdded = true;
			}

			auto* action = new QAction(column->icon(), column->name(), m_usedInActionGroup);
			action->setData(column->path());
			usedInMenu->addAction(action);
			showIsUsed = true;
		}
	}

	if (firstAction)
		menu->insertSeparator(firstAction);

	if (showIsUsed) {
		menu->insertMenu(firstAction, usedInMenu);
		menu->insertSeparator(firstAction);
	}

	if (hasValues())
		menu->insertAction(firstAction, m_copyDataAction);

	// pasting of data is only possible for spreadsheet columns that are not read-only
	if (auto* spreadsheet = parentAspect()->castTo<Spreadsheet>()) {
		if (!spreadsheet->readOnly()) {
			const auto* mimeData = QApplication::clipboard()->mimeData();
			if (mimeData->hasFormat(QStringLiteral("text/plain"))) {
				const QString& text = QApplication::clipboard()->text();
				if (!text.startsWith(QLatin1String("<?xml version=\"1.0\"?><!DOCTYPE LabPlotCopyPasteXML>")))
					menu->insertAction(firstAction, m_pasteDataAction);
			}
		}
	}

	menu->insertSeparator(firstAction);

	return menu;
}

void Column::updateLocale() {
	const auto numberLocale = QLocale();
	d->inputFilter()->setNumberLocale(numberLocale);
	d->outputFilter()->setNumberLocale(numberLocale);
}

void Column::navigateTo(QAction* action) {
	project()->navigateTo(action->data().toString());
}

/*!
 * copies the values of the column to the clipboard
 */
void Column::copyData() {
	QString output;
	int rows = rowCount();

	// TODO: use locale of filter?
	const auto numberLocale = QLocale();
	if (columnMode() == ColumnMode::Double) {
		const Double2StringFilter* filter = static_cast<Double2StringFilter*>(outputFilter());
		char format = filter->numericFormat();
		for (int r = 0; r < rows; r++) {
			output += numberLocale.toString(doubleAt(r), format, 16); // copy with max. precision
			if (r < rows - 1)
				output += QLatin1Char('\n');
		}
	} else if (columnMode() == ColumnMode::Integer || columnMode() == ColumnMode::BigInt) {
		for (int r = 0; r < rowCount(); r++) {
			output += numberLocale.toString(valueAt(r));
			if (r < rows - 1)
				output += QLatin1Char('\n');
		}
	} else {
		for (int r = 0; r < rowCount(); r++) {
			output += asStringColumn()->textAt(r);
			if (r < rows - 1)
				output += QLatin1Char('\n');
		}
	}

	QApplication::clipboard()->setText(output);
}

void Column::setSparkline(const QPixmap& pix) {
	m_sparkline = pix;
}

QPixmap Column::sparkline() {
	return m_sparkline;
}

void Column::pasteData() {
#ifndef SDK
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(parentAspect());
	if (spreadsheet)
		static_cast<SpreadsheetView*>(spreadsheet->view())->pasteIntoSelection();
#endif
}

void Column::addUsedInPlots(QVector<CartesianPlot*>& plotAreas) {
	const Project* project = this->project();

	// when executing tests we don't create any project,
	// add a null-pointer check for tests here.
	if (!project)
		return;

	const auto& plots = project->children<const Plot>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* plot : plots) {
		if (plot->usingColumn(this)) {
			auto* plotArea = static_cast<CartesianPlot*>(plot->parentAspect());
			if (plotAreas.indexOf(plotArea) == -1)
				plotAreas << plotArea;
		}
	}
}

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if necessary, converts it to another datatype.
 */
void Column::setColumnMode(AbstractColumn::ColumnMode mode) {
	DEBUG(Q_FUNC_INFO)
	if (mode == columnMode())
		return;

	beginMacro(i18n("%1: change column type", name()));

	auto* old_input_filter = d->inputFilter();
	auto* old_output_filter = d->outputFilter();
	exec(new ColumnSetModeCmd(d, mode));

	if (d->inputFilter() != old_input_filter) {
		DEBUG(Q_FUNC_INFO << ", INPUT")
		removeChild(old_input_filter);
		addChild(d->inputFilter());
		d->inputFilter()->input(0, m_string_io);
	}
	if (d->outputFilter() != old_output_filter) {
		DEBUG(Q_FUNC_INFO << ", OUTPUT")
		removeChild(old_output_filter);
		addChild(d->outputFilter());
		d->outputFilter()->input(0, this);
	}

	endMacro();
}

void Column::setColumnModeFast(AbstractColumn::ColumnMode mode) {
	if (mode == columnMode())
		return;

	auto* old_input_filter = d->inputFilter();
	auto* old_output_filter = d->outputFilter();
	exec(new ColumnSetModeCmd(d, mode));

	if (d->inputFilter() != old_input_filter) {
		removeChild(old_input_filter);
		addChildFast(d->inputFilter());
		d->inputFilter()->input(0, m_string_io);
	}
	if (d->outputFilter() != old_output_filter) {
		removeChild(old_output_filter);
		addChildFast(d->outputFilter());
		d->outputFilter()->input(0, this);
	}
}

bool Column::isDraggable() const {
	return true;
}

QVector<AspectType> Column::dropableOn() const {
	return QVector<AspectType>{AspectType::CartesianPlot};
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool Column::copy(const AbstractColumn* other) {
	Q_CHECK_PTR(other);
	if (other->columnMode() != columnMode())
		return false;
	exec(new ColumnFullCopyCmd(d, other));
	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param source pointer to the column to copy
 * \param source_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool Column::copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) {
	Q_CHECK_PTR(source);
	if (source->columnMode() != columnMode())
		return false;
	exec(new ColumnPartialCopyCmd(d, source, source_start, dest_start, num_rows));
	return true;
}

void Column::invalidateProperties() {
	d->invalidate();
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void Column::handleRowInsertion(int before, int count) {
	AbstractColumn::handleRowInsertion(before, count);
	exec(new ColumnInsertRowsCmd(d, before, count));
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void Column::handleRowRemoval(int first, int count) {
	AbstractColumn::handleRowRemoval(first, count);
	exec(new ColumnRemoveRowsCmd(d, first, count));
}

/**
 * \brief Set the column plot designation
 */
void Column::setPlotDesignation(AbstractColumn::PlotDesignation pd) {
	if (pd != plotDesignation())
		exec(new ColumnSetPlotDesignationCmd(d, pd));
}

/**
 * \brief Get width
 */
int Column::width() const {
	return d->width();
}

/**
 * \brief Set width
 */
void Column::setWidth(int value) {
	d->setWidth(value);
}

/**
 * \brief Clear the content of the column (data and formula definition)
 */
void Column::clear() {
	if (d->formula().isEmpty())
		exec(new ColumnClearCmd(d));
	else {
		beginMacro(i18n("%1: clear column", name()));
		exec(new ColumnClearCmd(d));
		exec(new ColumnSetGlobalFormulaCmd(d, QString(), QStringList(), QVector<Column*>(), false /* auto update */, true /* auto resize */));
		endMacro();
	}
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Returns the formula used to generate column values
 */
QString Column::formula() const {
	return d->formula();
}

const QVector<Column::FormulaData>& Column::formulaData() const {
	return d->formulaData();
}

void Column::setFormulaVariableColumn(Column* c) {
	d->setFormulVariableColumn(c);
}

void Column::setFormulVariableColumnsPath(int index, const QString& path) {
	d->setFormulVariableColumnsPath(index, path);
}

void Column::setFormulVariableColumn(int index, Column* column) {
	d->setFormulVariableColumn(index, column);
}

bool Column::formulaAutoUpdate() const {
	return d->formulaAutoUpdate();
}

bool Column::formulaAutoResize() const {
	return d->formulaAutoResize();
}

/**
 * \brief Sets the formula used to generate column values
 */
void Column::setFormula(const QString& formula, const QStringList& variableNames, const QVector<Column*>& columns, bool autoUpdate, bool autoResize) {
	exec(new ColumnSetGlobalFormulaCmd(d, formula, variableNames, columns, autoUpdate, autoResize));
}

/**
 * \brief Clears the formula used to generate column values
 */
void Column::clearFormula() {
	setFormula(QString(), QStringList(), QVector<Column*>());
}

/*!
 * in case the cell values are calculated via a global column formula,
 * updates the values on data changes in all the dependent changes in the
 * "variable columns".
 */
void Column::updateFormula() {
	invalidateProperties();
	d->updateFormula();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void Column::setFormula(const Interval<int>& i, const QString& formula) {
	exec(new ColumnSetFormulaCmd(d, i, formula));
}

/**
 * \brief Overloaded function for convenience
 */
void Column::setFormula(int row, const QString& formula) {
	setFormula(Interval<int>(row, row), formula);
}

/**
 * \brief Clear all formulas
 */
void Column::clearFormulas() {
	exec(new ColumnClearFormulasCmd(d));
}

STD_SETTER_CMD_IMPL(Column, SetRandomValuesData, Column::RandomValuesData, randomValuesData)
void Column::setRandomValuesData(const RandomValuesData& data) {
	exec(new ColumnSetRandomValuesDataCmd(d, data, ki18n("%1: set random values")));
}

Column::RandomValuesData Column::randomValuesData() const {
	return d->randomValuesData;
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void Column::setTextAt(int row, const QString& new_value) {
	exec(new ColumnSetCmd<QString>(d, row, textAt(row), new_value));
}

void Column::setText(const QVector<QString>& texts) {
	replaceTexts(-1, texts);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void Column::replaceTexts(int first, const QVector<QString>& new_values) {
	if (isLoading())
		d->replaceTexts(first, new_values);
	else
		exec(new ColumnReplaceCmd<QString>(d, first, new_values));
}

int Column::dictionaryIndex(int row) const {
	return d->dictionaryIndex(row);
}

const QMap<QString, int>& Column::frequencies() const {
	return d->frequencies();
}

void Column::addValueLabel(const QString& value, const QString& label) {
	d->addValueLabel(value, label);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setDateAt(int row, QDate new_value) {
	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setTimeAt(int row, QTime new_value) {
	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setDateTimeAt(int row, const QDateTime& new_value) {
	if (isLoading())
		d->setValueAt(row, new_value);
	else
		exec(new ColumnSetCmd<QDateTime>(d, row, dateTimeAt(row), new_value));
}

void Column::setDateTimes(const QVector<QDateTime>& dateTimes) {
	replaceDateTimes(-1, dateTimes);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
	if (isLoading())
		d->replaceDateTimes(first, new_values);
	else
		exec(new ColumnReplaceCmd<QDateTime>(d, first, new_values));
}

void Column::addValueLabel(const QDateTime& value, const QString& label) {
	d->addValueLabel(value, label);
	setProjectChanged(true);
}

void Column::setValues(const QVector<double>& values) {
	replaceValues(-1, values);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void Column::setValueAt(int row, const double new_value) {
	if (isLoading())
		d->setValueAt(row, new_value);
	else
		exec(new ColumnSetCmd<double>(d, row, valueAt(row), new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void Column::replaceValues(int first, const QVector<double>& new_values) {
	if (isLoading())
		d->replaceValues(first, new_values);
	else
		exec(new ColumnReplaceCmd<double>(d, first, new_values));
}

void Column::addValueLabel(double value, const QString& label) {
	d->addValueLabel(value, label);
	setProjectChanged(true);
}

void Column::setIntegers(const QVector<int>& integers) {
	replaceInteger(-1, integers);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void Column::setIntegerAt(int row, const int new_value) {
	if (isLoading())
		d->setValueAt(row, new_value);
	else
		exec(new ColumnSetCmd<int>(d, row, integerAt(row), new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void Column::replaceInteger(int first, const QVector<int>& new_values) {
	if (isLoading())
		d->replaceInteger(first, new_values);
	else
		exec(new ColumnReplaceCmd<int>(d, first, new_values));
}

void Column::addValueLabel(int value, const QString& label) {
	d->addValueLabel(value, label);
	setProjectChanged(true);
}

void Column::setBigInts(const QVector<qint64>& bigInts) {
	replaceBigInt(-1, bigInts);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
void Column::setBigIntAt(int row, const qint64 new_value) {
	if (isLoading())
		d->setValueAt(row, new_value);
	else
		exec(new ColumnSetCmd<qint64>(d, row, bigIntAt(row), new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is BigInt
 */
void Column::replaceBigInt(int first, const QVector<qint64>& new_values) {
	if (isLoading())
		d->replaceBigInt(first, new_values);
	else
		exec(new ColumnReplaceCmd<qint64>(d, first, new_values));
}

void Column::addValueLabel(qint64 value, const QString& label) {
	d->addValueLabel(value, label);
	setProjectChanged(true);
}

/*!
 * \brief Column::properties
 * Returns the column properties of this curve (monoton increasing, monoton decreasing, ... )
 * \see AbstractColumn::properties
 */
AbstractColumn::Properties Column::properties() const {
	if (!d->available.properties)
		d->updateProperties();

	return d->properties;
}

const Column::ColumnStatistics& Column::statistics() const {
	if (!d->available.statistics)
		d->calculateStatistics();

	return d->statistics;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void Column::setData(void* data) {
	d->setData(data);
}

void* Column::data() const {
	return d->data();
}

/*!
 * return \c true if the column has at least one valid (not empty) value, \c false otherwise.
 */
bool Column::hasValues() const {
	if (d->available.hasValues)
		return d->hasValues;

	bool foundValues = false;
	switch (columnMode()) {
	case ColumnMode::Double: {
		for (int row = 0; row < rowCount(); ++row) {
			if (!std::isnan(doubleAt(row))) {
				foundValues = true;
				break;
			}
		}
		break;
	}
	case ColumnMode::Text: {
		for (int row = 0; row < rowCount(); ++row) {
			if (!textAt(row).isEmpty()) {
				foundValues = true;
				break;
			}
		}
		break;
	}
	case ColumnMode::Integer:
	case ColumnMode::BigInt:
		// integer values are always valid
		foundValues = true;
		break;
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day: {
		for (int row = 0; row < rowCount(); ++row) {
			if (dateTimeAt(row).isValid()) {
				foundValues = true;
				break;
			}
		}
		break;
	}
	}

	d->hasValues = foundValues;
	d->available.hasValues = true;
	return d->hasValues;
}

/*!
 * return \c true if the column has a valid value in the row \row.
 */
bool Column::hasValueAt(int row) const {
	bool foundValue = false;
	switch (columnMode()) {
	case ColumnMode::Double: {
		foundValue = !std::isnan(doubleAt(row));
		break;
	}
	case ColumnMode::Text: {
		foundValue = !textAt(row).isEmpty();
		break;
	}
	case ColumnMode::Integer:
	case ColumnMode::BigInt:
		// integer values are always valid
		foundValue = true;
		break;
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day: {
		foundValue = dateTimeAt(row).isValid();
		break;
	}
	}

	return foundValue;
}

/*
 * set item at i to col[j] for same columnMode()
 */

void Column::setFromColumn(int i, AbstractColumn* col, int j) {
	if (col->columnMode() != columnMode())
		return;

	switch (columnMode()) {
	case ColumnMode::Double:
		setValueAt(i, col->doubleAt(j));
		break;
	case ColumnMode::Integer:
		setIntegerAt(i, col->integerAt(j));
		break;
	case ColumnMode::BigInt:
		setBigIntAt(i, col->bigIntAt(j));
		break;
	case ColumnMode::Text:
		setTextAt(i, col->textAt(j));
		break;
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day:
		setDateTimeAt(i, col->dateTimeAt(j));
		break;
	}
}

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString Column::textAt(int row) const {
	return d->textAt(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate Column::dateAt(int row) const {
	return d->dateAt(row);
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime Column::timeAt(int row) const {
	return d->timeAt(row);
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime Column::dateTimeAt(int row) const {
	return d->dateTimeAt(row);
}

double Column::doubleAt(int row) const {
	return d->doubleAt(row);
}

/**
 * \brief Return the double value in row 'row'
 */
double Column::valueAt(int row) const {
	return d->valueAt(row);
}

/**
 * \brief Return the int value in row 'row'
 */
int Column::integerAt(int row) const {
	return d->integerAt(row);
}

/**
 * \brief Return the bigint value in row 'row'
 */
qint64 Column::bigIntAt(int row) const {
	return d->bigIntAt(row);
}

bool Column::valueLabelsInitialized() const {
	return d->valueLabelsInitialized();
}

double Column::valueLabelsMinimum() const {
	return d->valueLabelsMinimum();
}

double Column::valueLabelsMaximum() const {
	return d->valueLabelsMaximum();
}

void Column::setLabelsMode(ColumnMode mode) {
	d->setLabelsMode(mode);
	setProjectChanged(true);
}

void Column::valueLabelsRemoveAll() {
	d->valueLabelsRemoveAll();
	setProjectChanged(true);
}

void Column::removeValueLabel(const QString& key) {
	d->removeValueLabel(key);
	setProjectChanged(true);
}

const QVector<Column::ValueLabel<QString>>* Column::textValueLabels() const {
	return d->textValueLabels();
}

const QVector<Column::ValueLabel<QDateTime>>* Column::dateTimeValueLabels() const {
	return d->dateTimeValueLabels();
}

int Column::valueLabelsCount() const {
	return d->valueLabelsCount();
}

int Column::valueLabelsCount(double min, double max) const {
	return d->valueLabelsCount(min, max);
}

int Column::valueLabelsIndexForValue(double x, bool smaller) const {
	return d->valueLabelsIndexForValue(x, smaller);
}

double Column::valueLabelsValueAt(int index) const {
	return d->valueLabelsValueAt(index);
}

QString Column::valueLabelAt(int index) const {
	return d->valueLabelAt(index);
}

const QVector<Column::ValueLabel<double>>* Column::valueLabels() const {
	return d->valueLabels();
}

const QVector<Column::ValueLabel<int>>* Column::intValueLabels() const {
	return d->intValueLabels();
}

const QVector<Column::ValueLabel<qint64>>* Column::bigIntValueLabels() const {
	return d->bigIntValueLabels();
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return an icon to be used for decorating the views and spreadsheet column headers
 */
QIcon Column::icon() const {
	if (formula().isEmpty())
		return modeIcon(columnMode());
	else
		return QIcon::fromTheme(QLatin1String("mathmode"));
}

QString Column::caption() const {
	QString caption = AbstractAspect::caption();

	caption += QLatin1String("<br>");
	caption += QLatin1String("<br>") + i18n("Size: %1", rowCount());
	// TODO: active this once we have a more efficient implementation of this function
	// caption += QLatin1String("<br>") + i18n("Values: %1", col->availableRowCount());
	caption += QLatin1String("<br>") + i18n("Type: %1", columnModeString());
	caption += QLatin1String("<br>") + i18n("Plot Designation: %1", plotDesignationString());

	// in case it's a calculated column, add additional information about the formula and parameters
	if (!formula().isEmpty()) {
		caption += QLatin1String("<br><br><b>") + i18n("Formula:") + QLatin1String("</b>");
		QString f(QStringLiteral("f("));
		QString parameters;
		for (int i = 0; i < formulaData().size(); ++i) {
			auto& data = formulaData().at(i);

			// string for the function definition like f(x,y), etc.
			f += data.variableName();
			if (i != formulaData().size() - 1)
				f += QStringLiteral(", ");

			// string for the parameters and the references to the used columns for them
			if (!parameters.isEmpty())
				parameters += QLatin1String("<br>");
			parameters += data.variableName();
			if (data.column())
				parameters += QStringLiteral(" = ") + data.column()->path();
		}

		caption += QStringLiteral("<br>") + f + QStringLiteral(") = ") + formula();
		caption += QStringLiteral("<br>") + parameters;
		if (formulaAutoUpdate())
			caption += QStringLiteral("<br>") + i18n("auto update: true") + QStringLiteral("   (*)");
		else
			caption += QStringLiteral("<br>") + i18n("auto update: false");
	}

	// add the information about the usage of this column in other places, similar to the logic in createContextMenu().
	auto* project = this->project();

	// add curves where the column is currently in use
	bool sectionAdded = false;
	const auto& plots = project->children<Plot>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* plot : plots) {
		const bool used = plot->usingColumn(this, true);
		if (used) {
			if (!sectionAdded) {
				caption += QStringLiteral("<br><br><b>") + i18n("Used in Plots:") + QStringLiteral("</b>");
				sectionAdded = true;
			}

			caption += QStringLiteral("<br>") + plot->path();
		}
	}

	// add axes where the column is used as a custom column for ticks positions or labels
	sectionAdded = false;
	const auto& axes = project->children<Axis>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* axis : axes) {
		const bool used = (axis->majorTicksColumn() == this || axis->minorTicksColumn() == this || axis->labelsTextColumn() == this);
		if (used) {
			if (!sectionAdded) {
				caption += QStringLiteral("<br><br><b>") + i18n("Used in Axes:") + QStringLiteral("</b>");
				sectionAdded = true;
			}

			caption += QStringLiteral("<br>") + axis->path();
		}
	}

	// add calculated columns where the column is used in formula variables
	sectionAdded = false;
	const auto& columns = project->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
	const QString& path = this->path();
	for (const auto* column : columns) {
		int index = -1;
		for (int i = 0; i < column->formulaData().count(); i++) {
			if (path == column->formulaData().at(i).columnName()) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			if (!sectionAdded) {
				caption += QStringLiteral("<br><br><b>") + i18n("Used in Spreadsheet Calculations:") + QStringLiteral("</b>");
				sectionAdded = true;
			}

			caption += QStringLiteral("<br>") + column->path();
		}
	}

	return caption;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Save the column as XML
 */
void Column::save(QXmlStreamWriter* writer) const {
	bool saveData = true;
	if (project() && !project()->saveData()) {
		saveData = false;
	}

	PERFTRACE(QStringLiteral("save column ") + name());

	writer->writeStartElement(QStringLiteral("column"));
	writeBasicAttributes(writer);

	writer->writeAttribute(QStringLiteral("rows"), QString::number(saveData ? rowCount() : 0));
	writer->writeAttribute(QStringLiteral("designation"), QString::number(static_cast<int>(plotDesignation())));
	writer->writeAttribute(QStringLiteral("mode"), QString::number(static_cast<int>(columnMode())));
	writer->writeAttribute(QStringLiteral("width"), QString::number(width()));

	// save the formula used to generate column values, if available
	if (!formula().isEmpty()) {
		writer->writeStartElement(QStringLiteral("formula"));
		writer->writeAttribute(QStringLiteral("autoUpdate"), QString::number(d->formulaAutoUpdate()));
		writer->writeAttribute(QStringLiteral("autoResize"), QString::number(d->formulaAutoResize()));
		writer->writeTextElement(QStringLiteral("text"), formula());

		QStringList formulaVariableNames;
		QStringList formulaVariableColumnPaths;
		for (auto& d : formulaData()) {
			formulaVariableNames << d.variableName();
			formulaVariableColumnPaths << d.columnName();
		}

		writer->writeStartElement(QStringLiteral("variableNames"));
		for (const auto& name : formulaVariableNames)
			writer->writeTextElement(QStringLiteral("name"), name);
		writer->writeEndElement();

		writer->writeStartElement(QStringLiteral("columnPathes"));
		for (const auto& path : formulaVariableColumnPaths)
			writer->writeTextElement(QStringLiteral("path"), path);
		writer->writeEndElement();

		writer->writeEndElement();
	}

	if (d->randomValuesData.available) {
		writer->writeStartElement(QStringLiteral("randomValuesData"));
		writer->writeAttribute(QStringLiteral("distribution"), QString::number(static_cast<int>(d->randomValuesData.distribution)));
		writer->writeAttribute(QStringLiteral("parameter1"), QString::number(d->randomValuesData.parameter1));
		writer->writeAttribute(QStringLiteral("parameter2"), QString::number(d->randomValuesData.parameter2));
		writer->writeAttribute(QStringLiteral("parameter3"), QString::number(d->randomValuesData.parameter3));
		writer->writeAttribute(QStringLiteral("seed"), QString::number(d->randomValuesData.seed));
		writer->writeEndElement();
	}

	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("input_filter"));
	d->inputFilter()->save(writer);
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("output_filter"));
	d->outputFilter()->save(writer);
	writer->writeEndElement();

	XmlWriteMask(writer);

	// TODO: formula in cells is not implemented yet
	//  	QVector< Interval<int> > formulas = formulaIntervals();
	//  	foreach(const Interval<int>& interval, formulas) {
	//  		writer->writeStartElement(QStringLiteral("formula"));
	//  		writer->writeAttribute(QStringLiteral("start_row"), QString::number(interval.start()));
	//  		writer->writeAttribute(QStringLiteral("end_row"), QString::number(interval.end()));
	//  		writer->writeCharacters(formula(interval.start()));
	//  		writer->writeEndElement();
	//  	}

	// value labels
	if (valueLabelsInitialized()) {
		writer->writeStartElement(QStringLiteral("valueLabels"));
		switch (d->m_labels.mode()) {
		case AbstractColumn::ColumnMode::Double: {
			const auto* labels = const_cast<Column*>(this)->valueLabels();
			if (labels) {
				auto it = labels->constBegin();
				while (it != labels->constEnd()) {
					writer->writeStartElement(QStringLiteral("valueLabel"));
					writer->writeAttribute(QStringLiteral("value"), QString::number(it->value));
					writer->writeAttribute(QStringLiteral("label"), it->label);
					writer->writeEndElement();
					++it;
				}
			}
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			const auto* labels = const_cast<Column*>(this)->intValueLabels();
			if (labels) {
				auto it = labels->constBegin();
				while (it != labels->constEnd()) {
					writer->writeStartElement(QStringLiteral("valueLabel"));
					writer->writeAttribute(QStringLiteral("value"), QString::number(it->value));
					writer->writeAttribute(QStringLiteral("label"), it->label);
					writer->writeEndElement();
					++it;
				}
			}
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			const auto* labels = const_cast<Column*>(this)->bigIntValueLabels();
			if (labels) {
				auto it = labels->constBegin();
				while (it != labels->constEnd()) {
					writer->writeStartElement(QStringLiteral("valueLabel"));
					writer->writeAttribute(QStringLiteral("value"), QString::number(it->value));
					writer->writeAttribute(QStringLiteral("label"), it->label);
					writer->writeEndElement();
					++it;
				}
			}
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			const auto* labels = const_cast<Column*>(this)->textValueLabels();
			if (labels) {
				auto it = labels->constBegin();
				while (it != labels->constEnd()) {
					writer->writeStartElement(QStringLiteral("valueLabel"));
					writer->writeAttribute(QStringLiteral("value"), it->value);
					writer->writeAttribute(QStringLiteral("label"), it->label);
					writer->writeEndElement();
					++it;
				}
			}
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			const auto* labels = const_cast<Column*>(this)->dateTimeValueLabels();
			if (labels) {
				auto it = labels->constBegin();
				while (it != labels->constEnd()) {
					writer->writeStartElement(QStringLiteral("valueLabel"));
					writer->writeAttribute(QStringLiteral("value"), QString::number(it->value.toMSecsSinceEpoch()));
					writer->writeAttribute(QStringLiteral("label"), it->label);
					writer->writeEndElement();
					++it;
				}
			}
			break;
		}
		}

		writer->writeEndElement(); // "valueLabels"
	}

	// conditional formatting
	if (hasHeatmapFormat()) {
		writer->writeStartElement(QStringLiteral("heatmapFormat"));
		const auto& format = heatmapFormat();
		writer->writeAttribute(QStringLiteral("min"), QString::number(format.min));
		writer->writeAttribute(QStringLiteral("max"), QString::number(format.max));
		writer->writeAttribute(QStringLiteral("name"), format.name);
		writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(format.type)));
		for (const auto& color : format.colors) {
			writer->writeStartElement(QStringLiteral("color"));
			WRITE_QCOLOR(color)
			writer->writeEndElement(); // "color"
		}
		writer->writeEndElement(); // "heatmapFormat"
	}

	// data
	if (saveData) {
		switch (columnMode()) {
		case ColumnMode::Double: {
			const char* data = reinterpret_cast<const char*>(static_cast<QVector<double>*>(d->data())->constData());
			size_t size = d->rowCount() * sizeof(double);
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, (int)size).toBase64()));
			break;
		}
		case ColumnMode::Integer: {
			const char* data = reinterpret_cast<const char*>(static_cast<QVector<int>*>(d->data())->constData());
			size_t size = d->rowCount() * sizeof(int);
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, (int)size).toBase64()));
			break;
		}
		case ColumnMode::BigInt: {
			const char* data = reinterpret_cast<const char*>(static_cast<QVector<qint64>*>(d->data())->constData());
			size_t size = d->rowCount() * sizeof(qint64);
			writer->writeCharacters(QLatin1String(QByteArray::fromRawData(data, (int)size).toBase64()));
			break;
		}
		case ColumnMode::Text: {
			// serialize text data using Base64 encoding with null separator
			QByteArray bytes;
			for (int i = 0; i < rowCount(); ++i) {
				bytes.append(textAt(i).toUtf8());
				bytes.append('\0');
			}
			bytes.removeLast(); // remove the last null character
			writer->writeCharacters(QLatin1String(bytes.toBase64()));
			break;
		}
		case ColumnMode::DateTime:
		case ColumnMode::Month:
		case ColumnMode::Day:
			for (int i = 0; i < rowCount(); ++i) {
				writer->writeStartElement(QStringLiteral("row"));
				writer->writeAttribute(QStringLiteral("index"), QString::number(i));
				writer->writeCharacters(dateTimeAt(i).toString(QLatin1String("yyyy-dd-MM hh:mm:ss:zzz")));
				writer->writeEndElement();
			}
			break;
		}
	}

	writer->writeEndElement(); // "column"
}

// TODO: extra header
class DecodeColumnTask : public QRunnable {
public:
	DecodeColumnTask(ColumnPrivate* priv, const QString& content)
		: m_private(priv)
		, m_content(content) { };
	void run() override {
		QByteArray bytes = QByteArray::fromBase64(m_content.toLatin1());
		switch (m_private->columnMode()) {
		case AbstractColumn::ColumnMode::Double: {
			auto* data = new QVector<double>(bytes.size() / (int)sizeof(double));
			memcpy(data->data(), bytes.data(), bytes.size());
			m_private->replaceData(data);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* data = new QVector<qint64>(bytes.size() / (int)sizeof(qint64));
			memcpy(data->data(), bytes.data(), bytes.size());
			m_private->replaceData(data);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* data = new QVector<int>(bytes.size() / (int)sizeof(int));
			memcpy(data->data(), bytes.data(), bytes.size());
			m_private->replaceData(data);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			// deserialize text data using Base64 encoding with null separator
			const QByteArray bytes = QByteArray::fromBase64(m_content.toLatin1());
			const QString decoded = QString::fromUtf8(bytes);
			auto textVector = decoded.split(QLatin1Char('\0'), Qt::KeepEmptyParts);
			m_private->replaceTexts(-1, textVector);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day: {
			// nothing to do here, the data are loaded row by row
			break;
		}
		}
	}

private:
	ColumnPrivate* m_private;
	QString m_content;
};

/**
 * \brief Load the column from XML
 */
bool Column::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	PERFTRACE(QLatin1String("load column ") + name());
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value(QStringLiteral("rows")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("rows"));
	else
		d->resizeTo(str.toInt());

	str = attribs.value(QStringLiteral("designation")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("designation"));
	else
		d->setPlotDesignation(AbstractColumn::PlotDesignation(str.toInt()));

	str = attribs.value(QStringLiteral("mode")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("mode"));
	else
		setColumnModeFast(AbstractColumn::ColumnMode(str.toInt()));

	str = attribs.value(QStringLiteral("width")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("width"));
	else
		d->setWidth(str.toInt());

	QVector<QDateTime> dateTimeVector;
	QVector<QString> textVector;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QLatin1String("column"))
			break;

		if (reader->isStartElement()) {
			bool ret_val = true;
			if (reader->name() == QLatin1String("comment"))
				ret_val = readCommentElement(reader);
			else if (reader->name() == QLatin1String("input_filter"))
				ret_val = XmlReadInputFilter(reader);
			else if (reader->name() == QLatin1String("output_filter"))
				ret_val = XmlReadOutputFilter(reader);
			else if (reader->name() == QLatin1String("mask"))
				ret_val = XmlReadMask(reader);
			else if (reader->name() == QLatin1String("formula"))
				ret_val = XmlReadFormula(reader);
			else if (reader->name() == QLatin1String("randomValuesData")) {
				attribs = reader->attributes();
				d->randomValuesData.available = true;
				READ_INT_VALUE("distribution", randomValuesData.distribution, nsl_sf_stats_distribution);
				READ_DOUBLE_VALUE("parameter1", randomValuesData.parameter1);
				READ_DOUBLE_VALUE("parameter2", randomValuesData.parameter2);
				READ_DOUBLE_VALUE("parameter3", randomValuesData.parameter3);
				READ_DOUBLE_VALUE("seed", randomValuesData.seed);
			} else if (reader->name() == QLatin1String("heatmapFormat")) {
				attribs = reader->attributes();

				auto& format = heatmapFormat();
				str = attribs.value(QStringLiteral("min")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("min"));
				else
					format.min = str.toDouble();

				str = attribs.value(QStringLiteral("max")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("max"));
				else
					format.max = str.toDouble();

				str = attribs.value(QStringLiteral("name")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("name"));
				else
					format.name = str;

				str = attribs.value(QStringLiteral("type")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("max"));
				else
					format.type = static_cast<Formatting>(str.toInt());

				ret_val = true;
			} else if (reader->name() == QLatin1String("color")) {
				attribs = reader->attributes();
				QColor color;
				READ_QCOLOR(color);
				auto& format = heatmapFormat();
				format.colors << color;
				ret_val = true;
			} else if (reader->name() == QLatin1String("valueLabels")) {
				continue;
			} else if (reader->name() == QLatin1String("valueLabel")) {
				attribs = reader->attributes();
				const QString& label = attribs.value(QLatin1String("label")).toString();
				switch (columnMode()) {
				case AbstractColumn::ColumnMode::Double:
					addValueLabel(attribs.value(QLatin1String("value")).toDouble(), label);
					break;
				case AbstractColumn::ColumnMode::Integer:
					addValueLabel(attribs.value(QLatin1String("value")).toInt(), label);
					break;
				case AbstractColumn::ColumnMode::BigInt:
					addValueLabel(attribs.value(QLatin1String("value")).toLongLong(), label);
					break;
				case AbstractColumn::ColumnMode::Text:
					addValueLabel(attribs.value(QLatin1String("value")).toString(), label);
					break;
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::DateTime:
					addValueLabel(QDateTime::fromMSecsSinceEpoch(attribs.value(QLatin1String("value")).toLongLong(), QTimeZone::UTC), label);
					break;
				}
			} else if (reader->name() == QLatin1String("row")) {
				// Assumption: the next elements are all rows
				switch (columnMode()) {
				case Column::ColumnMode::Double:
				case Column::ColumnMode::BigInt:
				case Column::ColumnMode::Integer:
					/* handled differently*/
					break;
				case Column::ColumnMode::DateTime:
				case Column::ColumnMode::Month:
				case Column::ColumnMode::Day: {
					dateTimeVector << QDateTime::fromString(reader->readElementText() + QStringLiteral("Z"),
															QStringLiteral("yyyy-dd-MM hh:mm:ss:zzzt")); // timezone is important
					break;
				}
				case Column::ColumnMode::Text: {
					textVector << reader->readElementText(); // old serialization format with row by row for xmlVersion < 18
					break;
				}
				}
			} else { // unknown element
				reader->raiseUnknownElementWarning();
				if (!reader->skipToEndElement())
					return false;
			}
			if (!ret_val)
				return false;
		}

		if (!preview) {
			// Decode data
			QString content = reader->text().toString().trimmed();
			// Datetime and Text for xmlVersion < 18 are read row by row above,
			// everything else is Base64 encoded and is decoded via DecodeColumnTask
			const auto mode = columnMode();
			if (!content.isEmpty() && (mode == ColumnMode::Double || mode == ColumnMode::Integer || mode == ColumnMode::BigInt || mode == ColumnMode::Text)) {
				auto* task = new DecodeColumnTask(d, content);
				QThreadPool::globalInstance()->start(task);
			}
		}
	}

	switch (columnMode()) {
	case AbstractColumn::ColumnMode::Double:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Integer:
		/* handled above in DecodeColumnTask */
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		setDateTimes(dateTimeVector);
		break;
	case AbstractColumn::ColumnMode::Text:
		if (!textVector.isEmpty() && Project::xmlVersion() < 18) // old serialization format with row by row reading
			setText(textVector);
		break;
	}

	return !reader->error();
}

void Column::finalizeLoad() {
	d->finalizeLoad();
}

/**
 * \brief Read XML input filter element
 */
bool Column::XmlReadInputFilter(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() == true && reader->name() == QLatin1String("input_filter"));
	if (!reader->skipToNextTag())
		return false;
	if (!d->inputFilter()->load(reader, false))
		return false;
	if (!reader->skipToNextTag())
		return false;
	Q_ASSERT(reader->isEndElement() == true && reader->name() == QLatin1String("input_filter"));
	return true;
}

/**
 * \brief Read XML output filter element
 */
bool Column::XmlReadOutputFilter(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() == true && reader->name() == QLatin1String("output_filter"));
	if (!reader->skipToNextTag())
		return false;
	if (!d->outputFilter()->load(reader, false))
		return false;
	if (!reader->skipToNextTag())
		return false;
	Q_ASSERT(reader->isEndElement() == true && reader->name() == QLatin1String("output_filter"));
	return true;
}

/**
 * \brief Read XML formula element
 */
bool Column::XmlReadFormula(XmlStreamReader* reader) {
	QString formula;
	QStringList variableNames;
	QStringList columnPathes;

	// read the autoUpdate attribute if available (older project files created with <2.8 don't have it)
	bool autoUpdate = false;
	if (reader->attributes().hasAttribute(QStringLiteral("autoUpdate")))
		autoUpdate = reader->attributes().value(QStringLiteral("autoUpdate")).toInt();

	// read the autoResize attribute if available (older project files created with <2.11 don't have it)
	bool autoResize = false;
	if (reader->attributes().hasAttribute(QStringLiteral("autoResize")))
		autoResize = reader->attributes().value(QStringLiteral("autoResize")).toInt();

	while (reader->readNext()) {
		if (reader->isEndElement())
			break;

		if (reader->name() == QLatin1String("text"))
			formula = reader->readElementText();
		else if (reader->name() == QLatin1String("variableNames")) {
			while (reader->readNext()) {
				if (reader->name() == QLatin1String("variableNames") && reader->isEndElement())
					break;

				if (reader->isStartElement())
					variableNames << reader->readElementText();
			}
		} else if (reader->name() == QLatin1String("columnPathes")) {
			while (reader->readNext()) {
				if (reader->name() == QLatin1String("columnPathes") && reader->isEndElement())
					break;

				if (reader->isStartElement())
					columnPathes << reader->readElementText();
			}
		}
	}

	d->setFormula(formula, variableNames, columnPathes, autoUpdate, autoResize);

	return true;
}

// TODO: read cell formula, not implemented yet
//  bool Column::XmlReadFormula(XmlStreamReader* reader)
//  {
//  	Q_ASSERT(reader->isStartElement() && reader->name() == QLatin1String("formula"));
//
//  	bool ok1, ok2;
//  	int start, end;
//  	start = reader->readAttributeInt("start_row", &ok1);
//  	end = reader->readAttributeInt("end_row", &ok2);
//  	if (!ok1 || !ok2)
//  	{
//  		reader->raiseError(i18n("invalid or missing start or end row"));
//  		return false;
//  	}
//  	setFormula(Interval<int>(start,end), reader->readElementText());
//
//  	return true;
//  }

/**
 * \brief Read XML row element
 */
bool Column::XmlReadRow(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() == true && reader->name() == QLatin1String("row"));

	//	QXmlStreamAttributes attribs = reader->attributes();

	bool ok;
	int index = reader->readAttributeInt(QStringLiteral("index"), &ok);
	if (!ok) {
		reader->raiseError(i18n("invalid or missing row index"));
		return false;
	}

	QString str = reader->readElementText();
	switch (columnMode()) {
	case ColumnMode::Double: {
		double value = str.toDouble(&ok);
		if (!ok) {
			reader->raiseError(i18n("invalid row value"));
			return false;
		}
		setValueAt(index, value);
		break;
	}
	case ColumnMode::Integer: {
		int value = str.toInt(&ok);
		if (!ok) {
			reader->raiseError(i18n("invalid row value"));
			return false;
		}
		setIntegerAt(index, value);
		break;
	}
	case ColumnMode::BigInt: {
		qint64 value = str.toLongLong(&ok);
		if (!ok) {
			reader->raiseError(i18n("invalid row value"));
			return false;
		}
		setBigIntAt(index, value);
		break;
	}
	case ColumnMode::Text:
		setTextAt(index, str);
		break;

	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day:
		// Same as in the Variable Parser. UTC must be used, otherwise
		// some dates are not valid. For example (2017-03-26)
		QDateTime date_time =
			QDateTime::fromString(str + QStringLiteral("Z"), QStringLiteral("yyyy-dd-MM hh:mm:ss:zzzt")); // last t is important. It is the timezone
		setDateTimeAt(index, date_time);
		break;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return whether the object is read-only
 */
bool Column::isReadOnly() const {
	return false;
}

/**
 * \brief Return the column mode
 *
 * This function is mostly used by spreadsheets but can also be used
 * by plots. The column mode specifies how to interpret
 * the values in the column additional to the data type.
 */
AbstractColumn::ColumnMode Column::columnMode() const {
	return d->columnMode();
}

QString Column::columnModeString() const {
	return AbstractColumn::columnModeString(d->columnMode());
}

void Column::resizeTo(int rows) {
	d->resizeTo(rows);
}
/**
 * \brief Return the data vector size
 */
int Column::rowCount() const {
	return d->rowCount();
}

int Column::rowCount(double min, double max) const {
	const auto p = properties();
	if (p == Properties::MonotonicIncreasing || p == Properties::MonotonicDecreasing) {
		int start, end;
		if (!indicesMinMax(min, max, start, end))
			return 0;
		return abs(start - end) + 1; // +1 because start/end is included
	} else if (p == Properties::Constant)
		return rowCount();

	return d->rowCount(min, max);
}

/**
 * \brief Return the number of available data rows
 *
 * This returns the number of rows that actually contain data.
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int Column::availableRowCount(int max) const {
	return d->availableRowCount(max);
}

/**
 * \brief Return the column plot designation
 */
AbstractColumn::PlotDesignation Column::plotDesignation() const {
	return d->plotDesignation();
}

QString Column::plotDesignationString(bool withBrackets) const {
	return AbstractColumn::plotDesignationString(d->plotDesignation(), withBrackets);
}

AbstractSimpleFilter* Column::outputFilter() const {
	return d->outputFilter();
}

/**
 * \brief Return a wrapper column object used for String I/O.
 */
ColumnStringIO* Column::asStringColumn() const {
	return m_string_io;
}

////////////////////////////////////////////////////////////////////////////////
//! \name IntervalAttribute related functions
//@{
////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Return the formula associated with row 'row'
 */
QString Column::formula(int row) const {
	return d->formula(row);
}

/**
 * \brief Return the intervals that have associated formulas
 *
 * This can be used to make a list of formulas with their intervals.
 * Here is some example code:
 *
 * \code
 * QStringList list;
 * QVector< Interval<int> > intervals = my_column.formulaIntervals();
 * foreach(Interval<int> interval, intervals)
 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
 * \endcode
 */
QVector<Interval<int>> Column::formulaIntervals() const {
	return d->formulaIntervals();
}

void Column::handleFormatChange() {
	DEBUG(Q_FUNC_INFO << ", mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode()));
	if (columnMode() == ColumnMode::DateTime) {
		auto* input_filter = static_cast<String2DateTimeFilter*>(d->inputFilter());
		auto* output_filter = static_cast<DateTime2StringFilter*>(d->outputFilter());
		DEBUG(Q_FUNC_INFO << ", change format " << STDSTRING(input_filter->format()) << " to " << STDSTRING(output_filter->format()));
		input_filter->setFormat(output_filter->format());
	}

	Q_EMIT aspectDescriptionChanged(this); // the icon for the type changed
	if (!d->m_suppressDataChangedSignal)
		Q_EMIT formatChanged(this); // all cells must be repainted

	d->available.setUnavailable();
}

/*!
 * calculates the minimal value in the column.
 * for \c count = 0, the minimum of all elements is returned.
 * for \c count > 0, the minimum of the first \p count elements is returned.
 * for \c count < 0, the minimum of the last \p count elements is returned.
 */
double Column::minimum(int count) const {
	if (count == 0 && d->available.min)
		return d->statistics.minimum;
	else {
		int startIndex = 0, endIndex = rowCount() - 1;

		if (count > 0)
			endIndex = std::min(rowCount() - 1, count - 1);
		else if (count < 0)
			startIndex = std::max(rowCount() - count, 0);

		return minimum(startIndex, endIndex);
	}
}

/*!
 * \brief Column::minimum
 * Calculates the minimum value in the column between the \p startIndex and \p endIndex, endIndex is excluded.
 * If startIndex is greater than endIndex the indices are swapped
 * \p startIndex
 * \p endIndex
 */
double Column::minimum(int startIndex, int endIndex) const {
#ifdef PERFTRACE_AUTOSCALE
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
#endif
	double min = INFINITY;

	if (rowCount() == 0)
		return min;

	if (startIndex > endIndex && startIndex >= 0 && endIndex >= 0)
		std::swap(startIndex, endIndex);

	startIndex = std::max(startIndex, 0);
	endIndex = std::max(endIndex, 0);

	startIndex = std::min(startIndex, rowCount() - 1);
	endIndex = std::min(endIndex, rowCount() - 1);

	if (startIndex == 0 && endIndex == rowCount() - 1 && d->available.min)
		return d->statistics.minimum;

	ColumnMode mode = columnMode();
	Properties property = properties();
	if (property == Properties::No || property == Properties::NonMonotonic) {
		// skipping values is only in Properties::No needed, because
		// when there are invalid values the property must be Properties::No
		switch (mode) {
		case ColumnMode::Double: {
			auto* vec = static_cast<QVector<double>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const double val = vec->at(row);
				if (std::isnan(val))
					continue;

				if (val < min)
					min = val;
			}
			break;
		}
		case ColumnMode::Integer: {
			auto* vec = static_cast<QVector<int>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const int val = vec->at(row);

				if (val < min)
					min = val;
			}
			break;
		}
		case ColumnMode::BigInt: {
			auto* vec = static_cast<QVector<qint64>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const qint64 val = vec->at(row);

				if (val < min)
					min = val;
			}
			break;
		}
		case ColumnMode::Text:
			break;
		case ColumnMode::DateTime: {
			auto* vec = static_cast<QVector<QDateTime>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const qint64 val = vec->at(row).toMSecsSinceEpoch();

				if (val < min)
					min = val;
			}
			break;
		}
		case ColumnMode::Day:
		case ColumnMode::Month:
			break;
		}
	} else { // monotonic: use the properties knowledge to determine maximum faster
		int foundIndex = 0;
		if (property == Properties::Constant || property == Properties::MonotonicIncreasing)
			foundIndex = startIndex;
		else if (property == Properties::MonotonicDecreasing) {
			foundIndex = endIndex;
			foundIndex = std::max(0, foundIndex);
		}

		switch (mode) {
		case ColumnMode::Double:
		case ColumnMode::Integer:
		case ColumnMode::BigInt:
			min = valueAt(foundIndex);
			break;
		case ColumnMode::DateTime:
		case ColumnMode::Month:
		case ColumnMode::Day:
			min = dateTimeAt(foundIndex).toMSecsSinceEpoch();
			break;
		case ColumnMode::Text:
			break;
		}
	}

	if (startIndex == 0 && endIndex == rowCount() - 1) {
		d->available.min = true;
		d->statistics.minimum = min;
	}

	return min;
}

/*!
 * calculates the maximal value in the column.
 * for \c count = 0, the maximum of all elements is returned.
 * for \c count > 0, the maximum of the first \p count elements is returned.
 * for \c count < 0, the maximum of the last \p count elements is returned.
 */
double Column::maximum(int count) const {
#ifdef PERFTRACE_AUTOSCALE
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
#endif
	if (count == 0 && d->available.max)
		return d->statistics.maximum;
	else {
		int startIndex = 0, endIndex = rowCount() - 1;

		if (count > 0)
			endIndex = std::min(rowCount() - 1, count - 1);
		else if (count < 0)
			startIndex = std::max(rowCount() - count, 0);

		return maximum(startIndex, endIndex);
	}
}

/*!
 * \brief Column::maximum
 * Calculates the maximum value in the column between the \p startIndex and \p endIndex.
 * If startIndex is greater than endIndex the indices are swapped
 * \p startIndex
 * \p endIndex
 */
double Column::maximum(int startIndex, int endIndex) const {
	double max = -INFINITY;
	if (rowCount() == 0)
		return max;

	if (startIndex > endIndex && startIndex >= 0 && endIndex >= 0)
		std::swap(startIndex, endIndex);

	startIndex = std::max(startIndex, 0);
	endIndex = std::max(endIndex, 0);

	startIndex = std::min(startIndex, rowCount() - 1);
	endIndex = std::min(endIndex, rowCount() - 1);

	if (startIndex == 0 && endIndex == rowCount() - 1 && d->available.max)
		return d->statistics.maximum;

	ColumnMode mode = columnMode();
	Properties property = properties();
	if (property == Properties::No || property == Properties::NonMonotonic) {
		switch (mode) {
		case ColumnMode::Double: {
			auto* vec = static_cast<QVector<double>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;
				const double val = vec->at(row);
				if (std::isnan(val))
					continue;

				if (val > max)
					max = val;
			}
			break;
		}
		case ColumnMode::Integer: {
			auto* vec = static_cast<QVector<int>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;
				const int val = vec->at(row);

				if (val > max)
					max = val;
			}
			break;
		}
		case ColumnMode::BigInt: {
			auto* vec = static_cast<QVector<qint64>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;
				const qint64 val = vec->at(row);

				if (val > max)
					max = val;
			}
			break;
		}
		case ColumnMode::Text:
			break;
		case ColumnMode::DateTime: {
			auto* vec = static_cast<QVector<QDateTime>*>(data());
			for (int row = startIndex; row <= endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;
				const qint64 val = vec->at(row).toMSecsSinceEpoch();

				if (val > max)
					max = val;
			}
			break;
		}
		case ColumnMode::Day:
		case ColumnMode::Month:
			break;
		}
	} else { // monotonic: use the properties knowledge to determine maximum faster
		int foundIndex = 0;
		if (property == Properties::Constant || property == Properties::MonotonicDecreasing)
			foundIndex = startIndex;
		else if (property == Properties::MonotonicIncreasing) {
			foundIndex = endIndex;
			foundIndex = std::max(0, foundIndex);
		}

		switch (mode) {
		case ColumnMode::Double:
		case ColumnMode::Integer:
		case ColumnMode::BigInt:
			max = valueAt(foundIndex);
			break;
		case ColumnMode::DateTime:
		case ColumnMode::Month:
		case ColumnMode::Day:
			max = dateTimeAt(foundIndex).toMSecsSinceEpoch();
			break;
		case ColumnMode::Text:
			break;
		}
	}

	if (startIndex == 0 && endIndex == rowCount() - 1) {
		d->statistics.maximum = max;
		d->available.max = true;
	}
	return max;
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int Column::indexForValue(double x, QVector<double>& column, Properties properties, bool smaller) {
	return ColumnPrivate::indexForValue(x, column, properties, smaller);
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int Column::indexForValue(const double x, const QVector<QPointF>& points, Properties properties, bool smaller) {
	return ColumnPrivate::indexForValue(x, points, properties, smaller);
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int Column::indexForValue(double x, QVector<QLineF>& lines, Properties properties, bool smaller) {
	return ColumnPrivate::indexForValue(x, lines, properties, smaller);
}

int Column::indexForValue(double x, bool smaller) const {
	return d->indexForValue(x, smaller);
}

/*!
 * Finds the minimal and maximal index which are between v1 and v2
 * \brief Column::indicesMinMax
 * \param v1
 * \param v2
 * \param start
 * \param end
 * \return
 */
bool Column::indicesMinMax(double v1, double v2, int& start, int& end) const {
	// DEBUG(Q_FUNC_INFO << ", values = " << v1 << " .. " << v2)
	start = -1;
	end = -1;
	const int rowCount = this->rowCount();
	if (rowCount == 0)
		return false;

	// Assumption: v1 is always the smaller value
	if (v1 > v2)
		qSwap(v1, v2);

	const auto& property = properties();
	if (property == Properties::MonotonicIncreasing || property == Properties::MonotonicDecreasing) {
		start = indexForValue(v1, true);
		end = indexForValue(v2, false);

		switch (columnMode()) {
		case ColumnMode::Integer:
		case ColumnMode::BigInt:
		case ColumnMode::Double: {
			if (property == Properties::MonotonicIncreasing) {
				if (start > 0 && valueAt(start - 1) <= v2 && valueAt(start - 1) >= v1)
					start--;
				if (end < rowCount - 1 && valueAt(end + 1) <= v2 && valueAt(end + 1) >= v1)
					end++;
			} else {
				if (end > 0 && valueAt(end - 1) <= v2 && valueAt(end - 1) >= v1)
					end--;
				if (start < rowCount - 1 && valueAt(start + 1) <= v2 && valueAt(start + 1) >= v1)
					start++;
			}

			break;
		}
		case ColumnMode::DateTime:
		case ColumnMode::Month:
		case ColumnMode::Day: {
			qint64 v1int64 = v1;
			qint64 v2int64 = v2;
			qint64 value;
			if (property == Properties::MonotonicIncreasing) {
				if (start > 0) {
					value = dateTimeAt(start - 1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						start--;
				}

				if (end > rowCount - 1) {
					value = dateTimeAt(end + 1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						end++;
				}
			} else {
				if (end > 0) {
					value = dateTimeAt(end - 1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						end--;
				}

				if (start > rowCount - 1) {
					value = dateTimeAt(start + 1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						start++;
				}
			}
			break;
		}
		case ColumnMode::Text:
			return false;
		}
		// DEBUG("monotonic start/end = " << start << "/" << end)

		return true;
	} else if (property == Properties::Constant) {
		start = 0;
		end = rowCount - 1;
		return true;
	}
	// property == Properties::No || AbstractColumn::Properties::NonMonotonic
	switch (columnMode()) {
	case ColumnMode::Integer:
	case ColumnMode::BigInt:
	case ColumnMode::Double: {
		double value;
		for (int i = 0; i < rowCount; i++) {
			if (!isValid(i) || isMasked(i))
				continue;
			value = valueAt(i);
			if (value <= v2 && value >= v1) {
				end = i;
				if (start < 0)
					start = i;
			}
		}
		break;
	}
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day: {
		qint64 value;
		qint64 v2int64 = v2;
		qint64 v1int64 = v1;
		for (int i = 0; i < rowCount; i++) {
			if (!isValid(i) || isMasked(i))
				continue;
			value = dateTimeAt(i).toMSecsSinceEpoch();
			if (value <= v2int64 && value >= v1int64) {
				end = i;
				if (start < 0)
					start = i;
			}
		}
		break;
	}
	case ColumnMode::Text:
		return false;
	}
	// DEBUG("non-monotonic start/end = " << start << "/" << end)

	return true;
}

AbstractColumn::ColumnMode Column::labelsMode() const {
	return d->m_labels.mode();
}

void Column::handleAspectUpdated(const QString&, const AbstractAspect* aspect) {
	d->formulaVariableColumnAdded(aspect);
}
