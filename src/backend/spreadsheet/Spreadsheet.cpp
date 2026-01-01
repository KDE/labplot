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
#include "backend/core/Project.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "frontend/spreadsheet/SpreadsheetView.h"

#include <KConfig>
#include <KConfigGroup>

#include <QMenu>
#include <QTimer>
#include <QXmlStreamWriter>

/*!
  \class Spreadsheet
  \brief Aspect providing a spreadsheet table with column logic.

  Spreadsheet is a container object for columns with no data of its own. By definition, it's columns
  are all of its children inheriting from class Column. Thus, the basic API is already defined
  by AbstractAspect (managing the list of columns, notification of column insertion/removal)
  and Column (changing and monitoring state of the actual data).

  Spreadsheet is a data container for related columns to be treated as a single entity.

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

/*!
 * @brief Constructor.
 *
 * Constructs a Spreadsheet with default 2 columns each with 100 rows.
 *
 * @param name The Spreadsheet name.
 */
Spreadsheet::Spreadsheet(const QString& name, bool loading, AspectType type)
	: AbstractDataSource(name, type)
	, d_ptr(new SpreadsheetPrivate(this)) {
	static_assert(AbstractAspect::typeName(AspectType::Spreadsheet) == "Spreadsheet");
	if (!loading)
		init();

	connect(this, &Spreadsheet::columnCountChanged, this, &Spreadsheet::initConnectionsRowCountChanges);
}

/*!
 * @brief Destructor.
 *
 * Destroys the Spreadsheet and its child columns.
 */
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

	Q_D(Spreadsheet);
	d->showComments = group.readEntry(QLatin1String("ShowComments"), false);
	d->showSparklines = group.readEntry(QLatin1String("ShowSparklines"), false);

	const int columns = group.readEntry(QLatin1String("ColumnCount"), 2);
	const int rows = group.readEntry(QLatin1String("RowCount"), 100);

	for (int i = 0; i < columns; i++) {
		Column* new_col = new Column(QString::number(i + 1), AbstractColumn::ColumnMode::Double);
		new_col->setPlotDesignation(i == 0 ? AbstractColumn::PlotDesignation::X : AbstractColumn::PlotDesignation::Y);
		addChild(new_col);
	}
	setRowCount(rows);
	initConnectionsRowCountChanges();
}

/*!
 * connects to the signals emitted in the first column to react on the row count changes that are
 * done internally in Column and to emit the corresponding signals in Spreadsheet.
 * called initially and on column count changes (columns inserts/removals).
 */
void Spreadsheet::initConnectionsRowCountChanges() {
	if (columnCount() == 0)
		return;

	// check first if the first column was changed
	Q_D(Spreadsheet);
	auto* firstColumn = children<Column>().first();
	if (d->firstColumn == firstColumn)
		return;
	else {
		if (d->firstColumn)
			disconnect(d->firstColumn, nullptr, this, nullptr);
		d->firstColumn = firstColumn;
	}

	// handle row insertions
	connect(d->firstColumn, &AbstractColumn::rowsAboutToBeInserted, this, [=](const AbstractColumn*, int before, int count) {
		Q_EMIT rowsAboutToBeInserted(before, before + count - 1);
	});
	connect(d->firstColumn, &AbstractColumn::rowsInserted, this, [=](const AbstractColumn* sender, int, int) {
		Q_EMIT rowsInserted(sender->rowCount());
		Q_EMIT rowCountChanged(sender->rowCount());
	});

	// handle row removals
	connect(d->firstColumn, &AbstractColumn::rowsAboutToBeRemoved, this, [=](const AbstractColumn*, int first, int count) {
		Q_EMIT rowsAboutToBeRemoved(first, first + count - 1);
	});
	connect(d->firstColumn, &AbstractColumn::rowsRemoved, this, [=](const AbstractColumn* sender, int, int) {
		Q_EMIT rowsRemoved(sender->rowCount());
		Q_EMIT rowCountChanged(sender->rowCount());
	});
}

