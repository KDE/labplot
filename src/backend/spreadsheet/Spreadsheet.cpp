/***************************************************************************
    File                 : Spreadsheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2012-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "Spreadsheet.h"
#include "SpreadsheetModel.h"
#include "backend/core/AspectPrivate.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include <QIcon>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <algorithm>

/*!
  \class Spreadsheet
  \brief Aspect providing a spreadsheet table with column logic.

  Spreadsheet is a container object for columns with no data of its own. By definition, it's columns
  are all of its children inheriting from class Column. Thus, the basic API is already defined
  by AbstractAspect (managing the list of columns, notification of column insertion/removal)
  and Column (changing and monitoring state of the actual data).

  Spreadsheet stores a pointer to its primary view of class SpreadsheetView. SpreadsheetView calls the Spreadsheet
  API but Spreadsheet only notifies SpreadsheetView by signals without calling its API directly. This ensures a
  maximum independence of UI and backend. SpreadsheetView can be easily replaced by a different class.
  User interaction is completely handled in SpreadsheetView and translated into
  Spreadsheet API calls (e.g., when a user edits a cell this will be handled by the delegate of
  SpreadsheetView and Spreadsheet will not know whether a script or a user changed the data.). All actions,
  menus etc. for the user interaction are handled SpreadsheetView, e.g., via a context menu.
  Selections are also handled by SpreadsheetView. The view itself is created by the first call to view();

  \ingroup backend
*/

Spreadsheet::Spreadsheet(const QString& name, bool loading, AspectType type)
	: AbstractDataSource(name, type) {

	if (!loading)
		init();
}

/*!
	initializes the spreadsheet with the default number of columns and rows
*/
void Spreadsheet::init() {
	KConfig config;
	KConfigGroup group = config.group(QLatin1String("Spreadsheet"));

	const int columns = group.readEntry(QLatin1String("ColumnCount"), 2);
	const int rows = group.readEntry(QLatin1String("RowCount"), 100);

	for (int i = 0; i < columns; i++) {
		Column* new_col = new Column(QString::number(i+1), AbstractColumn::ColumnMode::Numeric);
		new_col->setPlotDesignation(i == 0 ? AbstractColumn::PlotDesignation::X : AbstractColumn::PlotDesignation::Y);
		addChild(new_col);
	}
	setRowCount(rows);
}

void Spreadsheet::setModel(SpreadsheetModel* model) {
	m_model = model;
}

SpreadsheetModel* Spreadsheet::model() {
	return m_model;
}

/*! Constructs a primary view on me.
  This method may be called multiple times during the life time of an Aspect, or it might not get
  called at all. Aspects must not depend on the existence of a view for their operation.
*/
QWidget* Spreadsheet::view() const {
	if (!m_partView) {
		bool readOnly = (this->parentAspect()->type() == AspectType::DatapickerCurve);
		m_view = new SpreadsheetView(const_cast<Spreadsheet*>(this), readOnly);
		m_partView = m_view;
	}
	return m_partView;
}

bool Spreadsheet::exportView() const {
	return m_view->exportView();
}

bool Spreadsheet::printView() {
	return m_view->printView();
}

bool Spreadsheet::printPreview() const {
	return m_view->printPreview();
}

/*!
  Returns the maximum number of rows in the spreadsheet.
 */
int Spreadsheet::rowCount() const {
	int result = 0;
	for (auto* col : children<Column>()) {
		const int col_rows = col->rowCount();
		if ( col_rows > result)
			result = col_rows;
	}
	return result;
}

