/***************************************************************************
    File                 : Spreadsheet.h
    Project              : SciDAVis
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2006-2009 Knut Franke (knut.franke*gmx.de)
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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "core/AbstractPart.h"
#include "core/AbstractScriptingEngine.h"
#include "core/globals.h"
#include "core/column/Column.h"
#include <QList>

/*!\brief Aspect providing a spreadsheet table with column logic.
 *
 * Spreadsheet is a container object for columns with no data of its own. By definition, it's columns
 * are all of its children inheriting from class Column. Thus, the basic API is already defined
 * by AbstractAspect (managing the list of columns, notification of column insertion/removal)
 * and Column (changing and monitoring state of the actual data).
 *
 * Spreadsheet stores a pointer to its primary view of class SpreadsheetView. SpreadsheetView calls the Spreadsheet
 * API but Spreadsheet only notifies SpreadsheetView by signals without calling its API directly. This ensures a
 * maximum independence of UI and backend. SpreadsheetView can be easily replaced by a different class.
 * User interaction is completely handled in SpreadsheetView and translated into
 * Spreadsheet API calls (e.g., when a user edits a cell this will be handled by the delegate of
 * SpreadsheetView and Spreadsheet will not know whether a script or a user changed the data.). All actions,
 * menus etc. for the user interaction are handled SpreadsheetView, e.g., via a context menu.
 * Selections are also handled by SpreadsheetView. The view itself is created by the first call to view();
 */
class Spreadsheet : public AbstractPart, public scripted
{
	Q_OBJECT

	public:

		Spreadsheet(AbstractScriptingEngine *engine, int rows, int columns, const QString &name);
		~Spreadsheet();

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
		virtual QWidget *view() const;

		//! Return the total number of columns in the spreadsheet
		int columnCount() const { return childCount<Column>(); }
		//! Return the number of columns matching the given designation
		int columnCount(SciDAVis::PlotDesignation pd) const;
		//! Return column number 'index'
		/**
		 * Shallow wrapper around AbstractAspect::child() - see there for caveat.
		 */
		Column* column(int index) const { return child<Column>(index); }
		Column* column(const QString &name) const { return child<Column>(name); }
		//! Return the total number of rows in the spreadsheet
		int rowCount() const;

		void removeRows(int first, int count);
		void insertRows(int before, int count);
		void removeColumns(int first, int count);
		void insertColumns(int before, int count);

		// FIXME: replace index-based API with Column*-based one
		//! Determine the corresponding X column
		int colX(int col);
		//! Determine the corresponding Y column
		int colY(int col);
		//! Return the text displayed in the given cell
		QString text(int row, int col) const;

		void copy(Spreadsheet * other);

		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const;
		//! Load from XML
		virtual bool load(XmlStreamReader *);
		//@}
		
	public slots:		
		void appendRows(int count) { insertRows(rowCount(), count); }
		void appendRow() { insertRows(rowCount(), 1); }
		void appendColumns(int count) { insertColumns(columnCount(), count); }
		void appendColumn() { insertColumns(columnCount(), 1); }

		//! Set the number of columns
		void setColumnCount(int new_size);
		//! Set the number of rows of the spreadsheet
		void setRowCount(int new_size);

		//! Clear the whole spreadsheet
		void clear();
		//! Clear all mask in the spreadsheet
		void clearMasks();

		void moveColumn(int from, int to);
		//! Sort the given list of column
		/*
		 * If 'leading' is a null pointer, each column is sorted separately.
		 */
		void sortColumns(Column * leading, QList<Column*> cols, bool ascending);

	signals:
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		void requestProjectMenu(QMenu *menu, bool *rc);
		void requestProjectContextMenu(QMenu *menu);
#endif

	private:
		mutable QWidget *m_view;
};

#endif

