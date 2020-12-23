/***************************************************************************
    File                 : Axis.h
    Project              : LabPlot
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "backend/lib/Range.h"
#include "backend/lib/macros.h"

class CartesianPlot;
class TextLabel;
class AxisPrivate;
class AbstractColumn;
class QActionGroup;

class Axis: public WorksheetElement {
	Q_OBJECT

public:
	enum class Position {Top, Bottom, Left, Right, Centered, Custom};
	enum class LabelsFormat {Decimal, ScientificE, Powers10, Powers2, PowersE, MultipliesPi, Scientific};
	enum TicksFlags {
		noTicks = 0x00,
		ticksIn = 0x01,
		ticksOut = 0x02,
		ticksBoth = 0x03,
	};
	Q_DECLARE_FLAGS(TicksDirection, TicksFlags)

	enum class TicksType {TotalNumber, Spacing, CustomColumn, CustomValues};
	enum class ArrowType {NoArrow, SimpleSmall, SimpleBig, FilledSmall, FilledBig, SemiFilledSmall, SemiFilledBig};
	enum class ArrowPosition {Left, Right, Both};
	//TODO: use CartesianPlot::Scale?
	enum class Scale {Linear, Log10, Log2, Ln, Sqrt, X2};
	enum class LabelsPosition {NoLabels, In, Out};
	enum class LabelsBackgroundType {Transparent, Color};

	typedef AxisPrivate Private;	// for Axis::Private used in macros instead of AxisPrivate

	explicit Axis(const QString&, Orientation orientation = Orientation::Horizontal);
	~Axis() override;

	void finalizeAdd() override;
	QIcon icon() const override;
	QMenu* createContextMenu() override;

	QGraphicsItem* graphicsItem() const override;
	void setZValue(qreal) override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(bool, autoScale, AutoScale)
	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(Position, position, Position)
	BASIC_D_ACCESSOR_DECL(Scale, scale, Scale)
	BASIC_D_ACCESSOR_DECL(Range<double>, range, Range)
	void setStart(const double);
	void setEnd(const double);
	void setRange(const double, const double);
	void setOffset(const double, const bool=true);
	double offset() const;
	BASIC_D_ACCESSOR_DECL(qreal, scalingFactor, ScalingFactor)
	BASIC_D_ACCESSOR_DECL(qreal, zeroOffset, ZeroOffset)

	POINTER_D_ACCESSOR_DECL(TextLabel, title, Title)
	BASIC_D_ACCESSOR_DECL(double, titleOffsetX, TitleOffsetX)
	BASIC_D_ACCESSOR_DECL(double, titleOffsetY, TitleOffsetY)

	CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
	BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)
	BASIC_D_ACCESSOR_DECL(ArrowType, arrowType, ArrowType)
	BASIC_D_ACCESSOR_DECL(ArrowPosition, arrowPosition, ArrowPosition)
	BASIC_D_ACCESSOR_DECL(double, arrowSize, ArrowSize)

	BASIC_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection)
	BASIC_D_ACCESSOR_DECL(TicksType, majorTicksType, MajorTicksType)
	BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber)
	BASIC_D_ACCESSOR_DECL(qreal, majorTicksSpacing, MajorTicksSpacing)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, majorTicksColumn, MajorTicksColumn)
	QString& majorTicksColumnPath() const;
	CLASS_D_ACCESSOR_DECL(QPen, majorTicksPen, MajorTicksPen)
	BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength)
	BASIC_D_ACCESSOR_DECL(qreal, majorTicksOpacity, MajorTicksOpacity)

	BASIC_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection)
	BASIC_D_ACCESSOR_DECL(TicksType, minorTicksType, MinorTicksType)
	BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber)
	BASIC_D_ACCESSOR_DECL(qreal, minorTicksSpacing, MinorTicksSpacing)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, minorTicksColumn, MinorTicksColumn)
	QString& minorTicksColumnPath() const;
	CLASS_D_ACCESSOR_DECL(QPen, minorTicksPen, MinorTicksPen)
	BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength)
	BASIC_D_ACCESSOR_DECL(qreal, minorTicksOpacity, MinorTicksOpacity)

	BASIC_D_ACCESSOR_DECL(LabelsFormat, labelsFormat, LabelsFormat)
	BASIC_D_ACCESSOR_DECL(bool, labelsAutoPrecision, LabelsAutoPrecision)
	BASIC_D_ACCESSOR_DECL(int, labelsPrecision, LabelsPrecision)
	CLASS_D_ACCESSOR_DECL(QString, labelsDateTimeFormat, LabelsDateTimeFormat)
	BASIC_D_ACCESSOR_DECL(LabelsPosition, labelsPosition, LabelsPosition)
	BASIC_D_ACCESSOR_DECL(qreal, labelsOffset, LabelsOffset)
	BASIC_D_ACCESSOR_DECL(qreal, labelsRotationAngle, LabelsRotationAngle)
	CLASS_D_ACCESSOR_DECL(QColor, labelsColor, LabelsColor)
	CLASS_D_ACCESSOR_DECL(QFont, labelsFont, LabelsFont)
	BASIC_D_ACCESSOR_DECL(LabelsBackgroundType, labelsBackgroundType, LabelsBackgroundType)
	CLASS_D_ACCESSOR_DECL(QColor, labelsBackgroundColor, LabelsBackgroundColor)
	CLASS_D_ACCESSOR_DECL(QString, labelsPrefix, LabelsPrefix)
	CLASS_D_ACCESSOR_DECL(QString, labelsSuffix, LabelsSuffix)
	BASIC_D_ACCESSOR_DECL(qreal, labelsOpacity, LabelsOpacity)

	CLASS_D_ACCESSOR_DECL(QPen, majorGridPen, MajorGridPen)
	BASIC_D_ACCESSOR_DECL(qreal, majorGridOpacity, MajorGridOpacity)
	CLASS_D_ACCESSOR_DECL(QPen, minorGridPen, MinorGridPen)
	BASIC_D_ACCESSOR_DECL(qreal, minorGridOpacity, MinorGridOpacity)

	void setDefault(bool);
	bool isDefault() const;

	void setVisible(bool) override;
	bool isVisible() const override;

	void setPrinting(bool) override;
	bool isHovered() const;
	void setSuppressRetransform(bool);
	void retransform() override;
	void retransformTickLabelStrings();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	int coordinateSystemIndex() const;
	void setCoordinateSystemIndex(int);
	int coordinateSystemCount() const;
	QString coordinateSystemInfo(int index) const;

protected:
	AxisPrivate* const d_ptr;
	Axis(const QString&, Orientation, AxisPrivate*);
	TextLabel* m_title;

private:
	Q_DECLARE_PRIVATE(Axis)
	void init();
	void initActions();
	void initMenus();

	QAction* visibilityAction{nullptr};
	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};

	QActionGroup* orientationActionGroup{nullptr};
	QActionGroup* lineStyleActionGroup{nullptr};
	QActionGroup* lineColorActionGroup{nullptr};

	QMenu* orientationMenu{nullptr};
	QMenu* lineMenu{nullptr};
	QMenu* lineStyleMenu{nullptr};
	QMenu* lineColorMenu{nullptr};

private slots:
	void labelChanged();
	void retransformTicks();
	void majorTicksColumnAboutToBeRemoved(const AbstractAspect*);
	void minorTicksColumnAboutToBeRemoved(const AbstractAspect*);

	//SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void lineStyleChanged(QAction*);
	void lineColorChanged(QAction*);
	void visibilityChangedSlot();

signals:
	void orientationChanged(Orientation);
	void positionChanged(Position);
	void positionChanged(double);
	void scaleChanged(Scale);
	void startChanged(double);
	void autoScaleChanged(bool);
	void endChanged(double);
	void rangeChanged(Range<double>);
	void zeroOffsetChanged(qreal);
	void scalingFactorChanged(qreal);

	//title
	void titleOffsetXChanged(qreal);
	void titleOffsetYChanged(qreal);

	// line
	void linePenChanged(const QPen&);
	void lineOpacityChanged(qreal);
	void arrowTypeChanged(Axis::ArrowType);
	void arrowPositionChanged(Axis::ArrowPosition);
	void arrowSizeChanged(qreal);

	// major ticks
	void majorTicksDirectionChanged(Axis::TicksDirection);
	void majorTicksTypeChanged(Axis::TicksType);
	void majorTicksNumberChanged(int);
	void majorTicksSpacingChanged(qreal);
	void majorTicksColumnChanged(const AbstractColumn*);
	void majorTicksPenChanged(QPen);
	void majorTicksLengthChanged(qreal);
	void majorTicksOpacityChanged(qreal);

	// minor ticks
	void minorTicksDirectionChanged(Axis::TicksDirection);
	void minorTicksTypeChanged(Axis::TicksType);
	void minorTicksNumberChanged(int);
	void minorTicksIncrementChanged(qreal);
	void minorTicksColumnChanged(const AbstractColumn*);
	void minorTicksPenChanged(QPen);
	void minorTicksLengthChanged(qreal);
	void minorTicksOpacityChanged(qreal);

	//labels
	void labelsFormatChanged(Axis::LabelsFormat);
	void labelsAutoPrecisionChanged(bool);
	void labelsPrecisionChanged(int);
	void labelsDateTimeFormatChanged(const QString&);
	void labelsPositionChanged(Axis::LabelsPosition);
	void labelsOffsetChanged(double);
	void labelsRotationAngleChanged(qreal);
	void labelsColorChanged(QColor);
	void labelsFontChanged(QFont);
	void labelsBackgroundTypeChanged(Axis::LabelsBackgroundType);
	void labelsBackgroundColorChanged(QColor);
	void labelsPrefixChanged(QString);
	void labelsSuffixChanged(QString);
	void labelsOpacityChanged(qreal);

	void majorGridPenChanged(QPen);
	void majorGridOpacityChanged(qreal);
	void minorGridPenChanged(QPen);
	void minorGridOpacityChanged(qreal);

	void visibilityChanged(bool);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::TicksDirection)

#endif
