/***************************************************************************
    File                 : Axis.h
    Project              : LabPlot/SciDAVis
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
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

#ifndef  AXISNEW_H
#define AXISNEW_H

#include "worksheet/AbstractWorksheetElement.h"
#include "lib/macros.h"

class AxisPrivate;
class Axis: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum AxisFlags {
			axisHorizontal = 0x01,
			axisVertical = 0x02,
			axisNormalTicks = 0x04,
			axisInvertedTicks = 0x08,
			axisCustom = 0x10,
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
		
		enum TicksType {TicksTotalNumber, TicksIncrement};
		enum AxisScale {ScaleLinear, ScaleLog10, ScaleLog2, ScaleLn, ScaleSqrt};
		
		Axis(const QString &name, const AxisOrientation &orientation);
		virtual ~Axis();

		virtual QGraphicsItem *graphicsItem() const;

		CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
		BASIC_D_ACCESSOR_DECL(AxisScale, scale, Scale);
		BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
		BASIC_D_ACCESSOR_DECL(qreal, start, Start);
		BASIC_D_ACCESSOR_DECL(qreal, end, End);
		
		CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen);
		BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity);
		
		BASIC_D_ACCESSOR_DECL(qreal, tickStart, TickStart);
		BASIC_D_ACCESSOR_DECL(qreal, tickEnd, TickEnd);
		
		CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
		CLASS_D_ACCESSOR_DECL(TicksType, majorTicksType, MajorTicksType);
		BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber);
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksIncrement, MajorTicksIncrement);
		CLASS_D_ACCESSOR_DECL(QPen, majorTicksPen, MajorTicksPen);
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksOpacity, MajorTicksOpacity);
		
		CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
		CLASS_D_ACCESSOR_DECL(TicksType, minorTicksType, MinorTicksType);
		BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber);
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksIncrement, MinorTicksIncrement);
		CLASS_D_ACCESSOR_DECL(QPen, minorTicksPen, MinorTicksPen);
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksOpacity, MinorTicksOpacity);
		
		BASIC_D_ACCESSOR_DECL(qreal, labelRotationAngle, LabelRotationAngle);
		BASIC_D_ACCESSOR_DECL(qreal, labelFontSize, LabelFontSize);
		CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor);
		CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont);
		CLASS_D_ACCESSOR_DECL(QPointF, labelOffset, LabelOffset);

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

		typedef AbstractWorksheetElement BaseClass;
		typedef AxisPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		AxisPrivate * const d_ptr;
		Axis(const QString &name, const AxisOrientation &orientation, AxisPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(Axis)
		void init();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::AxisOrientation)
Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::TicksDirection)

#endif
