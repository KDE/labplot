/***************************************************************************
    File                 : Axis.h
    Project              : LabPlot
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
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

#include "backend/worksheet/WorksheetElement.h"
#include "backend/lib/macros.h"
#include <QActionGroup>

class TextLabel;
class AxisPrivate;
class AbstractColumn;

class Axis: public WorksheetElement {
	Q_OBJECT

	public:
		enum AxisOrientation {AxisHorizontal, AxisVertical};
		enum AxisPosition {AxisTop, AxisBottom, AxisLeft, AxisRight, AxisCentered, AxisCustom};
		enum LabelsFormat {FormatDecimal, FormatScientificE, FormatPowers10, FormatPowers2, FormatPowersE, FormatMultipliesPi};
		enum TicksFlags {
			noTicks = 0x00,
			ticksIn = 0x01,
			ticksOut = 0x02,
			ticksBoth = 0x03,
		};
		Q_DECLARE_FLAGS(TicksDirection, TicksFlags)

		enum TicksType {TicksTotalNumber, TicksIncrement, TicksCustomColumn, TicksCustomValues};
		enum ArrowType {NoArrow, SimpleArrowSmall, SimpleArrowBig, FilledArrowSmall, FilledArrowBig, SemiFilledArrowSmall, SemiFilledArrowBig};
		enum ArrowPosition {ArrowLeft, ArrowRight, ArrowBoth};
		enum AxisScale {ScaleLinear, ScaleLog10, ScaleLog2, ScaleLn, ScaleSqrt, ScaleX2};
		enum LabelsPosition {NoLabels, LabelsIn, LabelsOut};

		explicit Axis(const QString &name, const AxisOrientation &orientation = AxisHorizontal);
		virtual ~Axis();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();

		virtual QGraphicsItem* graphicsItem() const;
		virtual void setZValue(qreal);

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		BASIC_D_ACCESSOR_DECL(bool, autoScale, AutoScale)
		BASIC_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation)
		BASIC_D_ACCESSOR_DECL(AxisPosition, position, Position)
		BASIC_D_ACCESSOR_DECL(AxisScale, scale, Scale)
		BASIC_D_ACCESSOR_DECL(float, start, Start)
		BASIC_D_ACCESSOR_DECL(float, end, End)
		void setOffset(const float, const bool=true);
		float offset() const;
		BASIC_D_ACCESSOR_DECL(qreal, scalingFactor, ScalingFactor)
		BASIC_D_ACCESSOR_DECL(qreal, zeroOffset, ZeroOffset)

		POINTER_D_ACCESSOR_DECL(TextLabel, title, Title)
		BASIC_D_ACCESSOR_DECL(float, titleOffset, TitleOffset)

		CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
		BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)
		BASIC_D_ACCESSOR_DECL(ArrowType, arrowType, ArrowType)
		BASIC_D_ACCESSOR_DECL(ArrowPosition, arrowPosition, ArrowPosition)
		BASIC_D_ACCESSOR_DECL(float, arrowSize, ArrowSize)

		BASIC_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection)
		BASIC_D_ACCESSOR_DECL(TicksType, majorTicksType, MajorTicksType)
		BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksIncrement, MajorTicksIncrement)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, majorTicksColumn, MajorTicksColumn)
		QString& majorTicksColumnPath() const;
		CLASS_D_ACCESSOR_DECL(QPen, majorTicksPen, MajorTicksPen)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength)
		BASIC_D_ACCESSOR_DECL(qreal, majorTicksOpacity, MajorTicksOpacity)

		BASIC_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection)
		BASIC_D_ACCESSOR_DECL(TicksType, minorTicksType, MinorTicksType)
		BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber)
		BASIC_D_ACCESSOR_DECL(qreal, minorTicksIncrement, MinorTicksIncrement)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, minorTicksColumn, MinorTicksColumn)
		QString& minorTicksColumnPath() const;
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

		virtual void setVisible(bool);
		virtual bool isVisible() const;
		virtual void setPrinting(bool);

		typedef WorksheetElement BaseClass;
		typedef AxisPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		AxisPrivate * const d_ptr;
		Axis(const QString &name, const AxisOrientation &orientation, AxisPrivate *dd);
		TextLabel *m_title;

	private:
    	Q_DECLARE_PRIVATE(Axis)
		void init();
		void initActions();
		void initMenus();

		QAction* visibilityAction;
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
		void retransformTicks();
		void majorTicksColumnAboutToBeRemoved(const AbstractAspect*);
		void minorTicksColumnAboutToBeRemoved(const AbstractAspect*);

		//SLOTs for changes triggered via QActions in the context menu
		void orientationChanged(QAction*);
		void lineStyleChanged(QAction*);
		void lineColorChanged(QAction*);
		void visibilityChanged();

	signals:
		friend class AxisSetOrientationCmd;
		friend class AxisSetPositionCmd;
		friend class AxisSetScalingCmd;
		friend class AxisSetAutoScaleCmd;
		friend class AxisSetStartCmd;
		friend class AxisSetEndCmd;
		friend class AxisSetZeroOffsetCmd;
		friend class AxisSetScalingFactorCmd;
		void orientationChanged(Axis::AxisOrientation);
		void positionChanged(Axis::AxisPosition);
		void positionChanged(float);
		void scaleChanged(Axis::AxisScale);
		void startChanged(float);
		void autoScaleChanged(bool);
		void endChanged(float);
		void zeroOffsetChanged(qreal);
		void scalingFactorChanged(qreal);

		//title
		friend class AxisSetTitleOffsetCmd;
		void titleOffsetChanged(float);

		// line
		friend class AxisSetLinePenCmd;
		friend class AxisSetLineOpacityCmd;
		friend class AxisSetArrowTypeCmd;
		friend class AxisSetArrowPositionCmd;
		friend class AxisSetArrowSizeCmd;
		void linePenChanged(const QPen&);
		void lineOpacityChanged(qreal);
		void arrowTypeChanged(Axis::ArrowType);
		void arrowPositionChanged(Axis::ArrowPosition);
		void arrowSizeChanged(float);

		// major ticks
		friend class AxisSetMajorTicksDirectionCmd;
		friend class AxisSetMajorTicksTypeCmd;
		friend class AxisSetMajorTicksNumberCmd;
		friend class AxisSetMajorTicksIncrementCmd;
		friend class AxisSetMajorTicksColumnCmd;
		friend class AxisSetMajorTicksPenCmd;
		friend class AxisSetMajorTicksLengthCmd;
		friend class AxisSetMajorTicksOpacityCmd;
		void majorTicksDirectionChanged(Axis::TicksDirection);
		void majorTicksTypeChanged(Axis::TicksType);
		void majorTicksNumberChanged(int);
		void majorTicksIncrementChanged(qreal);
		void majorTicksColumnChanged(const AbstractColumn*);
		void majorTicksPenChanged(QPen);
		void majorTicksLengthChanged(qreal);
		void majorTicksOpacityChanged(qreal);

		// minor ticks
		friend class AxisSetMinorTicksDirectionCmd;
		friend class AxisSetMinorTicksTypeCmd;
		friend class AxisSetMinorTicksNumberCmd;
		friend class AxisSetMinorTicksIncrementCmd;
		friend class AxisSetMinorTicksColumnCmd;
		friend class AxisSetMinorTicksPenCmd;
		friend class AxisSetMinorTicksLengthCmd;
		friend class AxisSetMinorTicksOpacityCmd;
		void minorTicksDirectionChanged(Axis::TicksDirection);
		void minorTicksTypeChanged(Axis::TicksType);
		void minorTicksNumberChanged(int);
		void minorTicksIncrementChanged(qreal);
		void minorTicksColumnChanged(const AbstractColumn*);
		void minorTicksPenChanged(QPen);
		void minorTicksLengthChanged(qreal);
		void minorTicksOpacityChanged(qreal);

		//labels
		friend class AxisSetLabelsFormatCmd;
		friend class AxisSetLabelsAutoPrecisionCmd;
		friend class AxisSetLabelsPrecisionCmd;
		friend class AxisSetLabelsPositionCmd;
		friend class AxisSetLabelsOffsetCmd;
		friend class AxisSetLabelsRotationAngleCmd;
		friend class AxisSetLabelsColorCmd;
		friend class AxisSetLabelsFontCmd;
		friend class AxisSetLabelsPrefixCmd;
		friend class AxisSetLabelsSuffixCmd;
		friend class AxisSetLabelsOpacityCmd;
		void labelsFormatChanged(Axis::LabelsFormat);
		void labelsAutoPrecisionChanged(bool);
		void labelsPrecisionChanged(int);
		void labelsPositionChanged(Axis::LabelsPosition);
		void labelsOffsetChanged(float);
		void labelsRotationAngleChanged(qreal);
		void labelsColorChanged(QColor);
		void labelsFontChanged(QFont);
		void labelsPrefixChanged(QString);
		void labelsSuffixChanged(QString);
		void labelsOpacityChanged(qreal);

		friend class AxisSetMajorGridPenCmd;
		friend class AxisSetMajorGridOpacityCmd;
		friend class AxisSetMinorGridPenCmd;
		friend class AxisSetMinorGridOpacityCmd;
		void majorGridPenChanged(QPen);
		void majorGridOpacityChanged(qreal);
		void minorGridPenChanged(QPen);
		void minorGridOpacityChanged(qreal);

		void visibilityChanged(bool);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::TicksDirection)

#endif
