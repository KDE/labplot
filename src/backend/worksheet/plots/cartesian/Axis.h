/***************************************************************************
    File                 : Axis.h
    Project              : LabPlot/SciDAVis
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2012 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2013 Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
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

#ifndef AXISNEW_H
#define AXISNEW_H

#include "backend/worksheet/AbstractWorksheetElement.h"
#include "backend/lib/macros.h"
#include <QActionGroup>

class TextLabel;
class AxisPrivate;

class Axis: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum AxisOrientation {AxisHorizontal, AxisVertical};
		enum AxisPosition {AxisTop, AxisBottom, AxisLeft, AxisRight, AxisCustom};
		enum LabelsFormat {FormatDecimal, FormatScientificE};
		enum TicksFlags {
			noTicks = 0x00,
			ticksIn = 0x01,
			ticksOut = 0x02,
			ticksBoth = 0x03,
		};
		Q_DECLARE_FLAGS(TicksDirection, TicksFlags)

		enum TicksType {TicksTotalNumber, TicksIncrement};
		enum AxisScale {ScaleLinear, ScaleLog10, ScaleLog2, ScaleLn, ScaleSqrt, ScaleX2};
		enum LabelsPosition {NoLabels, LabelsIn, LabelsOut};
		
		Axis(const QString &name, const AxisOrientation &orientation = AxisHorizontal);
		virtual ~Axis();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		BASIC_D_ACCESSOR_DECL(bool, autoScale, AutoScale)
		BASIC_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation)
		BASIC_D_ACCESSOR_DECL(AxisPosition, position, Position)
		BASIC_D_ACCESSOR_DECL(AxisScale, scale, Scale)
		void setOffset(const float, const bool=true);
		float offset() const;
		void setStart(const float, const bool=true);
		float start() const;
		void setEnd(const float, const bool=true);
		float end() const;		
		BASIC_D_ACCESSOR_DECL(qreal, scalingFactor, ScalingFactor)
		BASIC_D_ACCESSOR_DECL(qreal, zeroOffset, ZeroOffset)

		POINTER_D_ACCESSOR_DECL(TextLabel, title, Title)
		BASIC_D_ACCESSOR_DECL(float, titleOffset, TitleOffset)
		
		CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
		BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)
		
		BASIC_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection)
		BASIC_D_ACCESSOR_DECL(TicksType, majorTicksType, MajorTicksType)
		BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksIncrement, MajorTicksIncrement)
		CLASS_D_ACCESSOR_DECL(QPen, majorTicksPen, MajorTicksPen)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksOpacity, MajorTicksOpacity)
		
		BASIC_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection)
		BASIC_D_ACCESSOR_DECL(TicksType, minorTicksType, MinorTicksType)
		BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber)
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksIncrement, MinorTicksIncrement)
		CLASS_D_ACCESSOR_DECL(QPen, minorTicksPen, MinorTicksPen)
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength)
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksOpacity, MinorTicksOpacity)
		
		BASIC_D_ACCESSOR_DECL(LabelsFormat, labelsFormat, LabelsFormat)
		BASIC_D_ACCESSOR_DECL(bool, labelsAutoPrecision, LabelsAutoPrecision)
		BASIC_D_ACCESSOR_DECL(int, labelsPrecision, LabelsPrecision)
		BASIC_D_ACCESSOR_DECL(LabelsPosition, labelsPosition, LabelsPosition)
		BASIC_D_ACCESSOR_DECL(float, labelsOffset, LabelsOffset)
		BASIC_D_ACCESSOR_DECL(qreal, labelsRotationAngle, LabelsRotationAngle)
		CLASS_D_ACCESSOR_DECL(QColor, labelsColor, LabelsColor)
		CLASS_D_ACCESSOR_DECL(QFont, labelsFont, LabelsFont)
		CLASS_D_ACCESSOR_DECL(QString, labelsPrefix, LabelsPrefix)
		CLASS_D_ACCESSOR_DECL(QString, labelsSuffix, LabelsSuffix)
		BASIC_D_ACCESSOR_DECL(qreal, labelsOpacity, LabelsOpacity)

		CLASS_D_ACCESSOR_DECL(QPen, majorGridPen, MajorGridPen)
		BASIC_D_ACCESSOR_DECL(qreal, majorGridOpacity, MajorGridOpacity)
		CLASS_D_ACCESSOR_DECL(QPen, minorGridPen, MinorGridPen)
		BASIC_D_ACCESSOR_DECL(qreal, minorGridOpacity, MinorGridOpacity)
		
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
		void initActions();
		void initMenus();

		QAction* orientationHorizontalAction;
		QAction* orientationVerticalAction;
		
		QActionGroup* orientationActionGroup;
		QActionGroup* lineStyleActionGroup;
		QActionGroup* lineColorActionGroup;
		
		QMenu* orientationMenu;
		QMenu* lineMenu;
		QMenu* lineStyleMenu;
		QMenu* lineColorMenu;
		
	private slots:
		void labelChanged();
		
		//SLOTs for changes triggered via QActions in the context menu
		void orientationChanged(QAction*);
		void lineStyleChanged(QAction*);
		void lineColorChanged(QAction*);
		
	signals:
		friend class AxisSetOrientationCmd;
		friend class AxisSetLinePenCmd;
		void orientationChanged(Axis::AxisOrientation);
		void linePenChanged(const QPen&);

		void startChanged(float);
		void endChanged(float);
		void positionChanged(float);

		void labelsPrecisionChanged(int);
		friend class AxisSetZeroOffsetCmd;
		friend class AxisSetScalingFactorCmd;
		void zeroOffsetChanged(qreal);
		void scalingFactorChanged(qreal);

		friend class AxisSetTitleOffsetCmd;
		void titleOffsetChanged(float);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::TicksDirection)

#endif
