/***************************************************************************
    File                 : PartMdiView.cpp
    Project              : LabPlot
    Description          : QMdiSubWindow wrapper for aspect views.
    --------------------------------------------------------------------
	Copyright            : (C) 2013 by Alexander Semke (alexander.semke*web.de)
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
#include <QIcon>

/*!
 * \class PartMdiView
 * 
 * \brief QMdiSubWindow wrapper for aspect views.
 * In addition to the functionality provided by QMdiSubWindow,
 * this class automatically updates the window title when
 * AbstractAspect::caption() is changed.
 */
//TODO:check whether m_status is somehow used or can be used in a meaningfull way.
PartMdiView::PartMdiView(AbstractPart *part, QWidget * embedded_view)
	: QMdiSubWindow(0), m_part(part), m_status(Closed) 
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

AbstractPart* PartMdiView::part() const {
	return m_part;
}

PartMdiView::SubWindowStatus PartMdiView::status() const {
	return m_status;
}

void PartMdiView::handleAspectDescriptionChanged(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;

	setWindowTitle(m_part->name());
	update();
}

void PartMdiView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;
	close();
}

void PartMdiView::closeEvent(QCloseEvent *event) {
	m_part->deleteView();
	event->accept();
	SubWindowStatus old_status = m_status;
	m_status = Closed;
	emit statusChanged(this, old_status, m_status);
}

void PartMdiView::hideEvent(QHideEvent *event) {
	SubWindowStatus old_status = m_status;
	m_status = Hidden;
	emit statusChanged(this, old_status, m_status);
	event->accept();
}

void PartMdiView::showEvent(QShowEvent *event) {
	SubWindowStatus old_status = m_status;
	m_status = Visible;
	emit statusChanged(this, old_status, m_status);
	event->accept();
}
