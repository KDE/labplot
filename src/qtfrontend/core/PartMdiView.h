/***************************************************************************
    File                 : PartMdiView.h
    Project              : SciDAVis
    Description          : MDI sub window to be wrapped around views of
                           AbstractPart.
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2007,2008 Knut Franke (knut.franke*gmx.de)
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
#ifndef ASPECT_VIEW_H
#define ASPECT_VIEW_H

#include <QMdiSubWindow>
#include "lib/macros.h"

class AbstractAspect;
class AbstractPart;

//! MDI sub window that implements functions common to all aspect views
/**
 * This class is to be subclassed for every aspect. Every aspect owns a
 * view that is shown in the ProjectWindow's MDI area. In addition to the functionality provided
 * by QMdiSubWindow, this class automatically updates the window title when
 * AbstractAspect::caption() changes, removes the Aspect when the MDI window 
 * is closed (the user is asked then whether he wants to hide the window
 * or remove the aspect). It also provides access to the aspect's context menu.
 */
class PartMdiView : public QMdiSubWindow
{
	Q_OBJECT

	public:
		//! Construct a view owned by the given aspect
		PartMdiView(AbstractPart * part, QWidget * embedded_view);
		virtual ~PartMdiView();

		//! Returns the part that owns the view
		AbstractPart *part() const { return m_part; }

		enum SubWindowStatus 
		{ 
			Closed,
			Hidden,
			Visible
		};

		SubWindowStatus status() const { return m_status; }
	
	signals:
		void statusChanged(PartMdiView *view, PartMdiView::SubWindowStatus from, PartMdiView::SubWindowStatus to);

	private slots:
		//! Keep my window title in sync with AbstractAspect::caption().
		void handleAspectDescriptionChanged(const AbstractAspect *aspect);
		void handleAspectAboutToBeRemoved(const AbstractAspect *aspect);

	protected:
		//! When I'm being closed, remove my Aspect from its parent.
		virtual void closeEvent(QCloseEvent *event);
		virtual void hideEvent(QHideEvent *event);
		virtual void showEvent(QShowEvent *event);
		//! Show a context menu for the view and the aspect
		/**
		 * This creates a view context menu with createContextMenu
		 * and then appends a context menu from the aspect to it.
		 * Then the context menu is presented to the user.
		 */
		virtual void contextMenuEvent(QContextMenuEvent *event);

	private:
		//! The aspect that owns the view
		AbstractPart *m_part;
		//! Whether I'm just being closed.
		/**
		 * Depending on whether an Aspect is removed programatically or by closing its
		 * default view (held by me), either aspectAboutToBeRemoved() generates a close
		 * event for me or closeEvent() removes #m_part from its parent. Before one causes
		 * the other to be called, #m_closing is set to true so as to avoid infinite
		 * recursion.
		 */
		bool m_closing;
		SubWindowStatus m_status;
};

#endif // ifndef ASPECT_VIEW_H
