/***************************************************************************
    File                 : LinearAxis.h
    Project              : LabPlot/SciDAVis
    Description          : Linear axis for cartesian coordinate systems.
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

#ifndef LINEARAXIS_H
#define LINEARAXIS_H

#include "worksheet/AbstractWorksheetElement.h"
#include "lib/macros.h"

class LinearAxisPrivate;
class LinearAxis: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum AxisFlags {
			axisHorizontal = 0x01,
			axisVertical = 0x02,
			axisNormalTicks = 0x04,
			axisInvertedTicks = 0x08,
			axisLeft = axisVertical | axisNormalTicks,
			axisRight = axisVertical | axisInvertedTicks,
			axisBottom = axisHorizontal | axisNormalTicks,
			axisTop = axisHorizontal | axisInvertedTicks,
		};
		Q_DECLARE_FLAGS(AxisOrientation, AxisFlags)

		enum TicksFlags {
			noTicks = 0x00,
			ticksIn = 0x01,
			ticksOut = 0x02,
			ticksBoth = 0x03,
		};
		Q_DECLARE_FLAGS(TicksDirection, TicksFlags)
			
		LinearAxis(const QString &name, const AxisOrientation &orientation);
		virtual ~LinearAxis();

		virtual QGraphicsItem *graphicsItem() const;

		CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
		BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
		BASIC_D_ACCESSOR_DECL(qreal, start, Start);
		BASIC_D_ACCESSOR_DECL(qreal, end, End);
		BASIC_D_ACCESSOR_DECL(qreal, tickStart, TickStart);
		BASIC_D_ACCESSOR_DECL(qreal, tickEnd, TickEnd);
		BASIC_D_ACCESSOR_DECL(int, majorTickCount, MajorTickCount);
		BASIC_D_ACCESSOR_DECL(int, minorTickCount, MinorTickCount);
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
		CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
		CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
		BASIC_D_ACCESSOR_DECL(qreal, labelRotationAngle, LabelRotationAngle);
		BASIC_D_ACCESSOR_DECL(qreal, labelFontSize, LabelFontSize);
		CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor);
		CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont);
		CLASS_D_ACCESSOR_DECL(QPointF, labelOffset, LabelOffset);

		//TODO: all style related stuff (line widths, color, ...)

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

		typedef LinearAxisPrivate Private;

	public slots:
		virtual void retransform();

	protected:
		LinearAxisPrivate * const d_ptr;
		LinearAxis(const QString &name, const AxisOrientation &orientation, LinearAxisPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(LinearAxis)
		void init();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LinearAxis::AxisOrientation)
Q_DECLARE_OPERATORS_FOR_FLAGS(LinearAxis::TicksDirection)

#endif


