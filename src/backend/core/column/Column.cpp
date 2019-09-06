/***************************************************************************
    File                 : Column.cpp
    Project              : LabPlot
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2013-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/column/columncommands.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include <gsl/gsl_sort.h>
}

#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QMenu>
#include <QThreadPool>

#include <KLocalizedString>

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
	: AbstractColumn(name, AspectType::Column), d(new ColumnPrivate(this, mode)) {

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
	addChild(d->inputFilter());
	addChild(d->outputFilter());
	m_suppressDataChangedSignal = false;

	m_usedInActionGroup = new QActionGroup(this);
	connect(m_usedInActionGroup, &QActionGroup::triggered, this, &Column::navigateTo);
	connect(this, &AbstractColumn::maskingChanged, this, [=]{d->propertiesAvailable = false;});
}

Column::~Column() {
	delete m_string_io;
	delete d;
}

QMenu* Column::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();
	QAction* firstAction{nullptr};

	 //insert after "rename" and "delete" actions, if available.
	 //MQTTTopic columns don't have these actions
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	//add actions available in SpreadsheetView
	//TODO: we don't need to add anything from the view for MQTTTopic columns.
	//at the moment it's ok to check to the null pointer for firstAction here.
	//later, once we have some actions in the menu also for MQTT topics we'll
	//need to explicitly to dynamic_cast for MQTTTopic
	if (firstAction)
		emit requestProjectContextMenu(menu);

	//"Used in" menu containing all curves where the column is used
	QMenu* usedInMenu = new QMenu(i18n("Used in"));
	usedInMenu->setIcon(QIcon::fromTheme("go-next-view"));
	usedInMenu->addSection(i18n("Curves"));

	//remove previously added actions
	for (auto* action : m_usedInActionGroup->actions())
		m_usedInActionGroup->removeAction(action);

	Project* project = this->project();

	//add curves where the column is currently in use
	usedInMenu->addSection(i18n("XY-Curves"));
	auto curves = project->children<XYCurve>(AbstractAspect::Recursive);
	for (const auto* curve : curves) {
		bool used = false;

		const auto* analysisCurve = dynamic_cast<const XYAnalysisCurve*>(curve);
		if (analysisCurve) {
			if (analysisCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet
					&& (analysisCurve->xDataColumn() == this || analysisCurve->yDataColumn() == this || analysisCurve->y2DataColumn() == this) )
				used = true;
		} else {
			if (curve->xColumn() == this || curve->yColumn() == this)
				used = true;
		}

		if (used) {
			QAction* action = new QAction(curve->icon(), curve->name(), m_usedInActionGroup);
			action->setData(curve->path());
			usedInMenu->addAction(action);
		}
	}

	//add histograms where the column is used
	usedInMenu->addSection(i18n("Histograms"));
	auto hists = project->children<Histogram>(AbstractAspect::Recursive);
	for (const auto* hist : hists) {
		bool used = (hist->dataColumn() == this);
		if (used) {
			QAction* action = new QAction(hist->icon(), hist->name(), m_usedInActionGroup);
			action->setData(hist->path());
			usedInMenu->addAction(action);
		}
	}

	//add calculated columns where the column is used in formula variables
	usedInMenu->addSection(i18n("Calculated Columns"));
	QVector<Column*> columns = project->children<Column>(AbstractAspect::Recursive);
	const QString& path = this->path();
	for (const auto* column : columns) {
		auto paths = column->formulaVariableColumnPaths();
		if (paths.indexOf(path) != -1) {
			QAction* action = new QAction(column->icon(), column->name(), m_usedInActionGroup);
			action->setData(column->path());
			usedInMenu->addAction(action);
		}
	}


	if (firstAction)
		menu->insertSeparator(firstAction);

	menu->insertMenu(firstAction, usedInMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

void Column::navigateTo(QAction* action) {
	project()->navigateTo(action->data().toString());
}

/*!
 *
 */
void Column::setSuppressDataChangedSignal(bool b) {
	m_suppressDataChangedSignal = b;
}

void Column::addUsedInPlots(QVector<CartesianPlot*>& plots) {
	const Project* project = this->project();

	//when executing tests we don't create any project,
	//add a null-pointer check for tests here.
	if (!project)
		return;

	QVector<const XYCurve*> curves = project->children<const XYCurve>(AbstractAspect::Recursive);

	//determine the plots where the column is consumed
	for (const auto* curve : curves) {
		if (curve->xColumn() == this || curve->yColumn() == this
			|| (curve->xErrorType() == XYCurve::SymmetricError && curve->xErrorPlusColumn() == this)
			|| (curve->xErrorType() == XYCurve::AsymmetricError && (curve->xErrorPlusColumn() == this ||curve->xErrorMinusColumn() == this))
			|| (curve->yErrorType() == XYCurve::SymmetricError && curve->yErrorPlusColumn() == this)
			|| (curve->yErrorType() == XYCurve::AsymmetricError && (curve->yErrorPlusColumn() == this ||curve->yErrorMinusColumn() == this)) ) {
			auto* plot = dynamic_cast<CartesianPlot*>(curve->parentAspect());
			if (plots.indexOf(plot) == -1)
				plots << plot;
		}
	}

	QVector<const Histogram*> hists = project->children<const Histogram>(AbstractAspect::Recursive);
	for (const auto* hist : hists) {
		if (hist->dataColumn() == this ) {
			auto* plot = dynamic_cast<CartesianPlot*>(hist->parentAspect());
			if (plots.indexOf(plot) == -1)
				plots << plot;
		}
	}
}

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if
 * necessary, converts it to another datatype.
 */
