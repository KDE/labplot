/***************************************************************************
    File                 : AbstractPart.cpp
    Project              : SciDAVis
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
	Copyright            : (C) 2012 Alexander Semke (alexander.semke*web.de)
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

#include "core/AbstractPart.h"
#include "core/PartMdiView.h"
#include <QMenu>
#include <QStyle>

/**
 * \class AbstractPart
 * \brief Base class of Aspects with MDI windows as views (AspectParts).
 */
AbstractPart::AbstractPart(const QString &name) : AbstractAspect(name),
	m_mdiWindow(0), m_view(0) {
}

/**
 * \fn QWidget *AbstractPart::view() const
 * \brief Construct a primary view on me.
 *
 * The caller recieves ownership of the view.
 *
 * This method may be called multiple times during the life time of a Part, or it might not get
 * called at all. Parts must not depend on the existence of a view for their operation.
 */

/**
 * \brief Wrap the view() into a PartMdiView.
 *
 * A new view is only created the first time this method is called;
 * after that, a pointer to the pre-existing view is returned.
 */
PartMdiView* AbstractPart::mdiSubWindow() const
{
	if (!m_mdiWindow)
		m_mdiWindow = new PartMdiView(const_cast<AbstractPart*>(this), view());
	return m_mdiWindow;
}

void AbstractPart::deleteView() const {
	if (m_view) {
		delete m_view;
		m_view = 0;
		m_mdiWindow = 0;
	}
}

/**
 * \brief Return AbstractAspect::createContextMenu() plus operations on the primary view.
 */
QMenu* AbstractPart::createContextMenu(){
	QMenu * menu = AbstractAspect::createContextMenu();
	Q_ASSERT(menu);
	menu->addSeparator();
	const QStyle *widget_style = m_mdiWindow->style();
   	
	QAction *action_temp;
	if(m_mdiWindow->windowState() & (Qt::WindowMinimized | Qt::WindowMaximized))	{
		action_temp = menu->addAction(tr("&Restore"), m_mdiWindow, SLOT(showNormal()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarNormalButton));
	}
	
	if(!(m_mdiWindow->windowState() & Qt::WindowMinimized))	{
		action_temp = menu->addAction(tr("Mi&nimize"), m_mdiWindow, SLOT(showMinimized()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarMinButton));
	}
	
	if(!(m_mdiWindow->windowState() & Qt::WindowMaximized))	{
		action_temp = menu->addAction(tr("Ma&ximize"), m_mdiWindow, SLOT(showMaximized()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarMaxButton));
	}

	//TODO add print and print-preview actions
	
	return menu;
}

/**
 * \brief Fill the part specific menu for the main window including setting the title
 *
 * \return true on success, otherwise false (e.g. part has no actions).
 */
bool AbstractPart::fillProjectMenu(QMenu * menu) {
	Q_UNUSED(menu);
	return false;
}

/**
 * \brief Copy current selection.
 */
void AbstractPart::editCopy() {}

/**
 * \brief Cut current selection.
 */
void AbstractPart::editCut() {}

/**
 * \brief Paste at the current location or into the current selection.
 */
void AbstractPart::editPaste() {}

/**
 * \var AbstractPart::m_mdiWindow
 * \brief The MDI sub-window that is wrapped around my primary view.
 */

