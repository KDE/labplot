/***************************************************************************
    File                 : Table.h
    Project              : SciDAVis
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2006-2007 Ion Vasilief (ion_vasilief*yahoo.fr)
                           (replace * with @ in the email addresses) 

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
#ifndef TABLE_H
#define TABLE_H

#include "core/AbstractPart.h"
#include "core/AbstractScriptingEngine.h"
#include "core/globals.h"
#include <QList>

class TableView;
class QUndoStack;
class QMenu;
class Column;
class QPoint;
class QAction;
class AbstractColumn;
class ActionManager;

/*!\brief Aspect providing a spreadsheet table with column logic.
 *
This class (incl. Table::Private and its commands) is one aspect in the projet hierarchy
that represents a spreadsheet table with column logic. Table provides the public API while
Table::Private completely encapsulates the data. The table commands (derived from QUndoCommand) 
encapsulate all write operations which can be undone and redone, if the table has an undo stack 
associated with it (usually by the project root aspect).

The underlying private data object is not visible to any classes other then those meantioned
above with one exeption:
Pointers to columns can be passed around an manipulated directly. The owner Table (parent aspect
of the Column objects) will be notified by emission of signals and react accordingly. 
All public methods of Table and Column are undo aware. 

Table also stores a pointer to its primary view of class TableView. TableView calls the Table
API but Table only notifies TableView by signals without calling its API directly. This ensures a
maximum independence of UI and backend. TableView can be easily replaced by a different class.
User interaction is completely handled in TableView and translated into 
Table API calls (e.g., when a user edits a cell this will be handled by the delegate of
TableView and Table will not know whether a script or a user changed the data.). All actions, 
menus etc. for the user interaction are handled TableView, e.g., via a context menu.
Selections are also handled by TableView. The view itself is created by the first call to view();
*/
class Table : public AbstractPart, public scripted
{
	Q_OBJECT

	public:
		class Private; // This could also be private, but then all commands need to be friend classes
		friend class Private;

		Table(AbstractScriptingEngine *engine, int rows, int columns, const QString &name);
		~Table();

		//! Return an icon to be used for decorating my views.
		virtual QIcon icon() const;
		//! Fill the part specific menu for the main window including setting the title
		/**
		 * \return true on success, otherwise false (e.g. part has no actions).
		 */
		virtual bool fillProjectMenu(QMenu * menu);
		//! Return a new context menu.
		/**
		 * The caller takes ownership of the menu.
		 */
		virtual QMenu *createContextMenu();

		//! Construct a primary view on me.
		/**
		 * This method may be called multiple times during the life time of an Aspect, or it might not get
		 * called at all. Aspects must not depend on the existence of a view for their operation.
		 */
		virtual QWidget *view();
		
		//! Insert columns
		/**
		 * Ownership of the columns is transferred to this Table.
		 *
		 * If before == columnCount() this will do the same as appendColumns();
		 */
		void insertColumns(int before, QList<Column *> new_cols);
		//! Append columns
		/*
		 * Convenience function, same as:
		 * <code>
		 * insertColumns(columnCount(), new_cols);
		 * </code>
		 */
		void appendColumns(QList<Column*> new_cols) { insertColumns(columnCount(), new_cols); }
		void removeColumns(int first, int count);
		void removeColumn(Column * col);
		void removeRows(int first, int count);
		void insertRows(int before, int count);
		void appendRows(int count) { insertRows(rowCount(), count); }
		//! Set the number of rows of the table
		void setRowCount(int new_size);
		//! Return the total number of columns in the table
		int columnCount() const;
		//! Return the total number of rows in the table
		int rowCount() const;
		//! Return the number of columns matching the given designation
		int columnCount(SciDAVis::PlotDesignation pd) const;
		//! Return column number 'index'
		Column* column(int index) const;
		//! Return the column determined by the given name
		/**
		 * This method should not be used unless absolutely necessary. 
		 * Columns should be addressed by their index. 
		 * This method is mainly meant to be used in scripts.
		 */
		Column* column(const QString & name) const;
		int columnIndex(const Column * col) const;
		//! Set the number of columns
		void setColumnCount(int new_size);
		QVariant headerData(int section, Qt::Orientation orientation,int role) const;

