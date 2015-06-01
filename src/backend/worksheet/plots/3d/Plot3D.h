/***************************************************************************
    File                 : Plot3D.h
    Project              : LabPlot
    Description          : 3D plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#ifndef PLOT3D_H
#define PLOT3D_H

#include "backend/worksheet/plots/AbstractPlot.h"

class Plot3DPrivate;

class Plot3D:public AbstractPlot{
	Q_OBJECT

	public:
		explicit Plot3D(const QString &name);
		virtual ~Plot3D();

		QIcon icon() const;
		void setRect(const QRectF&);

		typedef Plot3D BaseClass;
		typedef Plot3DPrivate Private;

	protected:
		Plot3D(const QString &name, Plot3DPrivate *dd);

	private:
		Q_DECLARE_PRIVATE(Plot3D);

		void init();
};

#endif