void Spreadsheet::setSuppressSetCommentFinalizeImport(bool suppress) {
	Q_D(Spreadsheet);
	d->suppressSetCommentFinalizeImport = suppress;
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
		Q_D(const Spreadsheet);
		m_view = new SpreadsheetView(const_cast<Spreadsheet*>(this), d->readOnly);
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

void Spreadsheet::setReadOnly(const bool value) {
	Q_D(Spreadsheet);
	d->readOnly = value;
}

bool Spreadsheet::readOnly() const {
	Q_D(const Spreadsheet);
	return d->readOnly;
}

QString Spreadsheet::caption() const {
	QString caption = AbstractAspect::caption();
	caption += QLatin1String("<br><br>") + i18n("Columns: %1", columnCount());
	caption += QLatin1String("<br>") + i18n("Rows: %1", rowCount());

	// add the information about the usage of spreadsheet columns in other places
	const auto* project = this->project();
	const auto& columns = children<Column>();
	QSet<const AbstractAspect*> usedInAspects;

	// add plots where the column is currently in use
	const auto& plots = project->children<Plot>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* col : columns) {
		for (const auto* plot : plots) {
			const bool used = plot->usingColumn(col, true);
			if (used)
				usedInAspects << plot;
		}
	}

	if (!usedInAspects.isEmpty()) {
		caption += QStringLiteral("<br><br><b>") + i18n("Used in Plots:") + QStringLiteral("</b>");
		for (auto* aspect : usedInAspects)
			caption += QStringLiteral("<br>") + aspect->path();
	}

	// add axes where the column is used as a custom column for ticks positions or labels
	usedInAspects.clear();
	const auto& axes = project->children<Axis>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* col : columns) {
		for (const auto* axis : axes) {
			const bool used = (axis->majorTicksColumn() == col || axis->minorTicksColumn() == col || axis->labelsTextColumn() == col);
			if (used)
				usedInAspects << axis;
		}
	}

	if (!usedInAspects.isEmpty()) {
		caption += QStringLiteral("<br><br><b>") + i18n("Used in Axes:") + QStringLiteral("</b>");
		for (auto* aspect : usedInAspects)
			caption += QStringLiteral("<br>") + aspect->path();
	}

	// add calculated columns where the column is used in formula variables
	usedInAspects.clear();
	const auto& columnsAll = project->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* col : columns) {
		const QString& path = col->path();
		for (const auto* colOther : columnsAll) {
			for (int i = 0; i < colOther->formulaData().count(); i++) {
				if (path == colOther->formulaData().at(i).columnName()) {
					usedInAspects << colOther;
					break;
				}
			}
		}
	}

	if (!usedInAspects.isEmpty()) {
		caption += QStringLiteral("<br><br><b>") + i18n("Used in Spreadsheet Calculations:") + QStringLiteral("</b>");
		for (auto* aspect : usedInAspects)
			caption += QStringLiteral("<br>") + aspect->path();
	}

	return caption;
}

/*!
 * Returns a pointer to the StatisticsSpreadsheet for the current Spreadsheet if exists or nullptr.
 * @see StatisticsSpreadsheet()
 * @see toggleStatisticsSpreadsheet(bool)
 * @return StatisticsSpreadsheet* or nullptr
 */
StatisticsSpreadsheet* Spreadsheet::statisticsSpreadsheet() const {
	Q_D(const Spreadsheet);
	return d->statisticsSpreadsheet;
}

/*!
 * \brief Called when the application settings were changed.
 *  adjusts the appearance of the spreadsheet header.
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

/*!
 * Updates locale for all columns from QLocale.
 */
void Spreadsheet::updateLocale() {
	for (auto* col : children<Column>())
		col->updateLocale();
}

/*!
 * Grows/shrinks the number of rows in the spreadsheet to \c new_size.
 * @param new_size The new number of rows in the spreadsheet.
 */