		//! Determine the corresponding X column
		int colX(int col);
		//! Determine the corresponding Y column
		int colY(int col);
		//! Return the text displayed in the given cell
		QString text(int row, int col);
		void copy(Table * other);

		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const;
		//! Load from XML
		virtual bool load(XmlStreamReader *);
		bool readColumnWidthElement(XmlStreamReader * reader);
		//@}
		
	public slots:
		//! Clear the whole table
		void clear();
		//! Clear all mask in the table
		void clearMasks();

		//! Append one column
		void addColumn();
		void addColumns(int count);
		void addRows(int count);
		void moveColumn(int from, int to);
		//! Sort the given list of column
		/*
		 * If 'leading' is a null pointer, each column is sorted separately.
		 */
		void sortColumns(Column * leading, QList<Column*> cols, bool ascending);

	protected:
		//! Called after a new child has been inserted or added.
		/**
		 * Unlike the aspectAdded() signals, this method does not get called inside undo/redo actions;
		 * allowing subclasses to execute undo commands of their own.
		 */
		virtual void completeAspectInsertion(AbstractAspect * aspect, int index);
		//! Called before a child is removed.
		/**
		 * Unlike the aspectAboutToBeRemoved() signals, this method does not get called inside undo/redo actions;
		 * allowing subclasses to execute undo commands of their own.
		 */
		virtual void prepareAspectRemoval(AbstractAspect * aspect);

	public:
		//! This method should only be called by the view.
		/** This method does not change the view, it only changes the
		 * values that are saved when the table is saved. The view
		 * has to take care of reading and applying these values */
		void setColumnWidth(int col, int width);
		int columnWidth(int col) const;

	private:
		//! Internal function to connect all column signals
		void connectColumn(const Column* col);
		//! Internal function to disconnect a column
		void disconnectColumn(const Column* col);

	private slots:
		//! \name Column event handlers
		//@{
		void handleDescriptionChange(const AbstractAspect * aspect);
		void handleModeChange(const AbstractColumn * col);
		void handlePlotDesignationChange(const AbstractColumn * col);
		void handleDataChange(const AbstractColumn * col);
		void handleRowsAboutToBeInserted(const AbstractColumn * col, int before, int count);
		void handleRowsInserted(const AbstractColumn * col, int before, int count);
		void handleRowsAboutToBeRemoved(const AbstractColumn * col, int first, int count);
		void handleRowsRemoved(const AbstractColumn * col, int first, int count);
		//@}

	signals:
		void columnsAboutToBeInserted(int before, QList<Column*> new_cols);
		void columnsInserted(int first, int count);
		void columnsAboutToBeReplaced(int first, int count);
		void columnsReplaced(int first, int count);
		void columnsAboutToBeRemoved(int first, int count);
		void columnsRemoved(int first, int count);
		void rowsAboutToBeInserted(int before, int count);
		void rowsInserted(int first, int count);
		void rowsAboutToBeRemoved(int first, int count);
		void rowsRemoved(int first, int count);
		void dataChanged(int top, int left, int bottom, int right);
		void headerDataChanged(Qt::Orientation orientation, int first, int last);
		void sectionSizesChanged();
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		void requestProjectMenu(QMenu *menu, bool *rc);
		void requestProjectContextMenu(QMenu *menu);
#endif

	private:

		QWidget *m_view;
		Private *m_table_private;
};