void Spreadsheet::removeRows(int first, int count) {
	if ( count < 1 || first < 0 || first+count > rowCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: remove 1 row", "%1: remove %2 rows", name(), count) );
	for (auto* col : children<Column>())
		col->removeRows(first, count);
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertRows(int before, int count) {
	if ( count < 1 || before < 0 || before > rowCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: insert 1 row", "%1: insert %2 rows", name(), count) );
	for (auto* col : children<Column>())
		col->insertRows(before, count);
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::appendRows(int count) {
	insertRows(rowCount(), count);
}

void Spreadsheet::appendRow() {
	insertRows(rowCount(), 1);
}

void Spreadsheet::appendColumns(int count) {
	insertColumns(columnCount(), count);
}

void Spreadsheet::appendColumn() {
	insertColumns(columnCount(), 1);
}

void Spreadsheet::prependColumns(int count) {
	insertColumns(0, count);
}

/*!
  Sets the number of rows of the spreadsheet to \c new_size
*/
void Spreadsheet::setRowCount(int new_size) {
	int current_size = rowCount();
	if (new_size > current_size)
		insertRows(current_size, new_size-current_size);
	if (new_size < current_size && new_size >= 0)
		removeRows(new_size, current_size-new_size);
}

/*!
  Returns the column with the number \c index.
  Shallow wrapper around \sa AbstractAspect::child() - see there for caveat.
*/
Column* Spreadsheet::column(int index) const {
	return child<Column>(index);
}

/*!
  Returns the column with the name \c name.
*/
Column* Spreadsheet::column(const QString &name) const {
	return child<Column>(name);
}

/*!
  Returns the total number of columns in the spreadsheet.
*/
int Spreadsheet::columnCount() const {
	return childCount<Column>();
}

/*!
  Returns the number of columns matching the given designation.
 */
int Spreadsheet::columnCount(AbstractColumn::PlotDesignation pd) const {
	int count = 0;
	for (auto* col : children<Column>())
		if (col->plotDesignation() == pd)
			count++;
	return count;
}

void Spreadsheet::removeColumns(int first, int count) {
	if ( count < 1 || first < 0 || first+count > columnCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: remove 1 column", "%1: remove %2 columns", name(), count) );
	for (int i = 0; i < count; i++)
		child<Column>(first)->remove();
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertColumns(int before, int count) {
	WAIT_CURSOR;
	beginMacro( i18np("%1: insert 1 column", "%1: insert %2 columns", name(), count) );
	Column * before_col = column(before);
	int rows = rowCount();
	for (int i = 0; i < count; i++) {
		Column * new_col = new Column(QString::number(i+1), AbstractColumn::ColumnMode::Numeric);
		new_col->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		new_col->insertRows(0, rows);
		insertChildBefore(new_col, before_col);
	}
	endMacro();
	RESET_CURSOR;
}
/*!
  Sets the number of columns to \c new_size
*/
void Spreadsheet::setColumnCount(int new_size) {
	int old_size = columnCount();
	if ( old_size == new_size || new_size < 0 )
		return;

	if (new_size < old_size)
		removeColumns(new_size, old_size-new_size);
	else
		insertColumns(old_size, new_size-old_size);
}

/*!
  Clears the whole spreadsheet.
*/
void Spreadsheet::clear() {
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear", name()));
	for (auto* col : children<Column>())
		col->clear();
	endMacro();
	RESET_CURSOR;
}

/*!
  Clears all mask in the spreadsheet.
*/
void Spreadsheet::clearMasks() {
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear all masks", name()));
	for (auto* col : children<Column>())
		col->clearMasks();
	endMacro();
	RESET_CURSOR;
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* Spreadsheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

void Spreadsheet::moveColumn(int from, int to) {
	Column* col = child<Column>(from);
	beginMacro(i18n("%1: move column %2 from position %3 to %4.", name(), col->name(), from+1, to+1));
	col->remove();
	insertChildBefore(col, child<Column>(to));
	endMacro();
}

void Spreadsheet::copy(Spreadsheet* other) {
	WAIT_CURSOR;
	beginMacro(i18n("%1: copy %2", name(), other->name()));

	for (auto* col : children<Column>())
		col->remove();
	for (auto* src_col : other->children<Column>()) {
		Column * new_col = new Column(src_col->name(), src_col->columnMode());
		new_col->copy(src_col);
		new_col->setPlotDesignation(src_col->plotDesignation());
		QVector< Interval<int> > masks = src_col->maskedIntervals();
		for (const auto& iv : masks)
			new_col->setMasked(iv);
		QVector< Interval<int> > formulas = src_col->formulaIntervals();
		for (const auto& iv : formulas)
			new_col->setFormula(iv, src_col->formula(iv.start()));
		new_col->setWidth(src_col->width());
		addChild(new_col);
	}
	setComment(other->comment());

	endMacro();
	RESET_CURSOR;
}

// FIXME: replace index-based API with Column*-based one
/*!
  Determines the corresponding X column.
*/
int Spreadsheet::colX(int col) {
	for (int i = col-1; i >= 0; i--) {
		if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::X)
			return i;
	}
	int cols = columnCount();
	for (int i = col+1; i < cols; i++) {
		if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::X)
			return i;
	}
	return -1;
}

/*!
  Determines the corresponding Y column.
*/
int Spreadsheet::colY(int col) {
	int cols = columnCount();

	if (column(col)->plotDesignation() == AbstractColumn::PlotDesignation::XError ||
	        column(col)->plotDesignation() == AbstractColumn::PlotDesignation::YError) {
		// look to the left first
		for (int i = col-1; i >= 0; i--) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
		for (int i = col+1; i < cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
	} else {
		// look to the right first
		for (int i = col+1; i < cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
		for (int i = col-1; i >= 0; i--) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
	}
	return -1;
}

/*! Sorts the given list of column.
  If 'leading' is a null pointer, each column is sorted separately.
*/
void Spreadsheet::sortColumns(Column* leading, const QVector<Column*>& cols, bool ascending) {
	DEBUG("Spreadsheet::sortColumns() : ascending = " << ascending)
	if (cols.isEmpty()) return;


	// the normal QPair comparison does not work properly with descending sorting
	// therefore we use our own compare functions
	// TODO: check this. a < b vs. a.first < b.first
	class CompareFunctions {
	public:
		static bool doubleLess(QPair<double, int> a, QPair<double, int> b) {
			return a.first < b.first;
		}
		static bool doubleGreater(QPair<double, int> a, QPair<double, int> b) {
			return a.first > b.first;
		}
		static bool integerLess(QPair<int, int> a, QPair<int, int> b) {
			return a.first < b.first;
		}
		static bool integerGreater(QPair<int, int> a, QPair<int, int> b) {
			return a.first > b.first;
		}
		static bool bigIntLess(QPair<qint64, int> a, QPair<qint64, int> b) {
			return a.first < b.first;
		}
		static bool bigIntGreater(QPair<qint64, int> a, QPair<qint64, int> b) {
			return a.first > b.first;
		}
		static bool QStringLess(const QPair<QString, int>& a, const QPair<QString, int>& b) {
			return a < b;
		}
		static bool QStringGreater(const QPair<QString, int>& a, const QPair<QString, int>& b) {
			return a > b;
		}
		static bool QDateTimeLess(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b) {
			return a < b;
		}
		static bool QDateTimeGreater(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b) {
			return a > b;
		}
	};

	WAIT_CURSOR;
	beginMacro(i18n("%1: sort columns", name()));

	if (leading == nullptr) { // sort separately
		DEBUG("	sort separately")
		for (auto* col : cols) {
			int rows = col->rowCount();
			std::unique_ptr<Column> tempCol(new Column("temp", col->columnMode()));

			switch (col->columnMode()) {
			case AbstractColumn::ColumnMode::Numeric: {
					QVector< QPair<double, int> > map;

					for (int i = 0; i < rows; i++)
						if (col->isValid(i))
							map.append(QPair<double, int>(col->valueAt(i), i));
					const int filledRows = map.size();

					if (ascending)
						std::stable_sort(map.begin(), map.end(), CompareFunctions::doubleLess);
					else
						std::stable_sort(map.begin(), map.end(), CompareFunctions::doubleGreater);

					// put the values in the right order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					break;
				}
			case AbstractColumn::ColumnMode::Integer: {
					QVector< QPair<int, int> > map;

					for (int i = 0; i < rows; i++)
						map.append(QPair<int, int>(col->valueAt(i), i));

					if (ascending)
						std::stable_sort(map.begin(), map.end(), CompareFunctions::integerLess);
					else
						std::stable_sort(map.begin(), map.end(), CompareFunctions::integerGreater);

					// put the values in the right order into tempCol
					for (int i = 0; i < rows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					break;
				}
			case AbstractColumn::ColumnMode::BigInt: {
					QVector< QPair<qint64, int> > map;

					for (int i = 0; i < rows; i++)
						map.append(QPair<qint64, int>(col->valueAt(i), i));

					if (ascending)
						std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntLess);
					else
						std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntGreater);

					// put the values in the right order into tempCol
					for (int i = 0; i < rows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					break;
				}
			case AbstractColumn::ColumnMode::Text: {
					QVector<QPair<QString, int>> map;

					for (int i = 0; i < rows; i++)
						if (!col->textAt(i).isEmpty())
							map.append(QPair<QString, int>(col->textAt(i), i));
					const int filledRows = map.size();

					if (ascending)
						std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringLess);
					else
						std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringGreater);

					// put the values in the right order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					break;
				}
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day: {
					QVector< QPair<QDateTime, int> > map;

					for (int i = 0; i < rows; i++)
						if (col->isValid(i))
							map.append(QPair<QDateTime, int>(col->dateTimeAt(i), i));
					const int filledRows = map.size();

					if (ascending)
						std::stable_sort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
					else
						std::stable_sort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);

					// put the values in the right order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					break;
				}
			}
			// copy the sorted column
			col->copy(tempCol.get(), 0, 0, rows);
		}
	} else { // sort with leading column
		DEBUG("	sort with leading column")
		int rows = leading->rowCount();

		switch (leading->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric: {
				QVector<QPair<double, int>> map;
				QVector<int> invalidIndex;

				for (int i = 0; i < rows; i++)
					if (leading->isValid(i))
						map.append(QPair<double, int>(leading->valueAt(i), i));
					else
						invalidIndex << i;
				const int filledRows = map.size();
				const int invalidRows = invalidIndex.size();

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::doubleLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::doubleGreater);

				for (auto* col : cols) {
					auto columnMode = col->columnMode();
					std::unique_ptr<Column> tempCol(new Column("temp", columnMode));
					// put the values in correct order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}

					// copy the sorted column
					if (col == leading)	// update all rows
						col->copy(tempCol.get(), 0, 0, rows);
					else {	// do not overwrite unused cols
						std::unique_ptr<Column> tempInvalidCol(new Column("temp2", col->columnMode()));
						for (int i = 0; i < invalidRows; i++) {
							const int idx = invalidIndex.at(i);
							//too slow: tempInvalidCol->copy(col, idx, i, 1);
							tempInvalidCol->setFromColumn(i, col, idx);
							tempInvalidCol->setMasked(col->isMasked(idx));
						}
						col->copy(tempCol.get(), 0, 0, filledRows);
						col->copy(tempInvalidCol.get(), 0, filledRows, invalidRows);
					}
				}
				break;
			}
		case AbstractColumn::ColumnMode::Integer: {
				//TODO: check if still working when invalid integer entries are supported
				QVector<QPair<int, int>> map;

				for (int i = 0; i < rows; i++)
					map.append(QPair<int, int>(leading->valueAt(i), i));

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::integerLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::integerGreater);

				for (auto* col : cols) {
					std::unique_ptr<Column> tempCol(new Column("temp", col->columnMode()));
					// put the values in the right order into tempCol
					for (int i = 0; i < rows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					// copy the sorted column
					col->copy(tempCol.get(), 0, 0, rows);
				}
				break;
			}
		case AbstractColumn::ColumnMode::BigInt: {
				QVector<QPair<qint64, int>> map;

				for (int i = 0; i < rows; i++)
					map.append(QPair<qint64, int>(leading->valueAt(i), i));

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntGreater);

				for (auto* col : cols) {
					std::unique_ptr<Column> tempCol(new Column("temp", col->columnMode()));
					// put the values in the right order into tempCol
					for (int i = 0; i < rows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					// copy the sorted column
					col->copy(tempCol.get(), 0, 0, rows);
				}
				break;
			}
		case AbstractColumn::ColumnMode::Text: {
				QVector<QPair<QString, int>> map;
				QVector<int> emptyIndex;

				for (int i = 0; i < rows; i++)
					if (!leading->textAt(i).isEmpty())
						map.append(QPair<QString, int>(leading->textAt(i), i));
					else
						emptyIndex << i;
				//QDEBUG("	empty indices: " << emptyIndex)
				const int filledRows = map.size();
				const int emptyRows = emptyIndex.size();

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringGreater);

				for (auto* col : cols) {
					std::unique_ptr<Column> tempCol(new Column("temp", col->columnMode()));
					// put the values in the right order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}

					// copy the sorted column
					if (col == leading)	// update all rows
						col->copy(tempCol.get(), 0, 0, rows);
					else {	// do not overwrite unused cols
						std::unique_ptr<Column> tempEmptyCol(new Column("temp2", col->columnMode()));
						for (int i = 0; i < emptyRows; i++) {
							const int idx = emptyIndex.at(i);
							//too slow: tempEmptyCol->copy(col, idx, i, 1);
							tempEmptyCol->setFromColumn(i, col, idx);
							tempEmptyCol->setMasked(col->isMasked(idx));
						}
						col->copy(tempCol.get(), 0, 0, filledRows);
						col->copy(tempEmptyCol.get(), 0, filledRows, emptyRows);
					}
				}
				break;
			}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day: {
				QVector<QPair<QDateTime, int>> map;
				QVector<int> invalidIndex;

				for (int i = 0; i < rows; i++)
					if (leading->isValid(i))
						map.append(QPair<QDateTime, int>(leading->dateTimeAt(i), i));
					else
						invalidIndex << i;
				const int filledRows = map.size();
				const int invalidRows = invalidIndex.size();

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);

				for (auto* col : cols) {
					std::unique_ptr<Column> tempCol(new Column("temp", col->columnMode()));
					// put the values in the right order into tempCol
					for (int i = 0; i < filledRows; i++) {
						int idx = map.at(i).second;
						//too slow: tempCol->copy(col, idx, i, 1);
						tempCol->setFromColumn(i, col, idx);
						tempCol->setMasked(col->isMasked(idx));
					}
					// copy the sorted column
					if (col == leading)	// update all rows
						col->copy(tempCol.get(), 0, 0, rows);
					else {	// do not overwrite unused cols
						std::unique_ptr<Column> tempInvalidCol(new Column("temp2", col->columnMode()));
						for (int i = 0; i < invalidRows; i++) {
							const int idx = invalidIndex.at(i);
							//too slow: tempInvalidCol->copy(col, idx, i, 1);
							tempInvalidCol->setFromColumn(i, col, idx);
							tempInvalidCol->setMasked(col->isMasked(idx));
						}
						col->copy(tempCol.get(), 0, 0, filledRows);
						col->copy(tempInvalidCol.get(), 0, filledRows, invalidRows);
					}
				}
				break;
			}
		}
	}

	endMacro();
	RESET_CURSOR;
} // end of sortColumns()

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon Spreadsheet::icon() const {
	return QIcon::fromTheme("labplot-spreadsheet");
}

/*!
  Returns the text displayed in the given cell.
*/
QString Spreadsheet::text(int row, int col) const {
	Column* c = column(col);
	if (!c)
		return QString();

	return c->asStringColumn()->textAt(row);
}

/*!
 * This slot is, indirectly, called when a child of \c Spreadsheet (i.e. column) was selected in \c ProjectExplorer.
 * Emits the signal \c columnSelected that is handled in \c SpreadsheetView.
 */
void Spreadsheet::childSelected(const AbstractAspect* aspect) {
	const Column* column = qobject_cast<const Column*>(aspect);
	if (column) {
		int index = indexOfChild<Column>(column);
		emit columnSelected(index);
	}
}

/*!
 * This slot is, indirectly, called when a child of \c Spreadsheet (i.e. column) was deselected in \c ProjectExplorer.
 * Emits the signal \c columnDeselected that is handled in \c SpreadsheetView.
 */
void Spreadsheet::childDeselected(const AbstractAspect* aspect) {
	const Column* column = qobject_cast<const Column*>(aspect);
	if (column) {
		int index = indexOfChild<Column>(column);
		emit columnDeselected(index);
	}
}

/*!
 *  Emits the signal to select or to deselect the column number \c index in the project explorer,
 *  if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 * This function is called in \c SpreadsheetView upon selection changes.
 */
void Spreadsheet::setColumnSelectedInView(int index, bool selected) {
	if (selected) {
		emit childAspectSelectedInView(child<Column>(index));

		//deselect the spreadsheet in the project explorer, if a child (column) was selected
		//and also all possible parents like folder, workbook, datapicker curve, datapicker
		//to prevents unwanted multiple selection in the project explorer
		//if one of the parents of the selected column was also selected before.
		AbstractAspect* parent = this;
		while (parent) {
			emit childAspectDeselectedInView(parent);
			parent = parent->parentAspect();
		}
	} else
		emit childAspectDeselectedInView(child<Column>(index));
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void Spreadsheet::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("spreadsheet");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//columns
	for (auto* col : children<Column>(ChildIndexFlag::IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); // "spreadsheet"
}

/*!
  Loads from XML.
*/
bool Spreadsheet::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement()) break;

		if (reader->isStartElement()) {
			if (reader->name() == "comment") {
				if (!readCommentElement(reader)) return false;
			} else if (reader->name() == "column") {
				Column* column = new Column(QString());
				if (!column->load(reader, preview)) {
					delete column;
					setColumnCount(0);
					return false;
				}
				addChildFast(column);
			} else {	// unknown element
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
				if (!reader->skipToEndElement()) return false;
			}
		}
	}

	return !reader->hasError();
}