void Column::setColumnMode(AbstractColumn::ColumnMode mode) {
	if (mode == columnMode())
		return;

	DEBUG("Column::setColumnMode()");
	beginMacro(i18n("%1: change column type", name()));

	auto* old_input_filter = d->inputFilter();
	auto* old_output_filter = d->outputFilter();
	exec(new ColumnSetModeCmd(d, mode));

	if (d->inputFilter() != old_input_filter) {
		removeChild(old_input_filter);
		addChild(d->inputFilter());
		d->inputFilter()->input(0, m_string_io);
	}
	if (d->outputFilter() != old_output_filter) {
		removeChild(old_output_filter);
		addChild(d->outputFilter());
		d->outputFilter()->input(0, this);
	}

	endMacro();
	DEBUG("Column::setColumnMode() DONE");
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
	if (other->columnMode() != columnMode()) return false;
	exec(new ColumnFullCopyCmd(d, other));
	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool Column::copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) {
	Q_CHECK_PTR(source);
	if (source->columnMode() != columnMode()) return false;
	exec(new ColumnPartialCopyCmd(d, source, source_start, dest_start, num_rows));
	return true;
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void Column::handleRowInsertion(int before, int count) {
	AbstractColumn::handleRowInsertion(before, count);
	exec(new ColumnInsertRowsCmd(d, before, count));
	if (!m_suppressDataChangedSignal)
		emit dataChanged(this);

	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void Column::handleRowRemoval(int first, int count) {
	AbstractColumn::handleRowRemoval(first, count);
	exec(new ColumnRemoveRowsCmd(d, first, count));
	if (!m_suppressDataChangedSignal)
		emit dataChanged(this);

	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
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
 * \brief Clear the whole column
 */
void Column::clear() {
	exec(new ColumnClearCmd(d));
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
QString Column:: formula() const {
	return d->formula();
}

const QStringList& Column::formulaVariableNames() const {
	return d->formulaVariableNames();
}

const QVector<Column*>& Column::formulaVariableColumns() const {
	return d->formulaVariableColumns();
}

const QStringList& Column::formulaVariableColumnPaths() const {
	return d->formulaVariableColumnPaths();
}

void Column::setformulVariableColumnsPath(int index, const QString path) {
	d->setformulVariableColumnsPath(index, path);
}

void Column::setformulVariableColumn(int index, Column* column) {
	d->setformulVariableColumn(index, column);
}

bool Column::formulaAutoUpdate() const {
	return d->formulaAutoUpdate();
}

/**
 * \brief Sets the formula used to generate column values
 */
void Column::setFormula(const QString& formula, const QStringList& variableNames,
						const QVector<Column*>& columns, bool autoUpdate) {
	exec(new ColumnSetGlobalFormulaCmd(d, formula, variableNames, columns, autoUpdate));
}

/*!
 * in case the cell values are calculated via a global column formula,
 * updates the values on data changes in all the dependent changes in the
 * "variable columns".
 */
void Column::updateFormula() {
	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
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
	DEBUG("Column::setTextAt()");
	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
	exec(new ColumnSetTextCmd(d, row, new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void Column::replaceTexts(int first, const QVector<QString>& new_values) {
	DEBUG("Column::replaceTexts()");
	if (!new_values.isEmpty()) { //TODO: do we really need this check?
		d->statisticsAvailable = false;
		d->hasValuesAvailable = false;
		d->propertiesAvailable = false;
		exec(new ColumnReplaceTextsCmd(d, first, new_values));
	}
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
	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
	exec(new ColumnSetDateTimeCmd(d, row, new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
	if (!new_values.isEmpty()) {
		d->statisticsAvailable = false;
		d->hasValuesAvailable = false;
		d->propertiesAvailable = false;
		exec(new ColumnReplaceDateTimesCmd(d, first, new_values));
	}
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void Column::setValueAt(int row, const double new_value) {
// 	DEBUG("Column::setValueAt()");
	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
	exec(new ColumnSetValueCmd(d, row, new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void Column::replaceValues(int first, const QVector<double>& new_values) {
	DEBUG("Column::replaceValues()");
	if (!new_values.isEmpty()) {
		d->statisticsAvailable = false;
		d->hasValuesAvailable = false;
		d->propertiesAvailable = false;
		exec(new ColumnReplaceValuesCmd(d, first, new_values));
	}
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void Column::setIntegerAt(int row, const int new_value) {
	DEBUG("Column::setIntegerAt()");
	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
	d->propertiesAvailable = false;
	exec(new ColumnSetIntegerCmd(d, row, new_value));
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void Column::replaceInteger(int first, const QVector<int>& new_values) {
	DEBUG("Column::replaceInteger()");
	if (!new_values.isEmpty()) {
		d->statisticsAvailable = false;
		d->hasValuesAvailable = false;
		d->propertiesAvailable = false;
		exec(new ColumnReplaceIntegersCmd(d, first, new_values));
	}
}
/*!
 * \brief Column::properties
 * Returns the column properties of this curve (monoton increasing, monoton decreasing, ... )
 * \see AbstractColumn::properties
 */
AbstractColumn::Properties Column::properties() const{
	if (!d->propertiesAvailable)
		d->updateProperties();

	return d->properties;
}

const Column::ColumnStatistics& Column::statistics() const {
	if (!d->statisticsAvailable)
		calculateStatistics();

	return d->statistics;
}

void Column::calculateStatistics() const {
	d->statistics = ColumnStatistics();
	ColumnStatistics& statistics = d->statistics;

	// TODO: support other data types?
	auto* rowValues = reinterpret_cast<QVector<double>*>(data());

	size_t notNanCount = 0;
	double val;
	double columnSum = 0.0;
	double columnProduct = 1.0;
	double columnSumNeg = 0.0;
	double columnSumSquare = 0.0;
	statistics.minimum = INFINITY;
	statistics.maximum = -INFINITY;
	QMap<double, int> frequencyOfValues;
	QVector<double> rowData;
	rowData.reserve(rowValues->size());
	for (int row = 0; row < rowValues->size(); ++row) {
		val = rowValues->value(row);
		if (std::isnan(val) || isMasked(row))
			continue;

		if (val < statistics.minimum)
			statistics.minimum = val;
		if (val > statistics.maximum)
			statistics.maximum = val;
		columnSum+= val;
		columnSumNeg += (1.0 / val);
		columnSumSquare += pow(val, 2.0);
		columnProduct *= val;
		if (frequencyOfValues.contains(val))
			frequencyOfValues.operator [](val)++;
		else
			frequencyOfValues.insert(val, 1);
		++notNanCount;
		rowData.push_back(val);
	}

	if (notNanCount == 0) {
		d->statisticsAvailable = true;
		return;
	}

	if (rowData.size() < rowValues->size())
		rowData.squeeze();

	statistics.arithmeticMean = columnSum / notNanCount;
	statistics.geometricMean = pow(columnProduct, 1.0 / notNanCount);
	statistics.harmonicMean = notNanCount / columnSumNeg;
	statistics.contraharmonicMean = columnSumSquare / columnSum;

	double columnSumVariance = 0;
	double columnSumMeanDeviation = 0.0;
	double columnSumMedianDeviation = 0.0;
	double sumForCentralMoment_r3 = 0.0;
	double sumForCentralMoment_r4 = 0.0;

	gsl_sort(rowData.data(), 1, notNanCount);
	statistics.median = (notNanCount%2) ? rowData.at((int)((notNanCount-1)/2)) :
	                    (rowData.at((int)((notNanCount-1)/2)) + rowData.at((int)(notNanCount/2)))/2.0;
	QVector<double> absoluteMedianList;
	absoluteMedianList.reserve((int)notNanCount);
	absoluteMedianList.resize((int)notNanCount);

	int idx = 0;
	for (int row = 0; row < rowValues->size(); ++row) {
		val = rowValues->value(row);
		if (std::isnan(val) || isMasked(row) )
			continue;
		columnSumVariance += pow(val - statistics.arithmeticMean, 2.0);

		sumForCentralMoment_r3 += pow(val - statistics.arithmeticMean, 3.0);
		sumForCentralMoment_r4 += pow(val - statistics.arithmeticMean, 4.0);
		columnSumMeanDeviation += fabs( val - statistics.arithmeticMean );

		absoluteMedianList[idx] = fabs(val - statistics.median);
		columnSumMedianDeviation += absoluteMedianList[idx];
		idx++;
	}

	statistics.meanDeviationAroundMedian = columnSumMedianDeviation / notNanCount;
	statistics.medianDeviation = (notNanCount%2) ? absoluteMedianList.at((int)((notNanCount-1)/2)) :
	                             (absoluteMedianList.at((int)((notNanCount-1)/2)) + absoluteMedianList.at((int)(notNanCount/2)))/2.0;

	const double centralMoment_r3 = sumForCentralMoment_r3 / notNanCount;
	const double centralMoment_r4 = sumForCentralMoment_r4 / notNanCount;

	statistics.variance = columnSumVariance / notNanCount;
	statistics.standardDeviation = sqrt(statistics.variance);
	statistics.skewness = centralMoment_r3 / pow(statistics.standardDeviation, 3.0);
	statistics.kurtosis = (centralMoment_r4 / pow(statistics.standardDeviation, 4.0)) - 3.0;
	statistics.meanDeviation = columnSumMeanDeviation / notNanCount;

	double entropy = 0.0;
	for (const auto& v : frequencyOfValues) {
		const double frequencyNorm = static_cast<double>(v) / notNanCount;
		entropy += (frequencyNorm * log2(frequencyNorm));
	}

	statistics.entropy = -entropy;

	d->statisticsAvailable = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void* Column::data() const {
	return d->data();
}

/*!
 * return \c true if the column has numeric values, \false otherwise.
 */
bool Column::hasValues() const {
	if (d->hasValuesAvailable)
		return d->hasValues;

	bool foundValues = false;
	if (columnMode() == AbstractColumn::Numeric) {
		for (int row = 0; row < rowCount(); ++row) {
			if (!std::isnan(valueAt(row))) {
				foundValues = true;
				break;
			}
		}
	} else if (columnMode() == AbstractColumn::Integer) {
		//integer column has always valid values
		foundValues = true;
	} else if (columnMode() == AbstractColumn::DateTime) {
		for (int row = 0; row < rowCount(); ++row) {
			if (dateTimeAt(row).isValid()) {
				foundValues = true;
				break;
			}
		}
	}

	d->hasValues = foundValues;
	d->hasValuesAvailable = true;
	return d->hasValues;
}

//TODO: support all data types
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

/*
 * call this function if the data of the column was changed directly via the data()-pointer
 * and not via the setValueAt() in order to emit the dataChanged-signal.
 * This is used e.g. in \c XYFitCurvePrivate::recalculate()
 */
void Column::setChanged() {
    d->propertiesAvailable = false;

	if (!m_suppressDataChangedSignal)
		emit dataChanged(this);

	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return an icon to be used for decorating the views and spreadsheet column headers
 */
QIcon Column::icon() const {
	return iconForMode(columnMode());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Save the column as XML
 */
void Column::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("column");
	writeBasicAttributes(writer);

	writer->writeAttribute("rows", QString::number(rowCount()));
	writer->writeAttribute("designation", QString::number(plotDesignation()));
	writer->writeAttribute("mode", QString::number(columnMode()));
	writer->writeAttribute("width", QString::number(width()));

	//save the formula used to generate column values, if available
	if (!formula().isEmpty() ) {
		writer->writeStartElement("formula");
		writer->writeAttribute("autoUpdate", QString::number(d->formulaAutoUpdate()));
		writer->writeTextElement("text", formula());

		writer->writeStartElement("variableNames");
		for (const auto& name : formulaVariableNames())
			writer->writeTextElement("name", name);
		writer->writeEndElement();

		writer->writeStartElement("columnPathes");
		for (const auto path : formulaVariableColumnPaths())
			writer->writeTextElement("path", path);
		writer->writeEndElement();

		writer->writeEndElement();
	}

	writeCommentElement(writer);

	writer->writeStartElement("input_filter");
	d->inputFilter()->save(writer);
	writer->writeEndElement();

	writer->writeStartElement("output_filter");
	d->outputFilter()->save(writer);
	writer->writeEndElement();

	XmlWriteMask(writer);

	//TODO: formula in cells is not implemented yet
	// 	QVector< Interval<int> > formulas = formulaIntervals();
	// 	foreach(const Interval<int>& interval, formulas) {
	// 		writer->writeStartElement("formula");
	// 		writer->writeAttribute("start_row", QString::number(interval.start()));
	// 		writer->writeAttribute("end_row", QString::number(interval.end()));
	// 		writer->writeCharacters(formula(interval.start()));
	// 		writer->writeEndElement();
	// 	}

	int i;
	switch (columnMode()) {
	case AbstractColumn::Numeric: {
			const char* data = reinterpret_cast<const char*>(static_cast< QVector<double>* >(d->data())->constData());
			size_t size = d->rowCount() * sizeof(double);
			writer->writeCharacters(QByteArray::fromRawData(data, (int)size).toBase64());
			break;
		}
	case AbstractColumn::Integer: {
			const char* data = reinterpret_cast<const char*>(static_cast< QVector<int>* >(d->data())->constData());
			size_t size = d->rowCount() * sizeof(int);
			writer->writeCharacters(QByteArray::fromRawData(data, (int)size).toBase64());
			break;
		}
	case AbstractColumn::Text:
		for (i = 0; i < rowCount(); ++i) {
			writer->writeStartElement("row");
			writer->writeAttribute("index", QString::number(i));
			writer->writeCharacters(textAt(i));
			writer->writeEndElement();
		}
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for (i = 0; i < rowCount(); ++i) {
			writer->writeStartElement("row");
			writer->writeAttribute("index", QString::number(i));
			writer->writeCharacters(dateTimeAt(i).toString("yyyy-dd-MM hh:mm:ss:zzz"));
			writer->writeEndElement();
		}
		break;
	}

	writer->writeEndElement(); // "column"
}

//TODO: extra header
class DecodeColumnTask : public QRunnable {
public:
	DecodeColumnTask(ColumnPrivate* priv, const QString& content) {
		m_private = priv;
		m_content = content;
	};
	void run() override {
		QByteArray bytes = QByteArray::fromBase64(m_content.toLatin1());
		if (m_private->columnMode() == AbstractColumn::Numeric) {
			auto* data = new QVector<double>(bytes.size()/(int)sizeof(double));
			memcpy(data->data(), bytes.data(), bytes.size());
			m_private->replaceData(data);
		} else {
			auto* data = new QVector<int>(bytes.size()/(int)sizeof(int));
			memcpy(data->data(), bytes.data(), bytes.size());
			m_private->replaceData(data);
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

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	QString str = attribs.value("rows").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("rows").toString());
	else
		d->resizeTo(str.toInt());

	str = attribs.value("designation").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("designation").toString());
	else
		d->setPlotDesignation( AbstractColumn::PlotDesignation(str.toInt()) );

	str = attribs.value("mode").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("mode").toString());
	else
		setColumnModeFast( AbstractColumn::ColumnMode(str.toInt()) );

	str = attribs.value("width").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("width").toString());
	else
		d->setWidth(str.toInt());

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement()) break;

		if (reader->isStartElement()) {
			bool ret_val = true;
			if (reader->name() == "comment")
				ret_val = readCommentElement(reader);
			else if (reader->name() == "input_filter")
				ret_val = XmlReadInputFilter(reader);
			else if (reader->name() == "output_filter")
				ret_val = XmlReadOutputFilter(reader);
			else if (reader->name() == "mask")
				ret_val = XmlReadMask(reader);
			else if (reader->name() == "formula")
				ret_val = XmlReadFormula(reader);
			else if (reader->name() == "row")
				ret_val = XmlReadRow(reader);
			else { // unknown element
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
				if (!reader->skipToEndElement()) return false;
			}
			if (!ret_val)
				return false;
		}
		if (!preview) {
			QString content = reader->text().toString().trimmed();
			if (!content.isEmpty() && ( columnMode() == AbstractColumn::Numeric ||  columnMode() == AbstractColumn::Integer)) {
				auto* task = new DecodeColumnTask(d, content);
				QThreadPool::globalInstance()->start(task);
			}
		}
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
	Q_ASSERT(reader->isStartElement() == true && reader->name() == "input_filter");
	if (!reader->skipToNextTag()) return false;
	if (!d->inputFilter()->load(reader, false)) return false;
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() == true && reader->name() == "input_filter");
	return true;
}

/**
 * \brief Read XML output filter element
 */
bool Column::XmlReadOutputFilter(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() == true && reader->name() == "output_filter");
	if (!reader->skipToNextTag()) return false;
	if (!d->outputFilter()->load(reader, false)) return false;
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() == true && reader->name() == "output_filter");
	return true;
}

/**
 * \brief Read XML formula element
 */
bool Column::XmlReadFormula(XmlStreamReader* reader) {
	QString formula;
	QStringList variableNames;
	QStringList columnPathes;
	bool autoUpdate = reader->attributes().value("autoUpdate").toInt();
	while (reader->readNext()) {
		if (reader->isEndElement()) break;

		if (reader->name() == "text")
			formula = reader->readElementText();
		else if (reader->name() == "variableNames") {
			while (reader->readNext()) {
				if (reader->name() == "variableNames" && reader->isEndElement()) break;

				if (reader->isStartElement())
					variableNames << reader->readElementText();
			}
		} else if (reader->name() == "columnPathes") {
			while (reader->readNext()) {
				if (reader->name() == "columnPathes" && reader->isEndElement()) break;

				if (reader->isStartElement())
					columnPathes << reader->readElementText();
			}
		}
	}

	d->setFormula(formula, variableNames, columnPathes, autoUpdate);

	return true;
}

//TODO: read cell formula, not implemented yet
// bool Column::XmlReadFormula(XmlStreamReader* reader)
// {
// 	Q_ASSERT(reader->isStartElement() && reader->name() == "formula");
//
// 	bool ok1, ok2;
// 	int start, end;
// 	start = reader->readAttributeInt("start_row", &ok1);
// 	end = reader->readAttributeInt("end_row", &ok2);
// 	if (!ok1 || !ok2)
// 	{
// 		reader->raiseError(i18n("invalid or missing start or end row"));
// 		return false;
// 	}
// 	setFormula(Interval<int>(start,end), reader->readElementText());
//
// 	return true;
// }


/**
 * \brief Read XML row element
 */
bool Column::XmlReadRow(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() == true && reader->name() == "row");

	//	QXmlStreamAttributes attribs = reader->attributes();

	bool ok;
	int index = reader->readAttributeInt("index", &ok);
	if (!ok) {
		reader->raiseError(i18n("invalid or missing row index"));
		return false;
	}

	QString str = reader->readElementText();
	switch (columnMode()) {
	case AbstractColumn::Numeric: {
			double value = str.toDouble(&ok);
			if (!ok) {
				reader->raiseError(i18n("invalid row value"));
				return false;
			}
			setValueAt(index, value);
			break;
		}
	case AbstractColumn::Integer: {
			int value = str.toInt(&ok);
			if (!ok) {
				reader->raiseError(i18n("invalid row value"));
				return false;
			}
			setIntegerAt(index, value);
			break;
		}
	case AbstractColumn::Text:
		setTextAt(index, str);
		break;

	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		QDateTime date_time = QDateTime::fromString(str,"yyyy-dd-MM hh:mm:ss:zzz");
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

/**
 * \brief Return the data vector size
 *
 * This returns the number of rows that actually contain data.
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int Column::rowCount() const {
	return d->rowCount();
}

/**
 * \brief Return the column plot designation
 */
AbstractColumn::PlotDesignation Column::plotDesignation() const {
	return d->plotDesignation();
}

QString Column::plotDesignationString() const {
	switch (plotDesignation()) {
	case AbstractColumn::NoDesignation:
		return QString("");
	case AbstractColumn::X:
		return QLatin1String("[X]");
	case AbstractColumn::Y:
		return QLatin1String("[Y]");
	case AbstractColumn::Z:
		return QLatin1String("[Z]");
	case AbstractColumn::XError:
		return QLatin1String("[") + i18n("X-error") + QLatin1Char(']');
	case AbstractColumn::XErrorPlus:
		return QLatin1String("[") + i18n("X-error +") + QLatin1Char(']');
	case AbstractColumn::XErrorMinus:
		return QLatin1String("[") + i18n("X-error -") + QLatin1Char(']');
	case AbstractColumn::YError:
		return QLatin1String("[") + i18n("Y-error") + QLatin1Char(']');
	case AbstractColumn::YErrorPlus:
		return QLatin1String("[") + i18n("Y-error +") + QLatin1Char(']');
	case AbstractColumn::YErrorMinus:
		return QLatin1String("[") + i18n("Y-error -") + QLatin1Char(']');
	}

	return QString("");
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
QVector< Interval<int> > Column::formulaIntervals() const {
	return d->formulaIntervals();
}

void Column::handleFormatChange() {
	DEBUG("Column::handleFormatChange() mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode()));
	if (columnMode() == AbstractColumn::DateTime) {
		auto* input_filter = static_cast<String2DateTimeFilter*>(d->inputFilter());
		auto* output_filter = static_cast<DateTime2StringFilter*>(d->outputFilter());
		DEBUG("change format " << input_filter->format().toStdString() << " to " << output_filter->format().toStdString());
		input_filter->setFormat(output_filter->format());
	}

	emit aspectDescriptionChanged(this); // the icon for the type changed
	if (!m_suppressDataChangedSignal)
		emit formatChanged(this); // all cells must be repainted

	d->statisticsAvailable = false;
	d->hasValuesAvailable = false;
    d->propertiesAvailable = false;
	DEBUG("Column::handleFormatChange() DONE");
}

/*!
 * calculates the minimal value in the column.
 * for \c count = 0, the minimum of all elements is returned.
 * for \c count > 0, the minimum of the first \count elements is returned.
 * for \c count < 0, the minimum of the last \count elements is returned.
 */
double Column::minimum(int count) const {
	double min = INFINITY;
	if (count == 0 && d->statisticsAvailable)
		min = const_cast<Column*>(this)->statistics().minimum;
	else {
		int start, end;

		if (count == 0) {
			start = 0;
			end = rowCount();
		} else if (count > 0) {
			start  = 0;
			end = qMin(rowCount(), count);
		} else {
			start = qMax(rowCount() + count, 0);
			end = rowCount();
		}
		return minimum(start, end);
	}

	return min;
}

/*!
 * \brief Column::minimum
 * Calculates the minimum value in the column between the \p startIndex and \p endIndex, endIndex is excluded.
 * If startIndex is greater than endIndex the indices are swapped
 * \p startIndex
 * \p endIndex
 */
double Column::minimum(int startIndex, int endIndex) const {
	double min = INFINITY;

	if (rowCount() == 0)
		return min;

	if (startIndex > endIndex && startIndex >= 0 && endIndex >= 0)
		std::swap(startIndex, endIndex);

	startIndex = qMax(startIndex, 0);
	endIndex = qMax(endIndex, 0);

	startIndex = qMin(startIndex, rowCount() - 1);
	endIndex = qMin(endIndex, rowCount() - 1);

	int foundIndex = 0;

	ColumnMode mode = columnMode();
	Properties property = properties();
	if (property == Properties::No) {
		// skipping values is only in Properties::No needed, because
		// when there are invalid values the property must be Properties::No
		switch (mode) {
		case Numeric: {
			auto* vec = static_cast<QVector<double>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
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
		case Integer: {
			auto* vec = static_cast<QVector<int>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const int val = vec->at(row);

				if (val < min)
					min = val;
			}
			break;
		}
		case Text:
			break;
		case DateTime: {
			auto* vec = static_cast<QVector<QDateTime>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
				if (!isValid(row) || isMasked(row))
					continue;

				const qint64 val = vec->at(row).toMSecsSinceEpoch();

				if (val < min)
					min = val;
			}
			break;
		}
		case Day:
		case Month:
		default:
			break;
		}
		return min;
	}

	// use the properties knowledge to determine maximum faster
	if (property == Properties::Constant || property == Properties::MonotonicIncreasing)
		foundIndex = startIndex;
	else if (property == Properties::MonotonicDecreasing)
		foundIndex = endIndex;

	switch (mode) {
		case Numeric:
		case Integer:
			return valueAt(foundIndex);
		case DateTime:
		case Month:
		case Day:
			return dateTimeAt(foundIndex).toMSecsSinceEpoch();
		case Text:
		default:
			break;
	}

	return min;
}

/*!
 * calculates the maximal value in the column.
 * for \c count = 0, the maximum of all elements is returned.
 * for \c count > 0, the maximum of the first \count elements is returned.
 * for \c count < 0, the maximum of the last \count elements is returned.
 */
double Column::maximum(int count) const {
	double max = -INFINITY;

	if (count == 0 && d->statisticsAvailable)
		max = const_cast<Column*>(this)->statistics().maximum;
	else {
		int start, end;

		if (count == 0) {
			start = 0;
			end = rowCount();
		} else if (count > 0) {
			start  = 0;
			end = qMin(rowCount(), count);
		} else {
			start = qMax(rowCount() + count, 0);
			end = rowCount();
		}
		return maximum(start, end);
	}

	return max;
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

	startIndex = qMax(startIndex, 0);
	endIndex = qMax(endIndex, 0);

	startIndex = qMin(startIndex, rowCount() - 1);
	endIndex = qMin(endIndex, rowCount() - 1);
	int foundIndex = 0;

	ColumnMode mode = columnMode();
	Properties property = properties();
	if (property == Properties::No) {
		switch (mode) {
		case Numeric: {
			auto* vec = static_cast<QVector<double>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
				const double val = vec->at(row);
				if (std::isnan(val))
					continue;

				if (val > max)
					max = val;
			}
			break;
		}
		case Integer: {
			auto* vec = static_cast<QVector<int>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
				const int val = vec->at(row);

				if (val > max)
					max = val;
			}
			break;
		}
		case Text:
			break;
		case DateTime: {
			auto* vec = static_cast<QVector<QDateTime>*>(data());
			for (int row = startIndex; row < endIndex; ++row) {
				const qint64 val = vec->at(row).toMSecsSinceEpoch();

				if (val > max)
					max = val;
			}
			break;
		}
		case Day:
		case Month:
		default:
			break;
		}
		return max;
	}

	// use the properties knowledge to determine maximum faster
	if (property == Properties::Constant || property == Properties::MonotonicDecreasing)
		foundIndex = startIndex;
	else if (property == Properties::MonotonicIncreasing)
		foundIndex = endIndex;

	switch (mode) {
		case Numeric:
		case Integer:
			return valueAt(foundIndex);
		case DateTime:
		case Month:
		case Day:
			return dateTimeAt(foundIndex).toMSecsSinceEpoch();
		case Text:
		default:
			break;
	}
	return max;
}

/*!
 * calculates log2(x)+1 for an integer value.
 * Used in y(double x) to calculate the maximum steps
 * source: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * source: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
 * @param value
 * @return returns calculated value
 */
// TODO: testing if it is faster than calculating log2.
int Column::calculateMaxSteps (unsigned int value) {
	const signed char LogTable256[256] = {
		-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};

	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries
	if ((tt = value >> 16))
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		r = (t = value >> 8) ? 8 + LogTable256[t] : LogTable256[value];

	return r+1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int Column::indexForValue(double x, QVector<double>& column, Properties properties) {
	int rowCount = column.count();
	if (rowCount == 0)
		return -1;

	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount-1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = column[index];

			if (higherIndex - lowerIndex < 2) {
				if (qAbs(column[lowerIndex] - x) < qAbs(column[higherIndex] - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value > x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value <= x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		int index = 0;
		prevValue = column[0];
		for (int row = 0; row < rowCount; row++) {

			double value = column[row];
			if (qAbs(value - x) <= qAbs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
					prevValue = value;
					index = row;
			}
		}
		return index;
	}
	return -1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int Column::indexForValue(const double x, const QVector<QPointF>& points, Properties properties) {
	int rowCount = points.count();

	if (rowCount == 0)
		return -1;

	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = points[index].x();

			if (higherIndex - lowerIndex < 2) {
				if (qAbs(points[lowerIndex].x() - x) < qAbs(points[higherIndex].x() - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value > x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value <= x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		prevValue = points[0].x();
		int index = 0;
		for (int row = 0; row < rowCount; row++) {

			double value = points[row].x();
			if (qAbs(value - x) <= qAbs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
					prevValue = value;
					index = row;
			}
		}
		return index;
	}
	return -1;
}

/*!
* Find index which corresponds to a @p x . In a vector of values
* When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
* @param x
* @return -1 if index not found, otherwise the index
*/
int Column::indexForValue(double x, QVector<QLineF>& lines, Properties properties) {
	int rowCount = lines.count();
	if (rowCount == 0)
		return -1;
	// use only p1 to find index
	double prevValue = 0;
	//qint64 prevValueDateTime = 0;
	if (properties == AbstractColumn::Properties::MonotonicIncreasing ||
			properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if(properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount-1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount))+1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
			double value = lines[index].p1().x();

			if (higherIndex - lowerIndex < 2) {
				if (qAbs(lines[lowerIndex].p1().x() - x) < qAbs(lines[higherIndex].p1().x() - x))
					index = lowerIndex;
				else
					index = higherIndex;

				return index;
			}

			if (value > x && increase)
				higherIndex = index;
			else if (value >= x && !increase)
				lowerIndex = index;
			else if (value <= x && increase)
				lowerIndex = index;
			else if (value < x && !increase)
				higherIndex = index;

		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No
		// naiv way
		int index = 0;
		prevValue = lines[0].p1().x();
		for (int row = 0; row < rowCount; row++) {
			double value = lines[row].p1().x();
			if (qAbs(value - x) <= qAbs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				prevValue = value;
				index = row;
			}
		}
		return index;
	}
	return -1;
}

int Column::indexForValue(double x) const {

	double prevValue = 0;
	qint64 prevValueDateTime = 0;
	AbstractColumn::ColumnMode mode = columnMode();
	int property = properties();
	if (property == AbstractColumn::Properties::MonotonicIncreasing ||
			property == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = (property != AbstractColumn::Properties::MonotonicDecreasing);

		int lowerIndex = 0;
		int higherIndex = rowCount() - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount()))+1;

		if ((mode == AbstractColumn::ColumnMode::Numeric ||
			 mode == AbstractColumn::ColumnMode::Integer)) {
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
				double value = valueAt(index);

				if (higherIndex - lowerIndex < 2) {
					if (qAbs(valueAt(lowerIndex) - x) < qAbs(valueAt(higherIndex) - x))
						index = lowerIndex;
					else
						index = higherIndex;

					return index;
				}

				if (value > x && increase)
					higherIndex = index;
				else if (value >= x && !increase)
					lowerIndex = index;
				else if (value <= x && increase)
					lowerIndex = index;
				else if (value < x && !increase)
					higherIndex = index;

			}
		} else if ((mode == AbstractColumn::ColumnMode::DateTime ||
					mode == AbstractColumn::ColumnMode::Month ||
					mode == AbstractColumn::ColumnMode::Day)) {
			qint64 xInt64 = static_cast<qint64>(x);
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex)/2);
				qint64 value = dateTimeAt(index).toMSecsSinceEpoch();

				if (higherIndex - lowerIndex < 2) {
					if (abs(dateTimeAt(lowerIndex).toMSecsSinceEpoch() - xInt64) < abs(dateTimeAt(higherIndex).toMSecsSinceEpoch() - xInt64))
						index = lowerIndex;
					else
						index = higherIndex;

					return index;
				}

				if (value > xInt64 && increase)
					higherIndex = index;
				else if (value >= xInt64 && !increase)
					lowerIndex = index;
				else if (value <= xInt64 && increase)
					lowerIndex = index;
				else if (value < xInt64 && !increase)
					higherIndex = index;

			}
		}

	} else if (property == AbstractColumn::Properties::Constant) {
		if (rowCount() > 0)
			return 0;
		else
			return -1;
	} else {
		// naiv way
		int index = 0;
		if ((mode == AbstractColumn::ColumnMode::Numeric ||
			 mode == AbstractColumn::ColumnMode::Integer)) {
			for (int row = 0; row < rowCount(); row++) {
				if (!isValid(row) || isMasked(row))
					continue;
				if (row == 0)
					prevValue = valueAt(row);

				double value = valueAt(row);
				if (abs(value - x) <= abs(prevValue - x)) { // <= prevents also that row - 1 become < 0
					if (row < rowCount() - 1) {
						prevValue = value;
						index = row;
					}
				}
			}
			return index;
		} else if ((mode == AbstractColumn::ColumnMode::DateTime ||
					mode == AbstractColumn::ColumnMode::Month ||
					mode == AbstractColumn::ColumnMode::Day)) {
			qint64 xInt64 = static_cast<qint64>(x);
			int index = 0;
			for (int row = 0; row < rowCount(); row++) {
				if (!isValid(row) || isMasked(row))
					continue;

				if (row == 0)
					prevValueDateTime = dateTimeAt(row).toMSecsSinceEpoch();

				qint64 value = dateTimeAt(row).toMSecsSinceEpoch();
				if (abs(value - xInt64) <= abs(prevValueDateTime - xInt64)) { // "<=" prevents also that row - 1 become < 0
					prevValueDateTime = value;
					index = row;
				}
			}
			return index;
		}
	}
	return -1;
}

/*!
 * Finds the minimal and maximal index which are between v1 and v2
 * \brief Column::indicesForX
 * \param x1
 * \param x2
 * \param start
 * \param end
 * \return
 */
bool Column::indicesMinMax(double v1, double v2, int& start, int& end) const {

	start = -1;
	end = -1;
	if (rowCount() == 0)
		return false;

	// Assumption: v1 is always the smaller value
	if (v1 > v2)
		qSwap(v1, v2);

	Properties property = properties();
	if (property == Properties::MonotonicIncreasing ||
		property == Properties::MonotonicDecreasing) {
		start = indexForValue(v1);
		end = indexForValue(v2);

		switch (columnMode()) {
			case Integer:
			case Numeric: {
			if (start > 0 && valueAt(start -1) <= v2 && valueAt(start -1) >= v1)
				start--;
			if (end < rowCount() - 1 && valueAt(end + 1) <= v2 && valueAt(end + 1) >= v1)
				end++;

			break;
			}
			case DateTime:
			case Month:
			case Day: {
				qint64 v1int64 = v1;
				qint64 v2int64 = v2;
				qint64 value;
				if (start > 0) {
					value = dateTimeAt(start -1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						start--;
				}

				if (end > rowCount() - 1) {
					value = dateTimeAt(end + 1).toMSecsSinceEpoch();
					if (value <= v2int64 && value >= v1int64)
						end++;
				}
				break;
			}
			case Text:
				return false;
		}
		return true;
	} else if (property == Properties::Constant) {
		start = 0;
		end = rowCount() - 1;
		return true;
	}
	// property == Properties::No
	switch (columnMode()) {
		case Integer:
		case Numeric: {
			double value;
			for (int i = 0; i < rowCount(); i++) {
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
		case DateTime:
		case Month:
		case Day: {
			qint64 value;
			qint64 v2int64 = v2;
			qint64 v1int64 = v2;
			for (int i = 0; i < rowCount(); i++) {
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
		case Text:
			return false;

	}
	return true;
}
