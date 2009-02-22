/***************************************************************************
    File                 : AbstractPart.cpp
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

#include "core/AbstractPart.h"
#include "core/PartMdiView.h"
#include <QMenu>
#include <QStyle>

PartMdiView* AbstractPart::mdiSubWindow() const
{
	if (!m_mdi_window)
		m_mdi_window = new PartMdiView(const_cast<AbstractPart*>(this), view());
	return m_mdi_window;
}

QMenu* AbstractPart::createContextMenu()
{
	QMenu * menu = AbstractAspect::createContextMenu();
	Q_ASSERT(menu);
	const QStyle *widget_style = m_mdi_window->style();
   	
	QAction *action_temp;
	if(m_mdi_window->windowState() & (Qt::WindowMinimized | Qt::WindowMaximized))
	{
		action_temp = menu->addAction(tr("&Restore"), m_mdi_window, SLOT(showNormal()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarNormalButton));
	}
	if(!(m_mdi_window->windowState() & Qt::WindowMinimized))
	{
		action_temp = menu->addAction(tr("Mi&nimize"), m_mdi_window, SLOT(showMinimized()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarMinButton));
	}
	if(!(m_mdi_window->windowState() & Qt::WindowMaximized))
	{
		action_temp = menu->addAction(tr("Ma&ximize"), m_mdi_window, SLOT(showMaximized()));
		action_temp->setIcon(widget_style->standardIcon(QStyle::SP_TitleBarMaxButton));
	}

	return menu;
}

