/***************************************************************************
    File                 : PartMdiView.h
    Project              : LabPlot
    Description          : QMdiSubWindow wrapper for aspect views.
    --------------------------------------------------------------------
    Copyright            : (C) 2013 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007,2008 Knut Franke (knut.franke@gmx.de)

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
#ifndef PART_MDI_VIEW_H
#define PART_MDI_VIEW_H

#include <QMdiSubWindow>

class AbstractAspect;
class AbstractPart;

class PartMdiView : public QMdiSubWindow {
	Q_OBJECT

public:
	explicit PartMdiView(AbstractPart* part);
	AbstractPart* part() const;

private:
	void closeEvent(QCloseEvent*) override;
	AbstractPart* m_part;

private slots:
	void handleAspectDescriptionChanged(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
};

#endif // ifndef PART_MDI_VIEW_H
