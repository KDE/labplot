/*
	File                 : Spreadsheet.cpp
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet table with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2006-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2006-2009 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2012-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "Spreadsheet.h"
#include "SpreadsheetModel.h"
#include "SpreadsheetPrivate.h"
#include "StatisticsSpreadsheet.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AspectPrivate.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include "backend/lib/commandtemplates.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QIcon>
#include <QMenu>
#include <QTimer>
#include <QUndoCommand>
#include <QXmlStreamWriter>

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
	: AbstractDataSource(name, type)
	, d_ptr(new SpreadsheetPrivate(this)) {
	if (!loading)
		init();
}

Spreadsheet::~Spreadsheet() {
	delete m_model;
	delete d_ptr;
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
		Column* new_col = new Column(QString::number(i + 1), AbstractColumn::ColumnMode::Double);
		new_col->setPlotDesignation(i == 0 ? AbstractColumn::PlotDesignation::X : AbstractColumn::PlotDesignation::Y);
		addChild(new_col);
	}
	setRowCount(rows);
}

void Spreadsheet::setModel(SpreadsheetModel* model) {
	m_model = model;
}

SpreadsheetModel* Spreadsheet::model() const {
	return m_model;
}

/*! Constructs a primary view on me.
  This method may be called multiple times during the life time of an Aspect, or it might not get
  called at all. Aspects must not depend on the existence of a view for their operation.
*/
QWidget* Spreadsheet::view() const {
#ifndef SDK
	if (!m_partView) {
		auto type = this->parentAspect()->type();
		bool readOnly = (type == AspectType::Spreadsheet || type == AspectType::DatapickerCurve);
		m_view = new SpreadsheetView(const_cast<Spreadsheet*>(this), readOnly);
		m_partView = m_view;
		connect(this, &Spreadsheet::viewAboutToBeDeleted, [this]() {
			m_view = nullptr;
		});

		// navigate to the first cell and set the focus so the user can start directly entering new data
		QTimer::singleShot(0, this, [=]() {
			if (m_view) { // we're accessing m_view outside of the event loop, it can be already deleted, check for nulltpr
				m_view->goToCell(0, 0);
				m_view->setFocus();
			}
		});
	}
	return m_partView;
#else
	return nullptr;
#endif
}

bool Spreadsheet::exportView() const {
#ifndef SDK
	return m_view->exportView();
#else
	return true;
#endif
}

bool Spreadsheet::printView() {
#ifndef SDK
	return m_view->printView();
#else
	return true;
#endif
}

bool Spreadsheet::printPreview() const {
#ifndef SDK
	return m_view->printPreview();
#else
	return true;
#endif
}

StatisticsSpreadsheet* Spreadsheet::statisticsSpreadsheet() const {
	Q_D(const Spreadsheet);
	return d->statisticsSpreadsheet;
}

/*!
 * \brief Called when the application settings were changed.
 *  adjusts the appearence of the spreadsheet header.
 */
void Spreadsheet::updateHorizontalHeader() {
#ifndef SDK
	if (m_model) {
		const QString& oldHeader = m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString();
		m_model->updateHorizontalHeader();
		const QString& newHeader = m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString();

		// if the header name of the first column has changed (column mode to be shown, etc.),
		// reset the column widths and request the view to adjuste the column sizes to the  content
		if (oldHeader != newHeader && m_view) {
			const auto& columns = children<Column>();
			for (auto col : columns)
				col->setWidth(0);
			m_view->resizeHeader();
		}
	}
#endif
}

void Spreadsheet::updateLocale() {
	for (auto* col : children<Column>())
		col->updateLocale();
}

/*!
  Returns the maximum number of rows in the spreadsheet.
 */
int Spreadsheet::rowCount() const {
	int result = 0;
	for (auto* col : children<Column>()) {
		const int col_rows = col->rowCount();
		if (col_rows > result)
			result = col_rows;
	}
	return result;
}

class SpreadsheetSetRowsCountCmd : public QUndoCommand {
public:
	SpreadsheetSetRowsCountCmd(Spreadsheet* spreadsheet, bool insert, int first, int count, QUndoCommand* parent)
		: QUndoCommand(parent)
		, m_spreadsheet(spreadsheet)
		, m_insert(insert)
		, m_first(first)
		, m_last(first + count - 1) {
		if (insert)
			setText(i18np("%1: insert 1 row", "%1: insert %2 rows", spreadsheet->name(), count));
		else
			setText(i18np("%1: remove 1 row", "%1: remove %2 rows", spreadsheet->name(), count));
	}

