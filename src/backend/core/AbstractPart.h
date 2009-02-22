/***************************************************************************
    File                 : AbstractPart.h
    Project              : SciDAVis
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
                           (replace * with @ in the email address)

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
#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "AbstractAspect.h"

class PartMdiView;
class QMenu;

//! Base class of Aspects with MDI windows as views.
/**
 * SciDAVis's Parts are somewhat similar to KDE's KParts in that they are independent application
 * components running on top of a kernel (a bit like KOffice's shell).
 */
class AbstractPart : public AbstractAspect
{
	Q_OBJECT

	public:
		//! Constructor.
		AbstractPart(const QString &name) : AbstractAspect(name), m_mdi_window(0) {}
		virtual ~AbstractPart() {}
		//! Construct a primary view on me.
		/**
		 * The caller recieves ownership of the view.
		 *
		 * This method may be called multiple times during the life time of a Part, or it might not get
		 * called at all. Parts must not depend on the existence of a view for their operation.
		 */
		virtual QWidget * view() const = 0;
		//! Wrap the view() into a PartMdiView.
		/**
		 * A new view is only created the first time this method is called;
		 * after that, a pointer to the pre-existing view is returned.
		 */
		PartMdiView * mdiSubWindow() const;
		//! Return AbstractAspect::createContextMenu() plus operations on the primary view.
		virtual QMenu * createContextMenu();
		//! Fill the part specific menu for the main window including setting the title
		/**
		 * \return true on success, otherwise false (e.g. part has no actions).
		 */
		virtual bool fillProjectMenu(QMenu * menu) { Q_UNUSED(menu); return false; }

	public slots:
		//! Copy current selection.
		virtual void editCopy() {};
		//! Cut current selection.
		virtual void editCut() {};
		//! Paste at the current location or into the current selection.
		virtual void editPaste() {};

	private:
		//! The MDI sub-window that is wrapped around my primary view.
		mutable PartMdiView *m_mdi_window;
};

#endif // ifndef ABSTRACT_PART_H
