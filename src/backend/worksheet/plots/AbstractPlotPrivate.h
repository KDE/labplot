/***************************************************************************
    File                 : AbstractPlotPrivate.h
    Project              : LabPlot
    Description          : Private members of AbstractPlot
    -------------------------------------------------------------------
    Copyright            : (C) 2012-2015 Alexander Semke (alexander.semke@web.de)

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

#ifndef ABSTRACTPLOTPRIVATE_H
#define ABSTRACTPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementContainerPrivate.h"


class AbstractPlotPrivate:public WorksheetElementContainerPrivate {
public:
	explicit AbstractPlotPrivate(AbstractPlot* owner);
	~AbstractPlotPrivate() override = default;
	virtual QString name() const;
	virtual void retransform() {}

	float horizontalPadding; //horiz. offset between the plot area and the area defining the coodinate system, in scene units
	float verticalPadding; //vert. offset between the plot area and the area defining the coodinate system, in scene units
};

#endif