void Spreadsheet::setRowCount(int new_size) {
	int current_size = rowCount();
	if (new_size > current_size)
		insertRows(current_size, new_size - current_size);
	if (new_size < current_size && new_size >= 0)
		removeRows(new_size, current_size - new_size);
}

/*!
 * Returns the number of rows in the \c Spreadsheet.
 * @return The number of rows in the \c Spreadsheet.
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

/*!
 * Removes \c count rows starting from the \c first row index in the spreadsheet.
 * @param count The number of rows to remove.
 * @param first The row index to start removing rows from.
 */
void Spreadsheet::removeRows(int first, int count) {
	if (count < 1 || first < 0 || first + count > rowCount())
		return;

	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18np("%1: remove 1 row", "%1: remove %2 rows", name(), count));
	for (auto* col : children<Column>())
		col->removeRows(first, count);
	endMacro();
}

/*!
 * Inserts \c count rows before the \c before row index in the spreadsheet.
 * @param count The number of rows to insert.
 * @param before The row index before which the rows are inserted.
 */
void Spreadsheet::insertRows(int before, int count) {
	if (count < 1 || before < 0 || before > rowCount())
		return;

	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18np("%1: insert 1 row", "%1: insert %2 rows", name(), count));
	for (auto* col : children<Column>())
		col->insertRows(before, count);
	endMacro();
}

/*!
 * Inserts \c count rows before the last row index in the spreadsheet.
 * @param count The number of rows to insert.
 */
void Spreadsheet::appendRows(int count) {
	insertRows(rowCount(), count);
}

/*!
 * Inserts a row before the last row index in the spreadsheet.
 */
void Spreadsheet::appendRow() {
	insertRows(rowCount(), 1);
}

/*!
 * Removes all rows in the spreadsheet in which the value of one or more of its columns is missing/empty.
 */
void Spreadsheet::removeEmptyRows() {
	const auto& rows = rowsWithMissingValues();
	if (rows.isEmpty())
		return;

	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18n("%1: remove rows with missing values", name()));

	for (int row = rows.count() - 1; row >= 0; --row)
		removeRows(rows.at(row), 1);

	endMacro();
}

/*!
 * Masks all rows in the spreadsheet in which the value of one or more of its columns is missing/empty.
 */
void Spreadsheet::maskEmptyRows() {
	const auto& rows = rowsWithMissingValues();
	if (rows.isEmpty())
		return;

	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18n("%1: mask rows with missing values", name()));

	const auto& columns = children<Column>();
	for (int row : rows) {
		for (const auto& col : columns)
			col->setMasked(row);
	}

	endMacro();
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

/*!
 * Inserts \c count columns before the last column index in the spreadsheet.
 * @param count The number of columns to insert.
 */
void Spreadsheet::appendColumns(int count) {
	insertColumns(columnCount(), count);
}

/*!
 * Inserts a column before the last column index in the spreadsheet.
 */
void Spreadsheet::appendColumn() {
	insertColumns(columnCount(), 1);
}

/*!
 * Inserts \c count columns before the first column index in the spreadsheet.
 * @param count The number of columns to insert.
 */
void Spreadsheet::prependColumns(int count) {
	insertColumns(0, count);
}

BASIC_SHARED_D_READER_IMPL(Spreadsheet, bool, showComments, showComments)
STD_SETTER_CMD_IMPL_F_S(Spreadsheet, SetShowComments, bool, showComments, updateCommentsHeader)
void Spreadsheet::setShowComments(bool showComments) {
	Q_D(Spreadsheet);
	if (d->showComments != showComments)
		exec(new SpreadsheetSetShowCommentsCmd(d, showComments, ki18n("%1: toggle comments header")));
}