//##############################################################################
//########################  Data Import  #######################################
//##############################################################################
int Spreadsheet::prepareImport(std::vector<void*>& dataContainer, AbstractFileFilter::ImportMode importMode,
                               int actualRows, int actualCols, QStringList colNameList, QVector<AbstractColumn::ColumnMode> columnMode) {
	DEBUG("Spreadsheet::prepareImport()")
	DEBUG("	resize spreadsheet to rows = " << actualRows << " and cols = " << actualCols)
	QDEBUG("	column name list = " << colNameList)
	int columnOffset = 0;
	setUndoAware(false);
	if (m_model != nullptr)
		m_model->suppressSignals(true);

	//make the available columns undo unaware before we resize and rename them below,
	//the same will be done for new columns in this->resize().
	for (int i = 0; i < childCount<Column>(); i++)
		child<Column>(i)->setUndoAware(false);

	columnOffset = this->resize(importMode, colNameList, actualCols);

	// resize the spreadsheet
	if (importMode == AbstractFileFilter::ImportMode::Replace) {
		clear();
		setRowCount(actualRows);
	}  else {
		if (rowCount() < actualRows)
			setRowCount(actualRows);
	}

	if (columnMode.size() < actualCols) {
		qWarning("columnMode[] size is too small! Giving up.");
		return -1;
	}

	dataContainer.resize(actualCols);
	for (int n = 0; n < actualCols; n++) {
		// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
		Column* column = this->child<Column>(columnOffset+n);
		DEBUG(" column " << n << " columnMode = " << static_cast<int>(columnMode[n]));
		column->setColumnModeFast(columnMode[n]);

		//in the most cases the first imported column is meant to be used as x-data.
		//Other columns provide mostly y-data or errors.
		//TODO: this has to be configurable for the user in the import widget,
		//it should be possible to specify x-error plot designation, etc.
		auto desig =  (n == 0) ? AbstractColumn::PlotDesignation::X : AbstractColumn::PlotDesignation::Y;
		column->setPlotDesignation(desig);

		switch (columnMode[n]) {
		case AbstractColumn::ColumnMode::Numeric: {
			auto* vector = static_cast<QVector<double>*>(column->data());
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* vector = static_cast<QVector<int>*>(column->data());
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* vector = static_cast<QVector<qint64>*>(column->data());
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* vector = static_cast<QVector<QString>*>(column->data());
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			auto* vector = static_cast<QVector<QDateTime>* >(column->data());
			vector->resize(actualRows);
			dataContainer[n] = static_cast<void*>(vector);
			break;
		}
		}
	}
//	QDEBUG("dataPointers =" << dataPointers);

	DEBUG("Spreadsheet::prepareImport() DONE");

	return columnOffset;
}

