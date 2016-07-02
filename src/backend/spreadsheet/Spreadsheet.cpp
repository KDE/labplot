/***************************************************************************
    File                 : Spreadsheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke@gmx.de)

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
#include "backend/core/AspectPrivate.h"
#include "backend/core/AbstractAspect.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <KIcon>
#include <KConfigGroup>
#include <KLocale>

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

Spreadsheet::Spreadsheet(AbstractScriptingEngine* engine, const QString& name, bool loading)
  : AbstractDataSource(engine, name){

	if (!loading)
		init();
}

/*!
	initializes the spreadsheet with the default number of columns and rows
*/
void Spreadsheet::init() {
	int columns, rows;
	KConfig config;
	KConfigGroup group = config.group( "Spreadsheet" );
	columns = group.readEntry("ColumnCount", 2);
	rows = group.readEntry("RowCount", 100);

	for(int i=0; i<columns; i++) {
		Column* new_col = new Column(QString::number(i+1), AbstractColumn::Numeric);
		new_col->setPlotDesignation(i == 0 ? AbstractColumn::X : AbstractColumn::Y);
		addChild(new_col);
	}
	setRowCount(rows);
}

/*! Constructs a primary view on me.
  This method may be called multiple times during the life time of an Aspect, or it might not get
  called at all. Aspects must not depend on the existence of a view for their operation.
*/
QWidget *Spreadsheet::view() const {
	if (!m_view) {
		m_view = new SpreadsheetView(const_cast<Spreadsheet*>(this));
		KConfig config;
		KConfigGroup group = config.group( "Spreadsheet" );
		reinterpret_cast<SpreadsheetView*>(m_view)->showComments(group.readEntry("ShowComments", false));
	}

	return m_view;
}

bool Spreadsheet::exportView() const {
	ExportSpreadsheetDialog* dlg = new ExportSpreadsheetDialog(view());
	dlg->setFileName(name());
    bool ret;
    if ((ret = dlg->exec()==QDialog::Accepted)){
        const QString path = dlg->path();
        const bool exportHeader = dlg->exportHeader();
        const SpreadsheetView* view = reinterpret_cast<const SpreadsheetView*>(m_view);
        WAIT_CURSOR;
        if (dlg->format() == ExportSpreadsheetDialog::LaTeX){
            const bool exportLatexHeader = dlg->exportLatexHeader();
            const bool gridLines = dlg->gridLines();
            const bool captions = dlg->captions();
            const bool skipEmptyRows =dlg->skipEmptyRows();
            const bool exportEntire = dlg->entireSpreadheet();
            view->exportToLaTeX(path, exportHeader, gridLines, captions,
                                exportLatexHeader, skipEmptyRows, exportEntire);
        }
        else {
            const QString separator = dlg->separator();
            view->exportToFile(path, exportHeader, separator);
        }
        RESET_CURSOR;
    }
	delete dlg;
    return ret;
}

bool Spreadsheet::printView() {
	QPrinter printer;
	QPrintDialog* dlg = new QPrintDialog(&printer, view());
	bool ret;
	dlg->setWindowTitle(i18n("Print Spreadsheet"));
	if ((ret = dlg->exec() == QDialog::Accepted)) {
		const SpreadsheetView* view = reinterpret_cast<const SpreadsheetView*>(m_view);
		view->print(&printer);
	}
	delete dlg;
	return ret;
}

bool Spreadsheet::printPreview() const {
	const SpreadsheetView* view = reinterpret_cast<const SpreadsheetView*>(m_view);
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, SIGNAL(paintRequested(QPrinter*)), view, SLOT(print(QPrinter*)));
	return dlg->exec();
}

/*!
  Returns the total number of rows in the spreadsheet.
 */
int Spreadsheet::rowCount() const
{
	int col_rows, result=0;
	foreach(Column * col, children<Column>())
		if ((col_rows=col->rowCount()) > result)
			result = col_rows;
	return result;
}