BASIC_SHARED_D_READER_IMPL(Spreadsheet, bool, showSparklines, showSparklines)
STD_SETTER_CMD_IMPL_F_S(Spreadsheet, SetShowSparklines, bool, showSparklines, updateSparklinesHeader)
void Spreadsheet::setShowSparklines(bool showSparklines) {
	Q_D(Spreadsheet);
	if (d->showSparklines != showSparklines)
		exec(new SpreadsheetSetShowSparklinesCmd(d, showSparklines, ki18n("%1: toggle sparklines header")));
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

		const auto l = m_target->linking;
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

		if (linking && d->linking.linkedSpreadsheet) {
			beginMacro(i18n("%1: set linking", name()));
			exec(new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linking")));
			setRowCount(d->linking.linkedSpreadsheet->rowCount());
			endMacro();
		} else
			exec(new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linking")));
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
			if (d->linking.linking && linkedSpreadsheet) {
				beginMacro(i18n("%1: set linked spreadsheet", name()));
				setRowCount(linkedSpreadsheet->rowCount());
				exec(new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linked spreadsheet")));
				endMacro();
			} else
				exec(new SpreadsheetSetLinkingCmd(d, l, ki18n("%1: set linked spreadsheet")));
		}
	}
}

QString Spreadsheet::linkedSpreadsheetPath() const {
	Q_D(const Spreadsheet);
	return d->linking.spreadsheetPath();
}

/*!
 * Returns the \c Column at the index \c index.
 * @param index The zero-based index of the \c Column.
 * @return The \c Column at the index \c index.
 */
Column* Spreadsheet::column(int index) const {
	return child<Column>(index);
}

/*!
 * Returns a pointer to the \c Column with the name \c name.
 * @param name The \c Column name.
 * @return Pointer to the \c Column with the name \c name.
 */
Column* Spreadsheet::column(const QString& name) const {
	return child<Column>(name);
}

/*!
 * Returns the number of columns in the Spreadsheet.
 * @return The number of columns in the Spreadsheet.
 */
int Spreadsheet::columnCount() const {
	return childCount<Column>();
}

/*!
 * Grows/shrinks the number of columns in the spreadsheet to \c new_size.
 * @param new_size The new number of columns in the spreadsheet.
 */
void Spreadsheet::setColumnCount(int new_size) {
	int old_size = columnCount();
	if (old_size == new_size || new_size < 0)
		return;

	if (new_size < old_size)
		removeColumns(new_size, old_size - new_size);
	else
		insertColumns(old_size, new_size - old_size);
}

class SpreadsheetSetColumnsCountCmd : public QUndoCommand {
public:
	SpreadsheetSetColumnsCountCmd(Spreadsheet* spreadsheet, int oldCount, int newCount)
		: m_spreadsheet(spreadsheet)
		, m_oldCount(oldCount)
		, m_newCount(newCount) {
	}

	virtual void redo() override {
		Q_EMIT m_spreadsheet->columnCountChanged(m_newCount);
	}

	virtual void undo() override {
		qSwap(m_oldCount, m_newCount);
		redo();
	}

private:
	Spreadsheet* m_spreadsheet;
	int m_oldCount;
	int m_newCount;
};

/*!
 * Returns the number of columns in the \c Spreadsheet matching the plot designation.
 * @param pd The plot designation the columns are matched against.
 * @return The number of columns in the \c Spreadsheet matching the passed plot designation.
 */
int Spreadsheet::columnCount(AbstractColumn::PlotDesignation pd) const {
	int count = 0;
	for (auto* col : children<Column>())
		if (col->plotDesignation() == pd)
			count++;
	return count;
}

/*!
 * Removes \c count columns starting from the \c first column index in the spreadsheet.
 * @param count The number of columns to remove.
 * @param first The column index to start removing column from.
 */
void Spreadsheet::removeColumns(int first, int count) {
	if (count < 1 || first < 0 || first + count > columnCount())
		return;

	WAIT_CURSOR_AUTO_RESET;

	const int oldCount = columnCount();
	beginMacro(i18np("%1: remove 1 column", "%1: remove %2 columns", name(), count));

	Q_EMIT aspectsAboutToBeRemoved(first, first + count - 1);
	for (int i = 0; i < count; i++)
		child<Column>(first)->remove();
	Q_EMIT aspectsRemoved();

	exec(new SpreadsheetSetColumnsCountCmd(this, oldCount, columnCount()));
	endMacro();
}

/*!
 * Inserts \c count columns before the \c before column index in the spreadsheet.
 * @param count The number of columns to insert.
 * @param before The column index before which the columns are inserted.
 */
void Spreadsheet::insertColumns(int before, int count) {
	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18np("%1: insert 1 column", "%1: insert %2 columns", name(), count));
	const int cols = columnCount();
	const int rows = rowCount();
	const int last = before + count - 1;
	Q_EMIT aspectsAboutToBeInserted(before, last);
	for (int i = 0; i < count; i++) {
		auto* new_col = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
		new_col->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		new_col->insertRows(0, rows);
		insertChild(new_col, before + i);
	}
	Q_EMIT aspectsInserted(before, last);

	exec(new SpreadsheetSetColumnsCountCmd(this, cols, columnCount()));
	endMacro();
}