/*!
	resize data source to cols columns
	returns column offset depending on import mode
*/
int Spreadsheet::resize(AbstractFileFilter::ImportMode mode, QStringList colNameList, int cols) {
	DEBUG("Spreadsheet::resize()")
	QDEBUG("	column name list = " << colNameList)
	// name additional columns
	for (int k = colNameList.size(); k < cols; k++ )
		colNameList.append( "Column " + QString::number(k+1) );

	int columnOffset = 0; //indexes the "start column" in the spreadsheet. Starting from this column the data will be imported.

	Column* newColumn = nullptr;
	if (mode == AbstractFileFilter::ImportMode::Append) {
		columnOffset = childCount<Column>();
		for (int n = 0; n < cols; n++ ) {
			newColumn = new Column(colNameList.at(n), AbstractColumn::ColumnMode::Numeric);
			newColumn->setUndoAware(false);
			addChildFast(newColumn);
		}
	} else if (mode == AbstractFileFilter::ImportMode::Prepend) {
		Column* firstColumn = child<Column>(0);
		for (int n = 0; n < cols; n++ ) {
			newColumn = new Column(colNameList.at(n), AbstractColumn::ColumnMode::Numeric);
			newColumn->setUndoAware(false);
			insertChildBeforeFast(newColumn, firstColumn);
		}
	} else if (mode == AbstractFileFilter::ImportMode::Replace) {
		//replace completely the previous content of the data source with the content to be imported.
		int columns = childCount<Column>();


		if (columns > cols) {
			//there're more columns in the data source then required -> remove the superfluous columns
			for (int i = 0; i < columns-cols; i++)
				removeChild(child<Column>(0));
		} else {
			//create additional columns if needed
			for (int i = columns; i < cols; i++) {
				newColumn = new Column(colNameList.at(i), AbstractColumn::ColumnMode::Numeric);
				newColumn->setUndoAware(false);
				addChildFast(newColumn);
			}
		}

		// 1. rename the columns that were already available
		// 2. suppress the dataChanged signal for all columns
		// 3. send aspectDescriptionChanged because otherwise the column
		//    will not be connected again to the curves (project.cpp, descriptionChanged)
		for (int i = 0; i < childCount<Column>(); i++) {
			child<Column>(i)->setSuppressDataChangedSignal(true);
			emit child<Column>(i)->reset(child<Column>(i));
			child<Column>(i)->setName(colNameList.at(i));
			child<Column>(i)->aspectDescriptionChanged(child<Column>(i));
		}
	}

	return columnOffset;
}