	virtual void redo() override {
		WAIT_CURSOR;
		if (m_insert)
			Q_EMIT m_spreadsheet->rowsAboutToBeInserted(m_first, m_last);
		else
			Q_EMIT m_spreadsheet->rowsAboutToBeRemoved(m_first, m_last);

		QUndoCommand::redo();

		if (m_insert)
			Q_EMIT m_spreadsheet->rowsInserted(m_spreadsheet->rowCount());
		else
			Q_EMIT m_spreadsheet->rowsRemoved(m_spreadsheet->rowCount());
		RESET_CURSOR;
		m_spreadsheet->emitRowCountChanged();
	}

	virtual void undo() override {
		WAIT_CURSOR;
		if (m_insert)
			Q_EMIT m_spreadsheet->rowsAboutToBeRemoved(m_first, m_last);
		else
			Q_EMIT m_spreadsheet->rowsAboutToBeInserted(m_first, m_last);
		QUndoCommand::undo();

		if (m_insert)
			Q_EMIT m_spreadsheet->rowsRemoved(m_spreadsheet->rowCount());
		else
			Q_EMIT m_spreadsheet->rowsInserted(m_spreadsheet->rowCount());
		RESET_CURSOR;
		m_spreadsheet->emitRowCountChanged();
	}

private:
	Spreadsheet* m_spreadsheet;
	bool m_insert;
	int m_first;
	int m_last;
};

void Spreadsheet::removeRows(int first, int count, QUndoCommand* parent) {
	if (count < 1 || first < 0 || first + count > rowCount())
		return;

	auto* command = new SpreadsheetSetRowsCountCmd(this, false, first, count, parent);

	for (auto* col : children<Column>())
		col->removeRows(first, count, command);

	if (!parent)
		exec(command);
}

void Spreadsheet::insertRows(int before, int count, QUndoCommand* parent) {
	if (count < 1 || before < 0 || before > rowCount())
		return;

	auto* command = new SpreadsheetSetRowsCountCmd(this, true, before, count, parent);

	for (auto* col : children<Column>())
		col->insertRows(before, count, command);

	if (!parent)
		exec(command);
}

void Spreadsheet::appendRows(int count) {
	insertRows(rowCount(), count);
}

void Spreadsheet::appendRow() {
	insertRows(rowCount(), 1);
}

/*!
 * removes all rows in the spreadsheet if the value in one of the columns is missing/empty.
 */
void Spreadsheet::removeEmptyRows() {
	const auto& rows = rowsWithMissingValues();
	if (rows.isEmpty())
		return;

	WAIT_CURSOR;
	beginMacro(i18n("%1: remove rows with missing values", name()));

	for (int row = rows.count() - 1; row >= 0; --row)
		removeRows(rows.at(row), 1);

	endMacro();
	RESET_CURSOR;
}

/*!
 * masks all rows in the spreadsheet if the value in one of the columns is missing/empty.
 */
void Spreadsheet::maskEmptyRows() {
	const auto& rows = rowsWithMissingValues();
	if (rows.isEmpty())
		return;

	WAIT_CURSOR;
	beginMacro(i18n("%1: mask rows with missing values", name()));

	const auto& columns = children<Column>();
	for (int row : rows) {
		for (const auto& col : columns)
			col->setMasked(row);
	}

	endMacro();
	RESET_CURSOR;
}

/*!
 * returns the list of all rows having at least one missing/empty value.
 */