/*!
 * Clears all values in the spreadsheet.
 */
void Spreadsheet::clear() {
	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18n("%1: clear", name()));
	for (auto* col : children<Column>())
		col->clear();
	endMacro();
}

/*!
 * Clears all values in the specified \c columns.
 * @param columns The columns in the spreadsheet to clear.
 */
void Spreadsheet::clear(const QVector<Column*>& columns) {
	// TODO
	// 	if (formulaModeActive()) {
	// 		for (auto* col : selectedColumns()) {
	// 			col->setSuppressDataChangedSignal(true);
	// 			col->clearFormulas();
	// 			col->setSuppressDataChangedSignal(false);
	// 			col->setChanged();
	// 		}
	// 	} else {
	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18n("%1: clear selected columns", name()));
	for (auto* col : columns) {
		col->setSuppressDataChangedSignal(true);
		col->clear();
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
	endMacro();
}

/*!
 * Clears all masks in the spreadsheet.
 */
void Spreadsheet::clearMasks() {
	WAIT_CURSOR_AUTO_RESET;

	beginMacro(i18n("%1: clear all masks", name()));
	for (auto* col : children<Column>())
		col->clearMasks();
	endMacro();
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

void Spreadsheet::fillColumnsContextMenu(QMenu* menu) {
#ifndef SDK
	if (m_view)
		m_view->fillColumnsContextMenu(menu);
#else
	Q_UNUSED(menu)
#endif
}

/*!
 * Move column at \c from index to \c to index.
 * @param from The current index of the column.
 * @param to The future index of the column.
 */
void Spreadsheet::moveColumn(int from, int to) {
	const auto& columns = children<Column>();
	auto* col = columns.at(from);
	beginMacro(i18n("%1: move column %2 from position %3 to %4.", name(), col->name(), from + 1, to + 1));
	col->remove();
	insertChildBefore(col, columns.at(to));
	endMacro();
}

/*!
 * Clears all values in the specified columns.
 * @param cols The columns in the spreadsheet to clear.
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

	WAIT_CURSOR_AUTO_RESET;
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
} // end of sortColumns()

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon Spreadsheet::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-spreadsheet"));
}

/*!
 * Returns a text representation of the data in cell at \c row index and \c col index.
 * @param row The cell row.
 * @param col The cell column.
 * @return Text representation of the data in cell.
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

/*!
 * Toggles the StatisticsSpreadsheet for the current spreadsheet.
 * @param on Enable/disable the StatisticsSpreadsheet if true/false.
 */
void Spreadsheet::toggleStatisticsSpreadsheet(bool on) {
	Q_D(Spreadsheet);
	if (on) {
		if (d->statisticsSpreadsheet)
			return;

		d->statisticsSpreadsheet = new StatisticsSpreadsheet(this);
		d->statisticsSpreadsheet->setReadOnly(true);
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

	writer->writeStartElement(QLatin1String("general"));
	writer->writeAttribute(QStringLiteral("readOnly"), QString::number(d->readOnly));
	writer->writeAttribute(QStringLiteral("showComments"), QString::number(d->showComments));
	writer->writeAttribute(QStringLiteral("showSparklines"), QString::number(d->showSparklines));
	writer->writeEndElement();

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
			} else if (reader->name() == QLatin1String("general")) {
				attribs = reader->attributes();
				READ_INT_VALUE("readOnly", readOnly, bool);
				READ_INT_VALUE("showComments", showComments, bool);
				READ_INT_VALUE("showSparklines", showSparklines, bool);
			} else if (reader->name() == QLatin1String("linking")) {
				attribs = reader->attributes();
				READ_INT_VALUE("enabled", linking.linking, bool);
				d->linking.linkedSpreadsheetPath = attribs.value(QStringLiteral("spreadsheet")).toString();
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

	initConnectionsRowCountChanges();

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
	Q_ASSERT(d->m_usedInPlots.size() == 0);
	int columnOffset = 0;
	setUndoAware(false);
	if (m_model != nullptr)
		m_model->suppressSignals(true);

	// make the available columns undo unaware before we resize and rename them below,
	// the same will be done for new columns in this->resize().
	{
		const auto& columns = children<Column>();
		for (auto* column : std::as_const(columns))
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
		// replace the previous content of the data source completely with the content to be imported
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
				if (!d->m_involvedColumns.contains(column))
					d->m_involvedColumns.append(column);
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
								 AbstractFileFilter::ImportMode columnImportMode) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	Q_D(Spreadsheet);
	// DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << " / " << endColumn <<", row count = " << rowCount());

	CleanupNoArguments cleanup([d]() {
		d->m_usedInPlots.clear();
		d->m_involvedColumns.clear();
	});

	// determine the dependent plots
	if (columnImportMode == AbstractFileFilter::ImportMode::Replace) {
		for (size_t n = startColumn; n <= endColumn; n++) {
			auto* column = this->column((int)(columnOffset + n - startColumn));
			if (column) {
				column->addUsedInPlots(d->m_usedInPlots);
				if (!d->m_involvedColumns.contains(column))
					d->m_involvedColumns.append(column);
			}
		}
	}

	if (columnImportMode == AbstractFileFilter::ImportMode::Replace) {
		// suppress retransform in the dependent plots
		for (auto* plot : d->m_usedInPlots)
			plot->setSuppressRetransform(true);
	}

	// set the comments for each of the columns if datasource is a spreadsheet
	const int rows = rowCount();
	for (size_t col = startColumn; col <= endColumn; col++) {
		// DEBUG(Q_FUNC_INFO << ", column " << columnOffset + col - startColumn);
		Column* column = this->column((int)(columnOffset + col - startColumn));
		// DEBUG(Q_FUNC_INFO << ", type " << ENUM_TO_STRING(AbstractColumn, ColumnMode, column->columnMode()))

		if (!d->suppressSetCommentFinalizeImport) {
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
		}

		if (columnImportMode == AbstractFileFilter::ImportMode::Replace) {
			column->setSuppressDataChangedSignal(true);
			column->setChanged(); // Invalidate properties
			column->setSuppressDataChangedSignal(false);
		}
	}

	if (columnImportMode == AbstractFileFilter::ImportMode::Replace) {
		QVector<AbstractColumn*> children;
		if (project())
			children = project()->children<AbstractColumn>(ChildIndexFlag::Recursive);
		else
			children = this->children<AbstractColumn>();

		// Update all columns with formulas
		bool allColumnsRecalculated = true;
		QHash<Column*, bool> columnMap;
		for (auto* c : std::as_const(children)) {
			auto* column = static_cast<Column*>(c);
			if (d->m_involvedColumns.contains(c) || column->formula().isEmpty())
				columnMap[column] = true; // no recalculation required
			else {
				columnMap[column] = false;
				allColumnsRecalculated = false;
			}
		}

		// Solve all dependencies in the column formulas
		for (int i = 0; i < 2 && !allColumnsRecalculated; i++) { // Make 2 rounds to solve also complex dependencies
			allColumnsRecalculated = true;
			for (auto it = columnMap.begin(), end = columnMap.end(); it != end; ++it) {
				if (it.value() == true)
					continue;

				bool allDependenciesRecalculated = true;
				const auto& formulaDatas = it.key()->formulaData();
				for (const auto& formulaData : formulaDatas) {
					auto depColumn = formulaData.column();
					const auto foundColumn = columnMap.constFind(const_cast<Column*>(depColumn));
					if (foundColumn != columnMap.end()) {
						if (foundColumn.value() == false) {
							allDependenciesRecalculated = false;
							break;
						}
					}
				}
				// All dependencies are already recalculated, so this can be recalculated as well
				if (allDependenciesRecalculated) {
					it.key()->updateFormula();
					*it = true;
				} else
					allColumnsRecalculated = false;
			}
		}

		// recalculate all curves
		QHash<Plot*, bool> curveMap; // second parameter determines if the curve was already recalculated or not
		for (auto* plot : std::as_const(d->m_usedInPlots)) {
			for (auto* curve : plot->children<Plot>())
				curveMap[curve] = false;
		}

		bool allCurvesRecalculated = true;
		for (const auto* c : std::as_const(d->m_involvedColumns)) {
			for (auto i = curveMap.begin(), end = curveMap.end(); i != end; ++i) {
				if (i.value() == true)
					continue;

				if (i.key()->usingColumn(c, false)) {
					auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(i.key());
					if (analysisCurve)
						analysisCurve->recalculate(); // Will call recalc() of the XYCurve at the end
					else
						i.key()->recalc(); // Normal recalc of the values required (XYCurve)
					*i = true;
				} else
					allCurvesRecalculated = false;
			}
			if (allCurvesRecalculated)
				break;
		}

		// Solve all dependencies and recalculate all analysis curves which depend on other curves
		for (int i = 0; i < 2 && !allCurvesRecalculated; i++) { // Make 2 rounds to solve also complex dependencies
			allCurvesRecalculated = true;
			for (auto it = curveMap.begin(), end = curveMap.end(); it != end; ++it) {
				if (it.value() == true)
					continue;

				auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(it.key());
				if (analysisCurve) {
					bool allDependenciesRecalculated = true;
					const auto& depPlots = analysisCurve->dependingPlots();
					for (const auto* depPlot : depPlots) {
						const auto foundPlot = curveMap.constFind(const_cast<Plot*>(depPlot));
						if (foundPlot != curveMap.end()) {
							if (foundPlot.value() == false) {
								allDependenciesRecalculated = false;
								break;
							}
						}
					}
					// All dependencies are already recalculated, so this can be recalculated as well
					if (allDependenciesRecalculated) {
						analysisCurve->recalculate();
						*it = true;
					} else
						allCurvesRecalculated = false;
				}
			}
		}

		// retransform the dependent plots
		for (auto* plot : d->m_usedInPlots) {
			plot->setSuppressRetransform(false);
			plot->dataChanged(-1, -1); // TODO: check if all ranges must be updated
		}
	}

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
	Q_EMIT rowCountChanged(rowCount());
	// need to notify about the column count change although this should already done by add/removeChild signals
	Q_EMIT columnCountChanged(columnCount());

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

void SpreadsheetPrivate::updateCommentsHeader() {
#ifndef SDK
	q->m_view->showComments(q->showComments());
#endif
}

void SpreadsheetPrivate::updateSparklinesHeader() {
#ifndef SDK
	q->m_view->showSparklines(q->showSparklines());
#endif
}