void Spreadsheet::finalizeImport(int columnOffset, int startColumn, int endColumn, const QString& dateTimeFormat, AbstractFileFilter::ImportMode importMode)  {
	DEBUG("Spreadsheet::finalizeImport()");

	//determine the dependent plots
	QVector<CartesianPlot*> plots;
	if (importMode == AbstractFileFilter::ImportMode::Replace) {
		for (int n = startColumn; n <= endColumn; n++) {
			Column* column = this->column(columnOffset + n - startColumn);
			column->addUsedInPlots(plots);
		}

		//suppress retransform in the dependent plots
		for (auto* plot : plots)
			plot->setSuppressDataChangedSignal(true);
	}

	// set the comments for each of the columns if datasource is a spreadsheet
	const int rows = rowCount();
	for (int n = startColumn; n <= endColumn; n++) {
		Column* column = this->column(columnOffset + n - startColumn);
		DEBUG("	column " << n << " of type " << static_cast<int>(column->columnMode()));

		QString comment;
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Numeric:
			comment = i18np("numerical data, %1 element", "numerical data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::Integer:
			comment = i18np("integer data, %1 element", "integer data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			comment = i18np("big integer data, %1 element", "big integer data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::Text:
			comment = i18np("text data, %1 element", "text data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::Month:
			comment = i18np("month data, %1 element", "month data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::Day:
			comment = i18np("day data, %1 element", "day data, %1 elements", rows);
			break;
		case AbstractColumn::ColumnMode::DateTime:
			comment = i18np("date and time data, %1 element", "date and time data, %1 elements", rows);
			// set same datetime format in column
			auto* filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
			filter->setFormat(dateTimeFormat);
		}
		column->setComment(comment);

		if (importMode == AbstractFileFilter::ImportMode::Replace) {
			column->setSuppressDataChangedSignal(false);
			column->setChanged();
		}
	}

	if (importMode == AbstractFileFilter::ImportMode::Replace) {
		//retransform the dependent plots
		for (auto* plot : plots) {
			plot->setSuppressDataChangedSignal(false);
			plot->dataChanged();
		}
	}

	//make the spreadsheet and all its children undo aware again
	setUndoAware(true);
	for (int i = 0; i < childCount<Column>(); i++)
		child<Column>(i)->setUndoAware(true);

	if (m_model != nullptr)
		m_model->suppressSignals(false);

	if (m_partView != nullptr && m_view != nullptr)
		m_view->resizeHeader();

	DEBUG("Spreadsheet::finalizeImport() DONE");
}