/**
  This private class manages column based data (i.e., 1D vector based 
  data such as x-values and y-values for a plot) for a Table. Its
  API is to be called by Table and table commands only. Table 
  may only call the reading functions to ensure that undo/redo
  is possible for all data changing operations.

  Each column is represented by a Column object and can be directly 
  accessed by the pointer returned by column(). Most of the column 
  manipulation is done directly to the columns. The signals of
  the columns are connected to various handlers in Table which
  acts according to all changes made to the columns.

  The Column objects are managed as child aspects by Table.

  Every column has two filters as children: An input filter that
  can convert a string (e.g., entered by the user in a cell) to
  the column's data type and an output filter that delivers
  the correct string representation to display in a table.

  The number of columns in the Table will always be equal to
  m_columns.size(). The number of rows is generally indepenent
  of the number of rows in the wrapped columns. It is however
  always adjusted to be large enough to display the longest column. 
  When columns are inserted, resized etc., the table is resized 
  automatically.
  */
class Table::Private
{
	public:
		Private(Table *owner) : m_owner(owner), m_column_count(0), m_row_count(0) {}
		//! Replace columns completely
		/**
		 * \param first the first column to be replaced
		 * \param new_cols list of the columns that replace the old ones
		 * This does not delete the replaced columns.
		 */
		void replaceColumns(int first, QList<Column*> new_cols);
		//! Insert columns before column number 'before'
		/**
		 * If 'first' is equal to the number of columns,
		 * the columns will be appended.
		 * \param before index of the column to insert before
		 * \param cols a list of column data objects
		 */
		void insertColumns(int before, QList<Column*> cols);
		//! Remove Columns
		/**
		 * This does not delete the removed columns because this
		 * must be handled by the undo/redo system.
		 * \param first index of the first column to be removed
		 * \param count number of columns to remove
		 */
		void removeColumns(int first, int count);
		//! Append columns to the table
		/**
		 * \sa insertColumns()
		 */
		void appendColumns(QList<Column*> cols);
		//! Move a column to another position
		void moveColumn(int from, int to);
		//! Return the number of columns in the table
		int columnCount() const { return m_column_count; }
		//! Return the number of rows in the table
		int rowCount() const { return m_row_count; }
		//! Set the number of rows of the table
		void setRowCount(int count);
		//! Return the full column header string
		QString columnHeader(int col);
		//! Return the number of columns with a given plot designation
		int numColsWithPD(SciDAVis::PlotDesignation pd);
		//! Return column number 'index'
		Column* column(int index) const;
		//! Return the index of the given column in the table.
		/**
		 * \return the index or -1 if the column is not in the table
		 */
		int columnIndex(const Column * col) const 
		{ 
			for(int i=0; i<m_columns.size(); i++)
				if(m_columns.at(i) == col) return i;
			return -1;
		}
		QString name() const { return m_owner->name(); }
		QVariant headerData(int section, Qt::Orientation orientation,int role) const;

		//! Update the vertical header labels
		/**
		 * This must be called whenever rows are added
		 * or removed.
		 * \param start_row first row that needs to be updated
		 */
		void updateVerticalHeader(int start_row);
		//! Update the horizontal header labels
		/**
		 * This must be called whenever columns are added or
		 * removed and when comments, labels, and column types
		 * change.
		 * \param start_col first column that needs to be updated
		 * \param end_col last column that needs to be updated
		 */
		void updateHorizontalHeader(int start_col, int end_col);
		void setColumnWidth(int col, int width) { m_column_widths[col] = width; }
		int columnWidth(int col) const { return m_column_widths.at(col); }

	private:
		//! The owner aspect
		Table *m_owner;
		//! The number of columns
		int m_column_count;
		//! The maximum number of rows of all columns
		int m_row_count;
		//! Vertical header data
		QStringList m_vertical_header_data;
		//! Horizontal header data
		QStringList m_horizontal_header_data;
		//! List of pointers to the column data vectors
		QList<Column *> m_columns;	
		//! Internal function to put together the column header
		/**
		 * Don't use this outside updateHorizontalHeader()
		 */
		void composeColumnHeader(int col, const QString& label);
		//! Columns widths
		QList<int> m_column_widths;
};

#endif