void Spreadsheet::removeRows(int first, int count)
{
	if( count < 1 || first < 0 || first+count > rowCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: remove 1 row", "%1: remove %2 rows", name(), count) );
	foreach(Column * col, children<Column>(IncludeHidden))
	    col->removeRows(first, count);
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertRows(int before, int count)
{
	if( count < 1 || before < 0 || before > rowCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: insert 1 row", "%1: insert %2 rows", name(), count) );
	foreach(Column * col, children<Column>(IncludeHidden))
	    col->insertRows(before, count);
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::appendRows(int count){
	insertRows(rowCount(), count);
}

void Spreadsheet::appendRow(){
	insertRows(rowCount(), 1);
}

void Spreadsheet::appendColumns(int count){
	insertColumns(columnCount(), count);
}

void Spreadsheet::appendColumn(){
	insertColumns(columnCount(), 1);
}

void Spreadsheet::prependColumns(int count){
	insertColumns(0, count);
}

/*!
  Sets the number of rows of the spreadsheet to \c new_size
*/
void Spreadsheet::setRowCount(int new_size)
{
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
Column* Spreadsheet::column(int index) const{
  return child<Column>(index);
}


/*!
  Returns the column with the name \c name.
*/
Column* Spreadsheet::column(const QString &name) const{
  return child<Column>(name);
}

/*!
  Returns the total number of columns in the spreadsheet.
*/
int Spreadsheet::columnCount() const{
  return childCount<Column>();
}

/*!
  Returns the number of columns matching the given designation.
 */
int Spreadsheet::columnCount(AbstractColumn::PlotDesignation pd) const
{
	int count = 0;
	foreach(Column * col, children<Column>())
	    if (col->plotDesignation() == pd)
		count++;
	return count;
}

void Spreadsheet::removeColumns(int first, int count)
{
	if( count < 1 || first < 0 || first+count > columnCount()) return;
	WAIT_CURSOR;
	beginMacro( i18np("%1: remove 1 column", "%1: remove %2 columns", name(), count) );
	for (int i=0; i<count; i++)
		child<Column>(first)->remove();
	endMacro();
	RESET_CURSOR;
}

void Spreadsheet::insertColumns(int before, int count)
{
	WAIT_CURSOR;
	beginMacro( i18np("%1: insert 1 column", "%1: insert %2 columns", name(), count) );
	Column * before_col = column(before);
	int rows = rowCount();
	for (int i=0; i<count; i++) {
		Column * new_col = new Column(QString::number(i+1), AbstractColumn::Numeric);
		new_col->setPlotDesignation(AbstractColumn::Y);
		new_col->insertRows(0, rows);
		insertChildBefore(new_col, before_col);
	}
	endMacro();
	RESET_CURSOR;
}
/*!
  Sets the number of columns to \c new_size
*/
void Spreadsheet::setColumnCount(int new_size)
{
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
void Spreadsheet::clear()
{
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear", name()));
	foreach (Column * col, children<Column>())
		col->clear();
	endMacro();
	RESET_CURSOR;
}

/*!
  Clears all mask in the spreadsheet.
*/
void Spreadsheet::clearMasks()
{
	WAIT_CURSOR;
	beginMacro(i18n("%1: clear all masks", name()));
	foreach(Column * col, children<Column>())
	    col->clearMasks();
	endMacro();
	RESET_CURSOR;
}

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu *Spreadsheet::createContextMenu(){
	QMenu *menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

void Spreadsheet::moveColumn(int from, int to)
{
	Column * col = child<Column>(from);
	beginMacro(i18n("%1: move column %2 from position %3 to %4.", name(), col->name(), from+1, to+1));
	col->remove();
	insertChildBefore(col, child<Column>(to));
	endMacro();
}

void Spreadsheet::copy(Spreadsheet * other)
{
	WAIT_CURSOR;
	beginMacro(i18n("%1: copy %2", name(), other->name()));

	foreach(Column * col, children<Column>())
		col->remove();
	foreach(Column * src_col, other->children<Column>()) {
		Column * new_col = new Column(src_col->name(), src_col->columnMode());
		new_col->copy(src_col);
		new_col->setPlotDesignation(src_col->plotDesignation());
		QList< Interval<int> > masks = src_col->maskedIntervals();
		foreach(const Interval<int>& iv, masks)
			new_col->setMasked(iv);
		QList< Interval<int> > formulas = src_col->formulaIntervals();
		foreach(const Interval<int>& iv, formulas)
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
int Spreadsheet::colX(int col)
{
	for(int i=col-1; i>=0; i--)
	{
		if (column(i)->plotDesignation() == AbstractColumn::X)
			return i;
	}
	int cols = columnCount();
	for(int i=col+1; i<cols; i++)
	{
		if (column(i)->plotDesignation() == AbstractColumn::X)
			return i;
	}
	return -1;
}

/*!
  Determines the corresponding Y column.
*/
int Spreadsheet::colY(int col)
{
	int cols = columnCount();

	if (column(col)->plotDesignation() == AbstractColumn::xErr ||
			column(col)->plotDesignation() == AbstractColumn::yErr) {
		// look to the left first
		for(int i=col-1; i>=0; i--) {
			if (column(i)->plotDesignation() == AbstractColumn::Y)
				return i;
		}
		for(int i=col+1; i<cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::Y)
				return i;
		}
	} else {
		// look to the right first
		for(int i=col+1; i<cols; i++) {
			if (column(i)->plotDesignation() == AbstractColumn::Y)
				return i;
		}
		for(int i=col-1; i>=0; i--) {
			if (column(i)->plotDesignation() == AbstractColumn::Y)
				return i;
		}
	}
	return -1;
}

/*! Sorts the given list of column.
  If 'leading' is a null pointer, each column is sorted separately.
*/
void Spreadsheet::sortColumns(Column *leading, QList<Column*> cols, bool ascending)
{
	if(cols.isEmpty()) return;

	// the normal QPair comparison does not work properly with descending sorting
	// thefore we use our own compare functions
	class CompareFunctions {
		public:
			static bool doubleLess(const QPair<double, int>& a, const QPair<double, int>& b)
			{
				return a.first < b.first;
			}
			static bool doubleGreater(const QPair<double, int>& a, const QPair<double, int>& b)
			{
				return a.first > b.first;
			}
			static bool QStringLess(const QPair<QString, int>& a, const QPair<QString, int>& b)
			{
				return a < b;
			}
			static bool QStringGreater(const QPair<QString, int>& a, const QPair<QString, int>& b)
			{
				return a > b;
			}
			static bool QDateTimeLess(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b)
			{
				return a < b;
			}
			static bool QDateTimeGreater(const QPair<QDateTime, int>& a, const QPair<QDateTime, int>& b)
			{
				return a > b;
			}
	};

	WAIT_CURSOR;
	beginMacro(i18n("%1: sort columns", name()));

	if(leading == 0) { // sort separately
		foreach(Column *col, cols) {
			switch (col->columnMode()) {
				case AbstractColumn::Numeric:
					{
						int rows = col->rowCount();
						QList< QPair<double, int> > map;

						for(int j=0; j<rows; j++)
							map.append(QPair<double, int>(col->valueAt(j), j));

						if(ascending)
							qStableSort(map.begin(), map.end(), CompareFunctions::doubleLess);
						else
							qStableSort(map.begin(), map.end(), CompareFunctions::doubleGreater);

						QListIterator< QPair<double, int> > it(map);
						Column *temp_col = new Column("temp", col->columnMode());

						int k=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, k, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							k++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
						break;
					}
				case AbstractColumn::Text:
					{
						int rows = col->rowCount();
						QList< QPair<QString, int> > map;

						for(int j=0; j<rows; j++)
							map.append(QPair<QString, int>(col->textAt(j), j));

						if(ascending)
							qStableSort(map.begin(), map.end(), CompareFunctions::QStringLess);
						else
							qStableSort(map.begin(), map.end(), CompareFunctions::QStringGreater);

						QListIterator< QPair<QString, int> > it(map);
						Column *temp_col = new Column("temp", col->columnMode());

						int k=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, k, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							k++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
						break;
					}
				case AbstractColumn::DateTime:
				case AbstractColumn::Month:
				case AbstractColumn::Day:
					{
						int rows = col->rowCount();
						QList< QPair<QDateTime, int> > map;

						for(int j=0; j<rows; j++)
							map.append(QPair<QDateTime, int>(col->dateTimeAt(j), j));

						if(ascending)
							qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
						else
							qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);

						QListIterator< QPair<QDateTime, int> > it(map);
						Column *temp_col = new Column("temp", col->columnMode());

						int k=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, k, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							k++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
						break;
					}
			}
		}
	} else { // sort with leading column
		switch (leading->columnMode()) {
			case AbstractColumn::Numeric:
				{
					QList< QPair<double, int> > map;
					int rows = leading->rowCount();

					for(int i=0; i<rows; i++)
						map.append(QPair<double, int>(leading->valueAt(i), i));

					if(ascending)
						qStableSort(map.begin(), map.end(), CompareFunctions::doubleLess);
					else
						qStableSort(map.begin(), map.end(), CompareFunctions::doubleGreater);
					QListIterator< QPair<double, int> > it(map);

					foreach (Column *col, cols) {
						Column *temp_col = new Column("temp", col->columnMode());
						it.toFront();
						int j=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, j, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							j++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
					}
					break;
				}
			case AbstractColumn::Text:
				{
					QList< QPair<QString, int> > map;
					int rows = leading->rowCount();

					for(int i=0; i<rows; i++)
						map.append(QPair<QString, int>(leading->textAt(i), i));

					if(ascending)
						qStableSort(map.begin(), map.end(), CompareFunctions::QStringLess);
					else
						qStableSort(map.begin(), map.end(), CompareFunctions::QStringGreater);
					QListIterator< QPair<QString, int> > it(map);

					foreach (Column *col, cols) {
						Column *temp_col = new Column("temp", col->columnMode());
						it.toFront();
						int j=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, j, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							j++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
					}
					break;
				}
			case AbstractColumn::DateTime:
			case AbstractColumn::Month:
			case AbstractColumn::Day:
				{
					QList< QPair<QDateTime, int> > map;
					int rows = leading->rowCount();

					for(int i=0; i<rows; i++)
						map.append(QPair<QDateTime, int>(leading->dateTimeAt(i), i));

					if(ascending)
						qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeLess);
					else
						qStableSort(map.begin(), map.end(), CompareFunctions::QDateTimeGreater);
					QListIterator< QPair<QDateTime, int> > it(map);

					foreach (Column *col, cols) {
						Column *temp_col = new Column("temp", col->columnMode());
						it.toFront();
						int j=0;
						// put the values in the right order into temp_col
						while(it.hasNext()) {
							temp_col->copy(col, it.peekNext().second, j, 1);
							temp_col->setMasked(col->isMasked(it.next().second));
							j++;
						}
						// copy the sorted column
						col->copy(temp_col, 0, 0, rows);
						delete temp_col;
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
	return KIcon("labplot-spreadsheet");
}

/*!
  Returns the text displayed in the given cell.
*/
QString Spreadsheet::text(int row, int col) const
{
	Column * col_ptr = column(col);
	if(!col_ptr)
		return QString();

	return col_ptr->asStringColumn()->textAt(row);
}

/*!
 * This slot is, indirectly, called when a child of \c Spreadsheet (i.e. column) was selected in \c ProjectExplorer.
 * Emits the signal \c columnSelected that is handled in \c SpreadsheetView.
 */
void Spreadsheet::childSelected(const AbstractAspect* aspect){
	const Column* column=qobject_cast<const Column*>(aspect);
	if (column){
		int index = indexOfChild<Column>(column);
		emit columnSelected(index);
	}
}

/*!
 * This slot is, indirectly, called when a child of \c Spreadsheet (i.e. column) was deselected in \c ProjectExplorer.
 * Emits the signal \c columnDeselected that is handled in \c SpreadsheetView.
 */
void Spreadsheet::childDeselected(const AbstractAspect* aspect){
	const Column* column=qobject_cast<const Column*>(aspect);
	if (column){
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
void Spreadsheet::setColumnSelectedInView(int index, bool selected){
  if (selected){
	emit childAspectSelectedInView(child<Column>(index));

	//deselect the spreadsheet in the project explorer, if a child (column) was selected.
	//prevents unwanted multiple selection with spreadsheet (if it was selected before).
	emit childAspectDeselectedInView(this);
  }else{
	emit childAspectDeselectedInView(child<Column>(index));
  }
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
	foreach (Column* col, children<Column>(IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); // "spreadsheet"
}

/*!
  Loads from XML.
*/
bool Spreadsheet::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "spreadsheet")
	{
		if (!readBasicAttributes(reader)) return false;

		// read child elements
		while (!reader->atEnd())
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement())
			{
				if (reader->name() == "comment")
				{
					if (!readCommentElement(reader)) return false;
				}
				else if(reader->name() == "column")
				{
					Column* column = new Column("");
					if (!column->load(reader))
					{
                        delete column;
						setColumnCount(0);
						return false;
					}
					addChild(column);
				}
				else // unknown element
				{
					reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			}
		}
	}
	else // no spreadsheet element
		reader->raiseError(i18n("no spreadsheet element found"));

	return !reader->hasError();
}
