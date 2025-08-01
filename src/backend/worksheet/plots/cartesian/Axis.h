/*
	File                 : Axis.h
	Project              : LabPlot
	Description          : Axis for cartesian coordinate systems.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AXISNEW_H
#define AXISNEW_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

class AbstractColumn;
class AxisPrivate;
class Line;
class TextLabel;
class QActionGroup;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Axis : public WorksheetElement {
#else
class Axis : public WorksheetElement {
#endif
	Q_OBJECT

public:
	enum class RangeType { Auto, AutoData, Custom };
	enum class Position { Top, Bottom, Left, Right, Centered, Custom, Logical };
	enum class LabelsFormat { Decimal, ScientificE, Powers10, Powers2, PowersE, MultipliesPi, Scientific };
	Q_ENUM(LabelsFormat)
	enum TicksFlags {
		noTicks = 0x00,
		ticksIn = 0x01,
		ticksOut = 0x02,
		ticksBoth = 0x03,
	};
	Q_DECLARE_FLAGS(TicksDirection, TicksFlags)

	enum class TicksType { TotalNumber, Spacing, CustomColumn, CustomValues, ColumnLabels };
	Q_ENUM(TicksType)
	enum class ArrowType { NoArrow, SimpleSmall, SimpleBig, FilledSmall, FilledBig, SemiFilledSmall, SemiFilledBig };
	enum class ArrowPosition { Left, Right, Both };
	enum class LabelsPosition { NoLabels, In, Out };
	enum class LabelsTextType { PositionValues, CustomValues };
	enum class LabelsBackgroundType { Transparent, Color };

	enum class TicksStartType { Absolute, Offset };

	// LabelsFormat <-> index, see AxisDock::init()
	static int labelsFormatToIndex(LabelsFormat format) {
		switch (format) {
		case LabelsFormat::Decimal:
			return 0;
		case LabelsFormat::Scientific:
			return 1;
		case LabelsFormat::ScientificE:
			return 2;
		case LabelsFormat::Powers10:
			return 3;
		case LabelsFormat::Powers2:
			return 4;
		case LabelsFormat::PowersE:
			return 5;
		case LabelsFormat::MultipliesPi:
			return 6;
		}
		return 0;
	}
	static LabelsFormat indexToLabelsFormat(int index) {
		switch (index) {
		case 0:
			return LabelsFormat::Decimal;
		case 1:
			return LabelsFormat::Scientific;
		case 2:
			return LabelsFormat::ScientificE;
		case 3:
			return LabelsFormat::Powers10;
		case 4:
			return LabelsFormat::Powers2;
		case 5:
			return LabelsFormat::PowersE;
		case 6:
			return LabelsFormat::MultipliesPi;
		}
		return LabelsFormat::Decimal;
	}

	typedef AxisPrivate Private; // for Axis::Private used in macros instead of AxisPrivate

	explicit Axis(const QString&, Orientation = Orientation::Horizontal, bool loading = false);
	~Axis() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void setZValue(qreal) override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(RangeType, rangeType, RangeType)
	BASIC_D_ACCESSOR_DECL(Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(Position, position, Position)
	BASIC_D_ACCESSOR_DECL(Range<double>, range, Range)
	BASIC_D_ACCESSOR_DECL(RangeT::Scale, scale, Scale)
	void setStart(const double);
	void setEnd(const double);
	void setRange(const double, const double);
	void setOffset(const double, const bool = true);
	double offset() const;
	BASIC_D_ACCESSOR_DECL(qreal, scalingFactor, ScalingFactor)
	BASIC_D_ACCESSOR_DECL(qreal, zeroOffset, ZeroOffset)
	BASIC_D_ACCESSOR_DECL(bool, showScaleOffset, ShowScaleOffset)
	BASIC_D_ACCESSOR_DECL(double, logicalPosition, LogicalPosition)

	TextLabel* title() const;
	BASIC_D_ACCESSOR_DECL(double, titleOffsetX, TitleOffsetX)
	BASIC_D_ACCESSOR_DECL(double, titleOffsetY, TitleOffsetY)

	Line* line() const;
	BASIC_D_ACCESSOR_DECL(ArrowType, arrowType, ArrowType)
	BASIC_D_ACCESSOR_DECL(ArrowPosition, arrowPosition, ArrowPosition)
	BASIC_D_ACCESSOR_DECL(double, arrowSize, ArrowSize)

	BASIC_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection)
	BASIC_D_ACCESSOR_DECL(TicksType, majorTicksType, MajorTicksType)
	BASIC_D_ACCESSOR_DECL(bool, majorTicksAutoNumber, MajorTicksAutoNumber)
	int majorTicksNumber() const;
	void setMajorTicksNumber(const int number);
	BASIC_D_ACCESSOR_DECL(qreal, majorTicksSpacing, MajorTicksSpacing)
	BASIC_D_ACCESSOR_DECL(TicksStartType, majorTicksStartType, MajorTicksStartType)
	BASIC_D_ACCESSOR_DECL(qreal, majorTickStartOffset, MajorTickStartOffset)
	BASIC_D_ACCESSOR_DECL(qreal, majorTickStartValue, MajorTickStartValue)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, majorTicksColumn, MajorTicksColumn)
	QString& majorTicksColumnPath() const;
	Line* majorTicksLine() const;
	BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength)

	BASIC_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection)
	BASIC_D_ACCESSOR_DECL(TicksType, minorTicksType, MinorTicksType)
	BASIC_D_ACCESSOR_DECL(bool, minorTicksAutoNumber, MinorTicksAutoNumber)
	BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber)
	BASIC_D_ACCESSOR_DECL(qreal, minorTicksSpacing, MinorTicksSpacing)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, minorTicksColumn, MinorTicksColumn)
	QString& minorTicksColumnPath() const;
	Line* minorTicksLine() const;
	BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength)

	BASIC_D_ACCESSOR_DECL(LabelsFormat, labelsFormat, LabelsFormat)
	BASIC_D_ACCESSOR_DECL(bool, labelsFormatAuto, LabelsFormatAuto)
	BASIC_D_ACCESSOR_DECL(bool, labelsAutoPrecision, LabelsAutoPrecision)
	BASIC_D_ACCESSOR_DECL(int, labelsPrecision, LabelsPrecision)
	CLASS_D_ACCESSOR_DECL(QString, labelsDateTimeFormat, LabelsDateTimeFormat)
	BASIC_D_ACCESSOR_DECL(LabelsPosition, labelsPosition, LabelsPosition)
	BASIC_D_ACCESSOR_DECL(qreal, labelsOffset, LabelsOffset)
	BASIC_D_ACCESSOR_DECL(qreal, labelsRotationAngle, LabelsRotationAngle)
	BASIC_D_ACCESSOR_DECL(LabelsTextType, labelsTextType, LabelsTextType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, labelsTextColumn, LabelsTextColumn)
	QString& labelsTextColumnPath() const;
	QVector<double> tickLabelValues() const;
	QVector<QString> tickLabelStrings() const;
	CLASS_D_ACCESSOR_DECL(QColor, labelsColor, LabelsColor)
	CLASS_D_ACCESSOR_DECL(QFont, labelsFont, LabelsFont)
	BASIC_D_ACCESSOR_DECL(LabelsBackgroundType, labelsBackgroundType, LabelsBackgroundType)
	CLASS_D_ACCESSOR_DECL(QColor, labelsBackgroundColor, LabelsBackgroundColor)
	CLASS_D_ACCESSOR_DECL(QString, labelsPrefix, LabelsPrefix)
	CLASS_D_ACCESSOR_DECL(QString, labelsSuffix, LabelsSuffix)
	BASIC_D_ACCESSOR_DECL(qreal, labelsOpacity, LabelsOpacity)
	static int maxNumberMajorTicksCustomColumn();

	Line* majorGridLine() const;
	Line* minorGridLine() const;

	bool isNumeric() const;

	void setDefault(bool);
	bool isDefault() const;

	void setSuppressRetransform(bool);
	void retransform() override;
	void retransformTickLabelStrings();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void updateLocale() override;

protected:
	Axis(const QString&, Orientation, AxisPrivate*);
	TextLabel* m_title{nullptr};

private:
	Q_DECLARE_PRIVATE(Axis)
	void init(Orientation, bool loading = false);
	void initActions();
	void initMenus();
	BASIC_D_ACCESSOR_DECL(bool, rangeScale, RangeScale) // if true, the scale of the range will be used. Deprecated, just for compatibility

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};

	QActionGroup* orientationActionGroup{nullptr};
	QActionGroup* lineStyleActionGroup{nullptr};
	QActionGroup* lineColorActionGroup{nullptr};

	QMenu* orientationMenu{nullptr};
	QMenu* lineMenu{nullptr};
	QMenu* lineStyleMenu{nullptr};
	QMenu* lineColorMenu{nullptr};

private Q_SLOTS:
	void labelChanged();
	void retransformTicks();
	void majorTicksColumnAboutToBeRemoved(const AbstractAspect*);
	void minorTicksColumnAboutToBeRemoved(const AbstractAspect*);

	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void lineStyleChanged(QAction*);
	void lineColorChanged(QAction*);

Q_SIGNALS:
	void orientationChanged(Orientation);
	void positionChanged(Position);
	void positionChanged(double);
	void scaleChanged(RangeT::Scale);
	void rangeScaleChanged(bool);
	void startChanged(double);
	void rangeTypeChanged(RangeType);
	void endChanged(double);
	void rangeChanged(Range<double>);
	void zeroOffsetChanged(qreal);
	void scalingFactorChanged(qreal);
	void showScaleOffsetChanged(bool);
	void logicalPositionChanged(double);
	void shiftSignal(int delta, Dimension dim, int index);

	// title
	void titleOffsetXChanged(qreal);
	void titleOffsetYChanged(qreal);

	// line
	void arrowTypeChanged(ArrowType);
	void arrowPositionChanged(ArrowPosition);
	void arrowSizeChanged(qreal);

	// major ticks
	void majorTicksDirectionChanged(TicksDirection);
	void majorTicksTypeChanged(TicksType);
	void majorTicksAutoNumberChanged(bool);
	void majorTicksNumberChanged(int);
	void majorTicksSpacingChanged(qreal);
	void majorTicksColumnChanged(const AbstractColumn*);
	void majorTicksStartTypeChanged(TicksStartType);
	void majorTickStartOffsetChanged(qreal);
	void majorTickStartValueChanged(qreal);
	void majorTicksLengthChanged(qreal);

	// minor ticks
	void minorTicksDirectionChanged(TicksDirection);
	void minorTicksTypeChanged(TicksType);
	void minorTicksAutoNumberChanged(bool);
	void minorTicksNumberChanged(int);
	void minorTicksIncrementChanged(qreal);
	void minorTicksColumnChanged(const AbstractColumn*);
	void minorTicksLengthChanged(qreal);

	// labels
	void labelsFormatChanged(LabelsFormat);
	void labelsFormatAutoChanged(bool);
	void labelsAutoPrecisionChanged(bool);
	void labelsPrecisionChanged(int);
	void labelsDateTimeFormatChanged(const QString&);
	void labelsPositionChanged(LabelsPosition);
	void labelsOffsetChanged(double);
	void labelsRotationAngleChanged(qreal);
	void labelsTextTypeChanged(LabelsTextType);
	void labelsTextColumnChanged(const AbstractColumn*);
	void labelsColorChanged(QColor);
	void labelsFontChanged(QFont);
	void labelsBackgroundTypeChanged(LabelsBackgroundType);
	void labelsBackgroundColorChanged(QColor);
	void labelsPrefixChanged(QString);
	void labelsSuffixChanged(QString);
	void labelsOpacityChanged(qreal);

	friend class RetransformTest;
	friend class AxisTest;
	friend class AxisDock; // To be able to access range scale
	friend class Project; // To be able to access range scale
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Axis::TicksDirection)

#endif
