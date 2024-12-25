/*
	File                 : XYCurve.h
	Project              : LabPlot
	Description          : A xy-curve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVE_H
#define XYCURVE_H

#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/Plot.h"

#include <QFont>

class AbstractColumn;
class Background;
class Line;
class Symbol;
class XYCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYCurve : public Plot {
#else
class XYCurve : public Plot {
#endif
	Q_OBJECT

public:
	friend class XYCurveSetXColumnCmd;
	friend class XYCurveSetYColumnCmd;
	friend class XYCurveSetXErrorPlusColumnCmd;
	friend class XYCurveSetXErrorMinusColumnCmd;
	friend class XYCurveSetYErrorPlusColumnCmd;
	friend class XYCurveSetYErrorMinusColumnCmd;
	friend class XYCurveSetValuesColumnCmd;

	enum class LineType {
		NoLine,
		Line,
		StartHorizontal,
		StartVertical,
		MidpointHorizontal,
		MidpointVertical,
		Segments2,
		Segments3,
		SplineCubicNatural,
		SplineCubicPeriodic,
		SplineAkimaNatural,
		SplineAkimaPeriodic
	};
	enum class DropLineType { NoDropLine, X, Y, XY, XZeroBaseline, XMinBaseline, XMaxBaseline };
	enum class ValuesType { NoValues, X, Y, XY, XYBracketed, CustomColumn };
	enum class ValuesPosition { Above, Under, Left, Right };

	explicit XYCurve(const QString& name, AspectType type = AspectType::XYCurve, bool loading = false);
	~XYCurve() override;

	void setPlotType(Plot::PlotType);

	QIcon icon() const override;
	static QIcon staticIcon(XYCurve::PlotType type);
	QMenu* createContextMenu() override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;
	double y(double x, bool& valueFound) const;
	QDateTime yDateTime(double x, bool& valueFound) const;

	bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension dim) const override;
	double maximum(CartesianCoordinateSystem::Dimension dim) const override;
	bool hasData() const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	QColor color() const override;

	const AbstractColumn* column(CartesianCoordinateSystem::Dimension dim) const;
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)

	BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType)
	BASIC_D_ACCESSOR_DECL(bool, lineSkipGaps, LineSkipGaps)
	BASIC_D_ACCESSOR_DECL(bool, lineIncreasingXOnly, LineIncreasingXOnly)
	BASIC_D_ACCESSOR_DECL(int, lineInterpolationPointsCount, LineInterpolationPointsCount)
	Line* line() const;
	Line* dropLine() const;

	Symbol* symbol() const;
	Background* background() const;

	BASIC_D_ACCESSOR_DECL(ValuesType, valuesType, ValuesType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, valuesColumn, ValuesColumn)
	CLASS_D_ACCESSOR_DECL(QString, valuesColumnPath, ValuesColumnPath)
	BASIC_D_ACCESSOR_DECL(ValuesPosition, valuesPosition, ValuesPosition)
	BASIC_D_ACCESSOR_DECL(qreal, valuesDistance, ValuesDistance)
	BASIC_D_ACCESSOR_DECL(qreal, valuesRotationAngle, ValuesRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, valuesOpacity, ValuesOpacity)
	BASIC_D_ACCESSOR_DECL(char, valuesNumericFormat, ValuesNumericFormat)
	BASIC_D_ACCESSOR_DECL(int, valuesPrecision, ValuesPrecision)
	CLASS_D_ACCESSOR_DECL(QString, valuesDateTimeFormat, ValuesDateTimeFormat)
	CLASS_D_ACCESSOR_DECL(QString, valuesPrefix, ValuesPrefix)
	CLASS_D_ACCESSOR_DECL(QString, valuesSuffix, ValuesSuffix)
	CLASS_D_ACCESSOR_DECL(QColor, valuesColor, ValuesColor)
	CLASS_D_ACCESSOR_DECL(QFont, valuesFont, ValuesFont)

	ErrorBar* errorBar() const;

	// margin plots
	BASIC_D_ACCESSOR_DECL(bool, rugEnabled, RugEnabled)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::Orientation, rugOrientation, RugOrientation)
	BASIC_D_ACCESSOR_DECL(double, rugOffset, RugOffset)
	BASIC_D_ACCESSOR_DECL(double, rugLength, RugLength)
	BASIC_D_ACCESSOR_DECL(double, rugWidth, RugWidth)

	bool isSourceDataChangedSinceLastRecalc() const;

	typedef XYCurvePrivate Private;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	double y(double x, double& x_new, bool& valueFound) const;
	int getNextValue(double xpos, int index, double& x, double& y, bool& valueFound) const;

private Q_SLOTS:
	void updateValues();
	void updateErrorBars();
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void valuesColumnAboutToBeRemoved(const AbstractAspect*);

	// SLOTs for changes triggered via QActions in the context menu
	void navigateTo();

protected:
	XYCurve(const QString& name, XYCurvePrivate* dd, AspectType type);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect*) override;

private:
	Q_DECLARE_PRIVATE(XYCurve)
	void init(bool loading = false);
	void initActions();
	void connectXColumn(const AbstractColumn*);
	void connectYColumn(const AbstractColumn*);
	void connectValuesColumn(const AbstractColumn*);

	bool minMax(const AbstractColumn* column1,
				const AbstractColumn* column2,
				const ErrorBar::ErrorType errorType,
				const AbstractColumn* errorPlusColumn,
				const AbstractColumn* errorMinusColumn,
				const Range<int>& indexRange,
				Range<double>& yRange,
				bool includeErrorBars) const;

	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const XYCurve*, const QVector<QLineF>&);

	// General-Tab
	void xDataChanged();
	void yDataChanged();
	void valuesDataChanged();
	void selected(double pos);

	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);

	// Line-Tab
	void lineTypeChanged(XYCurve::LineType);
	void lineSkipGapsChanged(bool);
	void lineIncreasingXOnlyChanged(bool);
	void lineInterpolationPointsCountChanged(int);
	void dropLineTypeChanged(XYCurve::DropLineType);

	// Values-Tab
	void valuesTypeChanged(XYCurve::ValuesType);
	void valuesColumnChanged(const AbstractColumn*);
	void valuesPositionChanged(XYCurve::ValuesPosition);
	void valuesDistanceChanged(qreal);
	void valuesRotationAngleChanged(qreal);
	void valuesOpacityChanged(qreal);
	void valuesNumericFormatChanged(char);
	void valuesPrecisionChanged(int);
	void valuesDateTimeFormatChanged(QString);
	void valuesPrefixChanged(QString);
	void valuesSuffixChanged(QString);
	void valuesFontChanged(QFont);
	void valuesColorChanged(QColor);

	// Margin Plots
	void rugEnabledChanged(bool);
	void rugOrientationChanged(WorksheetElement::Orientation);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);

	friend class RetransformTest;
	friend class XYCurveTest;
	friend class FourierTest;
	friend class FitTest;
};

#endif