QVector<int> Spreadsheet::rowsWithMissingValues() const {
	QVector<int> rows;
	const auto& columns = children<Column>();
	for (int row = 0; row < rowCount(); ++row) {
		for (const auto& col : columns) {
			if (col->asStringColumn()->textAt(row).isEmpty()) {
				rows << row;
				break;
			}
		}
	}

	return rows;
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
void Spreadsheet::setRowCount(int new_size, QUndoCommand* parent) {
	int current_size = rowCount();
	if (new_size > current_size)
		insertRows(current_size, new_size - current_size, parent);
	if (new_size < current_size && new_size >= 0)
		removeRows(new_size, current_size - new_size, parent);
}

void Spreadsheet::initConnectionsLinking(const Spreadsheet* sender, const Spreadsheet* receiver) {
	QObject::connect(sender, &Spreadsheet::aspectAboutToBeRemoved, receiver, &Spreadsheet::linkedSpreadsheetDeleted);
	QObject::connect(sender, &Spreadsheet::rowCountChanged, receiver, &Spreadsheet::linkedSpreadsheetNewRowCount);
}

class SpreadsheetSetLinkingCmd : public QUndoCommand {
public:
	SpreadsheetSetLinkingCmd(Spreadsheet::Private* target,
							 const Spreadsheet::Linking& newValue,
							 const KLocalizedString& description,
							 QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_target(target)
		, m_linking(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	void execute() {
		if (m_target->linking.linkedSpreadsheet)
			QObject::disconnect(m_target->linking.linkedSpreadsheet, nullptr, m_target->q, nullptr);

		if (m_linking.linkedSpreadsheet) {
			m_linking.linkedSpreadsheetPath = m_linking.linkedSpreadsheet->path();
			m_target->q->initConnectionsLinking(m_linking.linkedSpreadsheet, m_target->q);
		}

		const Spreadsheet::Linking l = m_target->linking;
		m_target->linking = m_linking;
		m_linking = l;
	}

	virtual void redo() override {
		execute();
		QUndoCommand::redo();
		finalize();
	}

	virtual void undo() override {
		execute();
		QUndoCommand::undo();
		finalize();
	}

	void finalize() const {
		Q_EMIT m_target->q->linkingChanged(m_target->linking.linking);
		Q_EMIT m_target->q->linkedSpreadsheetChanged(m_target->linking.linkedSpreadsheet);
	}

private:
	Spreadsheet::Private* m_target;
	Spreadsheet::Linking m_linking;
};

BASIC_SHARED_D_READER_IMPL(Spreadsheet, bool, linking, linking.linking)
void Spreadsheet::setLinking(bool linking) {
	Q_D(Spreadsheet);
	if (linking != d->linking.linking) {
		Linking l = d->linking;
		l.linking = linking;
		auto parent = new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linking"));
		if (linking && d->linking.linkedSpreadsheet)
			setRowCount(d->linking.linkedSpreadsheet->rowCount(), parent);
		exec(parent);
	}
}

BASIC_SHARED_D_READER_IMPL(Spreadsheet, const Spreadsheet*, linkedSpreadsheet, linking.linkedSpreadsheet)
void Spreadsheet::setLinkedSpreadsheet(const Spreadsheet* linkedSpreadsheet, bool skipUndo) {
	Q_D(Spreadsheet);
	if (!d->linking.linking)
		return; // Do not allow setting a spreadsheet when linking is disabled

	if (linkedSpreadsheet != d->linking.linkedSpreadsheet) {
		if (skipUndo) {
			d->linking.linkedSpreadsheet = linkedSpreadsheet;
			initConnectionsLinking(linkedSpreadsheet, this);
		} else {
			Linking l = d->linking;
			l.linkedSpreadsheet = linkedSpreadsheet;
			auto* parent = new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linked spreadsheet"));
			if (d->linking.linking && linkedSpreadsheet)
				setRowCount(linkedSpreadsheet->rowCount(), parent);
			exec(parent);
		}
	}
}

QString Spreadsheet::linkedSpreadsheetPath() const {
	Q_D(const Spreadsheet);
	return d->linking.spreadsheetPath();
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
Column* Spreadsheet::column(const QString& name) const {
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

class SpreadsheetSetColumnsCountCmd : public QUndoCommand {
public:
	SpreadsheetSetColumnsCountCmd(Spreadsheet* spreadsheet, bool insert, int first, int count, QUndoCommand* parent)
		: QUndoCommand(parent)
		, m_spreadsheet(spreadsheet)
		, m_insert(insert)
		, m_first(first)
		, m_last(first + count - 1) {
		if (insert)
			setText(i18np("%1: insert 1 column", "%1: insert %2 columns", spreadsheet->name(), count));
		else
			setText(i18np("%1: remove 1 column", "%1: remove %2 columns", spreadsheet->name(), count));
	}

	virtual void redo() override {
		WAIT_CURSOR;
		if (m_insert)
			Q_EMIT m_spreadsheet->aspectsAboutToBeInserted(m_first, m_last);
		else
			Q_EMIT m_spreadsheet->aspectsAboutToBeRemoved(m_first, m_last);

		QUndoCommand::redo();

		if (m_insert)
			Q_EMIT m_spreadsheet->aspectsInserted(m_first, m_last);
		else
			Q_EMIT m_spreadsheet->aspectsRemoved();
		RESET_CURSOR;
		m_spreadsheet->emitColumnCountChanged();
	}

	virtual void undo() override {
		WAIT_CURSOR;
		if (m_insert)
			Q_EMIT m_spreadsheet->aspectsAboutToBeRemoved(m_first, m_last);
		else
			Q_EMIT m_spreadsheet->aspectsAboutToBeInserted(m_first, m_last);
		QUndoCommand::undo();

		if (m_insert)
			Q_EMIT m_spreadsheet->aspectsRemoved();
		else
			Q_EMIT m_spreadsheet->aspectsInserted(m_first, m_last);
		RESET_CURSOR;
		m_spreadsheet->emitColumnCountChanged();
	}

private:
	Spreadsheet* m_spreadsheet;
	bool m_insert;
	int m_first;
	int m_last;
};

void Spreadsheet::removeColumns(int first, int count, QUndoCommand* parent) {
	if (count < 1 || first < 0 || first + count > columnCount())
		return;

	auto* command = new SpreadsheetSetColumnsCountCmd(this, false, first, count, parent);
	bool execute = false;
	if (!parent) {
		execute = true;
		parent = command;
	}

	const auto& columns = children<Column>();
	for (int i = (first + count - 1); i >= first; i--)
		columns.at(i)->remove(parent);

	if (execute)
		exec(command);
}

void Spreadsheet::insertColumns(int before, int count, QUndoCommand* parent) {
	auto* command = new SpreadsheetSetColumnsCountCmd(this, true, before, count, parent);
	bool execute = false;
	if (!parent) {
		execute = true;
		parent = command;
	}
	const int cols = columnCount();
	const int rows = rowCount();
	for (int i = 0; i < count; i++) {
		auto* new_col = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
		new_col->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		new_col->insertRows(0, rows);
		insertChild(new_col, before + i, parent);
	}

	if (execute)
		exec(command);
}

/*!
  Sets the number of columns to \c new_size
*/
void Spreadsheet::setColumnCount(int new_size, QUndoCommand* parent) {
	int old_size = columnCount();
	if (old_size == new_size || new_size < 0)
		return;

	if (new_size < old_size)
		removeColumns(new_size, old_size - new_size, parent);
	else
		insertColumns(old_size, new_size - old_size, parent);
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

void Spreadsheet::clear(const QVector<Column*>& columns) {
	auto* parent = new LongExecutionCmd(i18n("%1: clear selected columns", name()));

	// 	if (formulaModeActive()) {
	// 		for (auto* col : selectedColumns()) {
	// 			col->setSuppressDataChangedSignal(true);
	// 			col->clearFormulas();
	// 			col->setSuppressDataChangedSignal(false);
	// 			col->setChanged();
	// 		}
	// 	} else {
	for (auto* col : columns) {
		col->setSuppressDataChangedSignal(true);
		col->clear(parent);
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
	exec(parent);
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
	if (type() != AspectType::StatisticsSpreadsheet)
		Q_EMIT requestProjectContextMenu(menu);
	else {
		menu->addSeparator();
		auto* action = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete"), this);
		connect(action, &QAction::triggered, this, [=]() {
			auto* parentSpreadsheet = static_cast<Spreadsheet*>(parentAspect());
			parentSpreadsheet->toggleStatisticsSpreadsheet(false);
		});
		menu->addAction(action);
	}
	return menu;
}

void Spreadsheet::fillColumnContextMenu(QMenu* menu, Column* column) {
#ifndef SDK
	if (m_view)
		m_view->fillColumnContextMenu(menu, column);
#else
	Q_UNUSED(menu)
	Q_UNUSED(column)
#endif
}

void Spreadsheet::moveColumn(int from, int to) {
	const auto& columns = children<Column>();
	auto* col = columns.at(from);
	beginMacro(i18n("%1: move column %2 from position %3 to %4.", name(), col->name(), from + 1, to + 1));
	col->remove();
	insertChildBefore(col, columns.at(to));
	endMacro();
}

// FIXME: replace index-based API with Column*-based one
/*!
  Determines the corresponding X column.
*/
int Spreadsheet::colX(int col) {
	for (int i = col - 1; i >= 0; i--) {
		if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::X)
			return i;
	}
	int cols = columnCount();
	for (int i = col + 1; i < cols; i++) {
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

	if (column(col)->plotDesignation() == AbstractColumn::PlotDesignation::XError
		|| column(col)->plotDesignation() == AbstractColumn::PlotDesignation::YError) {
		// look to the left first
		for (int i = col - 1; i >= 0; i--) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
		for (int i = col + 1; i < cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
	} else {
		// look to the right first
		for (int i = col + 1; i < cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::PlotDesignation::Y)
				return i;
		}
		for (int i = col - 1; i >= 0; i--) {
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
	DEBUG(Q_FUNC_INFO << ", ascending = " << ascending)
	if (cols.isEmpty())
		return;

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

	if (!leading) { // sort separately
		DEBUG("	sort separately")
		for (auto* col : cols) {
			int rows = col->rowCount();
			std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), col->columnMode()));

			switch (col->columnMode()) {
			case AbstractColumn::ColumnMode::Double: {
				QVector<QPair<double, int>> map;

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
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}
				break;
			}
			case AbstractColumn::ColumnMode::Integer: {
				QVector<QPair<int, int>> map;

				for (int i = 0; i < rows; i++)
					map.append(QPair<int, int>(col->valueAt(i), i));

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::integerLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::integerGreater);

				// put the values in the right order into tempCol
				for (int i = 0; i < rows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}
				break;
			}
			case AbstractColumn::ColumnMode::BigInt: {
				QVector<QPair<qint64, int>> map;

				for (int i = 0; i < rows; i++)
					map.append(QPair<qint64, int>(col->valueAt(i), i));

				if (ascending)
					std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntLess);
				else
					std::stable_sort(map.begin(), map.end(), CompareFunctions::bigIntGreater);

				// put the values in the right order into tempCol
				for (int i = 0; i < rows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
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
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}
				break;
			}
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day: {
				QVector<QPair<QDateTime, int>> map;

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
					// too slow: tempCol->copy(col, idx, i, 1);
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
		case AbstractColumn::ColumnMode::Double: {
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
				std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), columnMode));
				// put the values in correct order into tempCol
				for (int i = 0; i < filledRows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}

				// copy the sorted column
				if (col == leading) // update all rows
					col->copy(tempCol.get(), 0, 0, rows);
				else { // do not overwrite unused cols
					std::unique_ptr<Column> tempInvalidCol(new Column(QStringLiteral("temp2"), col->columnMode()));
					for (int i = 0; i < invalidRows; i++) {
						const int idx = invalidIndex.at(i);
						// too slow: tempInvalidCol->copy(col, idx, i, 1);
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
			// TODO: check if still working when invalid integer entries are supported
			QVector<QPair<int, int>> map;

			for (int i = 0; i < rows; i++)
				map.append(QPair<int, int>(leading->valueAt(i), i));

			if (ascending)
				std::stable_sort(map.begin(), map.end(), CompareFunctions::integerLess);
			else
				std::stable_sort(map.begin(), map.end(), CompareFunctions::integerGreater);

			for (auto* col : cols) {
				std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), col->columnMode()));
				// put the values in the right order into tempCol
				for (int i = 0; i < rows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
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
				std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), col->columnMode()));
				// put the values in the right order into tempCol
				for (int i = 0; i < rows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
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
			// QDEBUG("	empty indices: " << emptyIndex)
			const int filledRows = map.size();
			const int emptyRows = emptyIndex.size();

			if (ascending)
				std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringLess);
			else
				std::stable_sort(map.begin(), map.end(), CompareFunctions::QStringGreater);

			for (auto* col : cols) {
				std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), col->columnMode()));
				// put the values in the right order into tempCol
				for (int i = 0; i < filledRows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}

				// copy the sorted column
				if (col == leading) // update all rows
					col->copy(tempCol.get(), 0, 0, rows);
				else { // do not overwrite unused cols
					std::unique_ptr<Column> tempEmptyCol(new Column(QStringLiteral("temp2"), col->columnMode()));
					for (int i = 0; i < emptyRows; i++) {
						const int idx = emptyIndex.at(i);
						// too slow: tempEmptyCol->copy(col, idx, i, 1);
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
				std::unique_ptr<Column> tempCol(new Column(QStringLiteral("temp"), col->columnMode()));
				// put the values in the right order into tempCol
				for (int i = 0; i < filledRows; i++) {
					int idx = map.at(i).second;
					// too slow: tempCol->copy(col, idx, i, 1);
					tempCol->setFromColumn(i, col, idx);
					tempCol->setMasked(col->isMasked(idx));
				}
				// copy the sorted column
				if (col == leading) // update all rows
					col->copy(tempCol.get(), 0, 0, rows);
				else { // do not overwrite unused cols
					std::unique_ptr<Column> tempInvalidCol(new Column(QStringLiteral("temp2"), col->columnMode()));
					for (int i = 0; i < invalidRows; i++) {
						const int idx = invalidIndex.at(i);
						// too slow: tempInvalidCol->copy(col, idx, i, 1);
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
	return QIcon::fromTheme(QStringLiteral("labplot-spreadsheet"));
}

/*!
  Returns the text displayed in the given cell.
*/
QString Spreadsheet::text(int row, int col) const {
	Column* c = column(col);
	if (!c)
		return {};

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
		Q_EMIT columnSelected(index);
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
		Q_EMIT columnDeselected(index);
	}
}

void Spreadsheet::linkedSpreadsheetDeleted() {
	Q_D(Spreadsheet);
	Linking l = d->linking;
	l.linkedSpreadsheet = nullptr;
	exec(new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: linked spreadsheet removed")));
}

void Spreadsheet::linkedSpreadsheetNewRowCount(int rowCount) {
	setRowCount(rowCount);
}

/*!
 *  Emits the signal to select or to deselect the column number \c index in the project explorer,
 *  if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 * This function is called in \c SpreadsheetView upon selection changes.
 */
void Spreadsheet::setColumnSelectedInView(int index, bool selected) {
	if (selected) {
		Q_EMIT childAspectSelectedInView(child<Column>(index));

		// deselect the spreadsheet in the project explorer, if a child (column) was selected
		// and also all possible parents like folder, workbook, datapicker curve, datapicker
		// to prevents unwanted multiple selection in the project explorer
		// if one of the parents of the selected column was also selected before.
		AbstractAspect* parent = this;
		while (parent) {
			Q_EMIT childAspectDeselectedInView(parent);
			parent = parent->parentAspect();
		}
	} else
		Q_EMIT childAspectDeselectedInView(child<Column>(index));
}

QVector<AspectType> Spreadsheet::pasteTypes() const {
	return QVector<AspectType>{AspectType::Column};
}

QVector<AspectType> Spreadsheet::dropableOn() const {
	auto vec = AbstractPart::dropableOn();
	vec << AspectType::Workbook;
	return vec;
}

void Spreadsheet::toggleStatisticsSpreadsheet(bool on) {
	Q_D(Spreadsheet);
	if (on) {
		if (d->statisticsSpreadsheet)
			return;

		d->statisticsSpreadsheet = new StatisticsSpreadsheet(this);
		addChildFast(d->statisticsSpreadsheet);
	} else {
		if (!d->statisticsSpreadsheet)
			return;

		setUndoAware(false);
		removeChild(d->statisticsSpreadsheet);
		setUndoAware(true);
		d->statisticsSpreadsheet = nullptr;
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void Spreadsheet::save(QXmlStreamWriter* writer) const {
	Q_D(const Spreadsheet);
	writer->writeStartElement(QStringLiteral("spreadsheet"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QLatin1String("linking"));
	writer->writeAttribute(QStringLiteral("enabled"), QString::number(d->linking.linking));
	writer->writeAttribute(QStringLiteral("spreadsheet"), d->linking.spreadsheetPath());
	writer->writeEndElement();

	// columns
	const auto& columns = children<Column>(ChildIndexFlag::IncludeHidden);
	for (auto* column : columns)
		column->save(writer);

	// statistics spreadsheet, if available
	if (d->statisticsSpreadsheet)
		d->statisticsSpreadsheet->save(writer);

	writer->writeEndElement(); // "spreadsheet"
}

/*!
  Loads from XML.
*/
bool Spreadsheet::load(XmlStreamReader* reader, bool preview) {
	Q_D(Spreadsheet);
	if (!readBasicAttributes(reader))
		return false;

	QString str;
	QXmlStreamAttributes attribs;

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QLatin1String("spreadsheet"))
			break;

		if (reader->isStartElement()) {
			if (reader->name() == QLatin1String("comment")) {
				if (!readCommentElement(reader))
					return false;
			} else if (reader->name() == QLatin1String("linking")) {
				attribs = reader->attributes();
				str = attribs.value(QStringLiteral("enabled")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("enabled"));
				else
					d->linking.linking = static_cast<bool>(str.toInt());

				str = attribs.value(QStringLiteral("spreadsheet")).toString();
				d->linking.linkedSpreadsheetPath = str;
			} else if (reader->name() == QLatin1String("column")) {
				Column* column = new Column(QString());
				column->setIsLoading(true);
				if (!column->load(reader, preview)) {
					delete column;
					setColumnCount(0);
					return false;
				}
				addChildFast(column);
			} else if (reader->name() == QLatin1String("statisticsSpreadsheet")) {
				d->statisticsSpreadsheet = new StatisticsSpreadsheet(this, true);
				if (!d->statisticsSpreadsheet->load(reader, preview)) {
					delete d->statisticsSpreadsheet;
					d->statisticsSpreadsheet = nullptr;
				} else
					addChildFast(d->statisticsSpreadsheet);
			} else { // unknown element
				reader->raiseUnknownElementWarning();
				if (!reader->skipToEndElement())
					return false;
			}
		}
	}

	return !reader->hasError();
}

// ##############################################################################
// ########################  Data Import  #######################################
// ##############################################################################
int Spreadsheet::prepareImport(std::vector<void*>& dataContainer,
							   AbstractFileFilter::ImportMode importMode,
							   int actualRows,
							   int actualCols,
							   const QStringList& colNameList,
							   const QVector<AbstractColumn::ColumnMode>& columnMode,
							   bool& ok,
							   bool initializeContainer) {
	Q_D(Spreadsheet);
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	DEBUG(Q_FUNC_INFO << ", resize spreadsheet to rows = " << actualRows << " and cols = " << actualCols)
	QDEBUG(Q_FUNC_INFO << ", column name list = " << colNameList)
	assert(d->m_usedInPlots.size() == 0);
	int columnOffset = 0;
	setUndoAware(false);
	if (m_model != nullptr)
		m_model->suppressSignals(true);

	// make the available columns undo unaware before we resize and rename them below,
	// the same will be done for new columns in this->resize().
	{
		const auto& columns = children<Column>();
		for (auto* column : qAsConst(columns))
			column->setUndoAware(false);
	}

	columnOffset = this->resize(importMode, colNameList, actualCols);
	if (initializeContainer)
		dataContainer.resize(actualCols);
	const auto& columns = children<Column>(); // Get new children because of the resize it might be different

	// resize the spreadsheet
	if (initializeContainer) {
		try {
			if (importMode == AbstractFileFilter::ImportMode::Replace) {
				clear();
				setRowCount(actualRows);
			} else {
				if (rowCount() < actualRows)
					setRowCount(actualRows);
			}
		} catch (std::bad_alloc&) {
			ok = false;
			return 0;
		}
	}

	if (columnMode.size() < actualCols) {
		QDEBUG(Q_FUNC_INFO << ", columnMode[] size " << columnMode.size() << " is too small, should be " << actualCols << "! Giving up.");
		return -1;
	}

	for (int n = 0; n < actualCols; n++) {
		// data() returns a void* which is a pointer to any data type (see ColumnPrivate.cpp)
		auto* column = columns.at(columnOffset + n);
		DEBUG(" column " << n << " columnMode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode[n]));
		column->setColumnModeFast(columnMode[n]);

		// in most cases the first imported column is meant to be used as x-data.
		// Other columns provide mostly y-data or errors.
		// TODO: this has to be configurable for the user in the import widget,
		// it should be possible to specify x-error plot designation, etc.
		if (n == 0 && importMode == AbstractFileFilter::ImportMode::Replace)
			column->setPlotDesignation(AbstractColumn::PlotDesignation::X);
		else
			column->setPlotDesignation(AbstractColumn::PlotDesignation::Y);

		if (initializeContainer) {
			switch (columnMode[n]) {
			case AbstractColumn::ColumnMode::Double: {
				auto* vector = static_cast<QVector<double>*>(column->data());
				dataContainer[n] = static_cast<void*>(vector);
				break;
			}
			case AbstractColumn::ColumnMode::Integer: {
				auto* vector = static_cast<QVector<int>*>(column->data());
				dataContainer[n] = static_cast<void*>(vector);
				break;
			}
			case AbstractColumn::ColumnMode::BigInt: {
				auto* vector = static_cast<QVector<qint64>*>(column->data());
				dataContainer[n] = static_cast<void*>(vector);
				break;
			}
			case AbstractColumn::ColumnMode::Text: {
				auto* vector = static_cast<QVector<QString>*>(column->data());
				dataContainer[n] = static_cast<void*>(vector);
				break;
			}
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime: {
				auto* vector = static_cast<QVector<QDateTime>*>(column->data());
				dataContainer[n] = static_cast<void*>(vector);
				break;
			}
			}
		} else {
			// Assign already allocated datacontainer to the column
			column->setData(dataContainer[n]);
		}
	}
	//	QDEBUG("dataPointers =" << dataPointers);

	// DEBUG(Q_FUNC_INFO << ", DONE");

	ok = true;
	return columnOffset;
}

/*!
	resize data source to cols columns
	returns column offset depending on import mode
*/
int Spreadsheet::resize(AbstractFileFilter::ImportMode mode, const QStringList& names, int cols) {
	//	PERFTRACE(Q_FUNC_INFO);
	DEBUG(Q_FUNC_INFO << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, mode) << ", cols = " << cols)
	// QDEBUG("	column name list = " << colNameList)

	Q_EMIT aboutToResize(); // call this to disable the retransforms in worksheet elements in Project

	// make sure the column names provided by the user don't have any duplicates
	QStringList uniqueNames;
	if (names.count() > 1) {
		uniqueNames << names.at(0);
		for (int i = 1; i < names.count(); ++i)
			uniqueNames << AbstractAspect::uniqueNameFor(names.at(i), uniqueNames);
	} else
		uniqueNames = names;

	// if the number of provided column names is smaller than the number of columns to be created,
	// create standard names
	for (int k = uniqueNames.size(); k < cols; k++)
		uniqueNames.append(QStringLiteral("Column ") + QString::number(k + 1));

	int columnOffset = 0; // indexes the "start column" in the spreadsheet. Starting from this column the data will be imported.

	Column* newColumn = nullptr;
	int rows = rowCount();
	if (mode == AbstractFileFilter::ImportMode::Append) {
		columnOffset = childCount<Column>();
		for (int n = 0; n < cols; n++) {
			newColumn = new Column(uniqueNames.at(n), AbstractColumn::ColumnMode::Double);
			newColumn->resizeTo(rows);
			newColumn->setUndoAware(false);
			newColumn->resizeTo(rows);
			addChild(newColumn);
		}
	} else if (mode == AbstractFileFilter::ImportMode::Prepend) {
		Column* firstColumn = child<Column>(0);
		for (int n = 0; n < cols; n++) {
			newColumn = new Column(uniqueNames.at(n), AbstractColumn::ColumnMode::Double);
			newColumn->resizeTo(rows);
			newColumn->setUndoAware(false);
			newColumn->resizeTo(rows);
			insertChildBefore(newColumn, firstColumn);
		}
	} else if (mode == AbstractFileFilter::ImportMode::Replace) {
		// replace completely the previous content of the data source with the content to be imported.
		int columnsCount = childCount<Column>();

		if (columnsCount > cols) {
			// there are more columns in the data source than required -> remove the superfluous columns
			for (int i = 0; i < columnsCount - cols; i++)
				removeChild(child<Column>(0));
		} else {
			// create additional columns if needed
			if (cols - columnsCount > 30)
				Q_EMIT manyAspectsAboutToBeInserted();
			Q_EMIT aspectsAboutToBeInserted(columnsCount, cols - 1);
			for (int i = columnsCount; i < cols; i++) {
				newColumn = new Column(uniqueNames.at(i), AbstractColumn::ColumnMode::Double);
				newColumn->resizeTo(rows);
				newColumn->setUndoAware(false);
				newColumn->resizeTo(rows);
				addChildFast(newColumn); // in the replace mode, we can skip checking the uniqueness of the names and use the "fast" method
			}
			Q_EMIT aspectsInserted(columnsCount, cols - 1);
		}

		// 1. if the column name has changed, call Column::reset() to disconnect all dependent objects from the dataChanged signal
		// 2. suppress the dataChanged signal for all columns (will be restored later in finalizeImport())
		// 3. rename the columns that were already available
		// 4. column->aspectDescriptionChanged() to trigger the update of the dependencies on column in Project.
		const auto& columns = children<Column>();
		int index = 0;
		Q_D(Spreadsheet);
		for (auto* column : columns) {
			column->setSuppressDataChangedSignal(true);
			const auto& newName = uniqueNames.at(index);
			if (column->name() != newName) {
				column->addUsedInPlots(d->m_usedInPlots);
				column->reset();
				column->setName(newName, AbstractAspect::NameHandling::UniqueNotRequired);
				column->aspectDescriptionChanged(column);
			}
			++index;
		}
	}

	Q_EMIT resizeFinished(); // call this to re-enable the retransforms in worksheet elements in Project

	return columnOffset;
}

void Spreadsheet::finalizeImport(size_t columnOffset,
								 size_t startColumn,
								 size_t endColumn,
								 const QString& dateTimeFormat,
								 AbstractFileFilter::ImportMode importMode) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	Q_D(Spreadsheet);
	// DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << " / " << endColumn);

	// determine the dependent plots
	if (importMode == AbstractFileFilter::ImportMode::Replace) {
		for (size_t n = startColumn; n <= endColumn; n++) {
			auto* column = this->column((int)(columnOffset + n - startColumn));
			if (column)
				column->addUsedInPlots(d->m_usedInPlots);
		}
	}

	if (importMode == AbstractFileFilter::ImportMode::Replace) {
		// suppress retransform in the dependent plots
		for (auto* plot : d->m_usedInPlots)
			plot->setSuppressRetransform(true);
	}

	// set the comments for each of the columns if datasource is a spreadsheet
	const int rows = rowCount();
	for (size_t col = startColumn; col <= endColumn; col++) {
		// DEBUG(Q_FUNC_INFO << ", column " << columnOffset + col - startColumn);
		Column* column = this->column((int)(columnOffset + col - startColumn));
		DEBUG(Q_FUNC_INFO << ", type " << ENUM_TO_STRING(AbstractColumn, ColumnMode, column->columnMode()))

		QString comment;
		switch (column->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			comment = i18np("double precision data, %1 element", "numerical data, %1 elements", rows);
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
		// retransform the dependent plots
		for (auto* plot : d->m_usedInPlots) {
			plot->setSuppressRetransform(false);
			plot->dataChanged(-1, -1); // TODO: check if all ranges must be updated
		}
	}
	d->m_usedInPlots.clear();

	// make the spreadsheet and all its children undo aware again
	setUndoAware(true);
	for (int i = 0; i < childCount<Column>(); i++)
		child<Column>(i)->setUndoAware(true);

	if (m_model)
		m_model->suppressSignals(false);

#ifndef SDK
	if (m_partView && m_view)
		m_view->resizeHeader();
#endif

	// row count most probably changed after the import, notify the dock widget.
	// no need to notify about the column count change, this is already done by add/removeChild signals
	Q_EMIT rowCountChanged(rowCount());

	// DEBUG(Q_FUNC_INFO << " DONE");
}

void Spreadsheet::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	const auto* sh = dynamic_cast<const Spreadsheet*>(aspect);
	if (sh && linkedSpreadsheetPath() == aspectPath) {
		setUndoAware(false);
		setLinkedSpreadsheet(sh);
		setUndoAware(true);
	}
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
SpreadsheetPrivate::SpreadsheetPrivate(Spreadsheet* owner)
	: q(owner) {
}

QString SpreadsheetPrivate::name() const {
	return q->name();
}
