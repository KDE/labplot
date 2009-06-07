/***************************************************************************
    File                 : LogAxis.h
    Project              : LabPlot/SciDAVis
    Description          : Logarithmic axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef LOGAXIS_H
#define LOGAXIS_H

#include "worksheet/AbstractWorksheetElement.h"
#include "worksheet/LinearAxis.h"
#include "lib/macros.h"

class LogAxisPrivate;
class LogAxis: public LinearAxis {
	Q_OBJECT

	public:
		LogAxis(const QString &name, const AxisOrientation &orientation, double base);
		virtual ~LogAxis();

		typedef LogAxisPrivate Private;

	protected:
		LogAxis(const QString &name, const AxisOrientation &orientation, double base, LogAxisPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(LogAxis)
		void init(double base);
};

#endif


