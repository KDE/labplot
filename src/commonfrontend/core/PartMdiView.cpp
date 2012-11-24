/***************************************************************************
    File                 : PartMdiView.cpp
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
#include "core/PartMdiView.h"
#include "core/AbstractPart.h"

#include <QCloseEvent>
#include <QMenu>
#include <QMdiArea>

//TODO:currently, the only(?) purpose of this class is to change the name of the mdi sub window
//when the aspect name was changed. This can also be handled in MainWin.
//Check whether this class can be removed.

PartMdiView::PartMdiView(AbstractPart *part, QWidget * embedded_view)
	: QMdiSubWindow(0), m_part(part), m_closing(false), m_status(Closed)
{
	setWindowIcon(m_part->icon());
	handleAspectDescriptionChanged(m_part);
	connect(m_part, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), 
		this, SLOT(handleAspectDescriptionChanged(const AbstractAspect *)));
	connect(m_part, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
			this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	setWidget(embedded_view);
	setAttribute(Qt::WA_DeleteOnClose);
}

//TODO deactivated this feature because of the crash in worksheetview.
//decide later whether we really need a customised contextmenu for the mdi-subwindow bar.

// void PartMdiView::contextMenuEvent(QContextMenuEvent *event)
// {
// 	QMenu *menu = m_part->createContextMenu();
// 	Q_ASSERT(menu);
// 	menu->exec(event->globalPos());
// 	delete menu;
// }

PartMdiView::~PartMdiView()
{
}

void PartMdiView::handleAspectDescriptionChanged(const AbstractAspect *aspect)
{
	if (aspect != m_part) return;
	setWindowTitle(m_part->caption());
	update();
}

void PartMdiView::handleAspectAboutToBeRemoved(const AbstractAspect *aspect)
{
	if (aspect != m_part) return;
	m_closing = true;
	close();
}

void PartMdiView::closeEvent(QCloseEvent *event)
{
	if (!m_closing) {
		m_closing = true;
		m_part->deleteView();
		event->accept();
	}
	m_closing = false;
	
	SubWindowStatus old_status = m_status;
	m_status = Closed;
	emit statusChanged(this, old_status, m_status);
}

void PartMdiView::hideEvent(QHideEvent *event)
{
	SubWindowStatus old_status = m_status;
	m_status = Hidden;
	emit statusChanged(this, old_status, m_status);
	event->accept();
}

void PartMdiView::showEvent(QShowEvent *event)
{
	SubWindowStatus old_status = m_status;
	m_status = Visible;
	emit statusChanged(this, old_status, m_status);
	event->accept();
}

