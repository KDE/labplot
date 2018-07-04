/***************************************************************************
    File                 : PartMdiView.cpp
    Project              : LabPlot
    Description          : QMdiSubWindow wrapper for aspect views.
    --------------------------------------------------------------------
	Copyright            : (C) 2013 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)

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
#include "commonfrontend/core/PartMdiView.h"
#include "backend/core/AbstractPart.h"
#include "backend/worksheet/Worksheet.h"

#include <QCloseEvent>
#include <QIcon>

/*!
 * \class PartMdiView
 *
 * \brief QMdiSubWindow wrapper for aspect views.
 *
 * In addition to the functionality provided by QMdiSubWindow,
 * this class automatically updates the window title when AbstractAspect::caption() is changed
 * and holds the connection to the actuall data visualized in this window via the pointer to \c AbstractPart.
 */
PartMdiView::PartMdiView(AbstractPart* part) : QMdiSubWindow(0), m_part(part) {

	setWindowIcon(m_part->icon());
	setWidget(m_part->view());
	setAttribute(Qt::WA_DeleteOnClose);
	handleAspectDescriptionChanged(m_part);

	//resize the MDI sub window to fit the content of the view
	resize(m_part->view()->size());

	connect(m_part, &AbstractPart::aspectDescriptionChanged, this, &PartMdiView::handleAspectDescriptionChanged);
	connect(m_part, &AbstractPart::aspectAboutToBeRemoved, this, &PartMdiView::handleAspectAboutToBeRemoved);
}

AbstractPart* PartMdiView::part() const {
	return m_part;
}

void PartMdiView::handleAspectDescriptionChanged(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;

	setWindowTitle(m_part->name());
}

void PartMdiView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (aspect != m_part)
		return;

	close();
}

void PartMdiView::closeEvent(QCloseEvent *event) {
	m_part->deleteView();
	event->accept();
}
