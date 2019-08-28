/***************************************************************************
    File                 : XYCurve.h
    Project              : LabPlot
    Description          : A xy-curve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef XYCURVE_H
#define XYCURVE_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/macros.h"
#include "backend/lib/macrosXYCurve.h"
#include "backend/core/AbstractColumn.h"

#include <QFont>
#include <QPen>

class XYCurvePrivate;

class XYCurve: public WorksheetElement {
	Q_OBJECT

public:
	friend class XYCurveSetXColumnCmd;
	friend class XYCurveSetYColumnCmd;
	friend class XYCurveSetXErrorPlusColumnCmd;
	friend class XYCurveSetXErrorMinusColumnCmd;
	friend class XYCurveSetYErrorPlusColumnCmd;
	friend class XYCurveSetYErrorMinusColumnCmd;
	friend class XYCurveSetValuesColumnCmd;
	enum LineType {NoLine, Line, StartHorizontal, StartVertical, MidpointHorizontal, MidpointVertical, Segments2, Segments3,
	               SplineCubicNatural, SplineCubicPeriodic, SplineAkimaNatural, SplineAkimaPeriodic
	              };
	enum DropLineType {NoDropLine, DropLineX, DropLineY, DropLineXY, DropLineXZeroBaseline, DropLineXMinBaseline, DropLineXMaxBaseline};
	enum ValuesType {NoValues, ValuesX, ValuesY, ValuesXY, ValuesXYBracketed, ValuesCustomColumn};
	enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
	enum ErrorType {NoError, SymmetricError, AsymmetricError};
	enum FillingPosition {NoFilling, FillingAbove, FillingBelow, FillingZeroBaseline, FillingLeft, FillingRight};
	enum ErrorBarsType {ErrorBarsSimple, ErrorBarsWithEnds};

	explicit XYCurve(const QString &name, AspectType type = AspectType::XYCurve);
	~XYCurve() override;

	void finalizeAdd() override;
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;
	double y(double x, bool &valueFound) const;
	QDateTime yDateTime(double x, bool &valueFound) const;
	bool minMax(const AbstractColumn *column, const ErrorType errorType, const AbstractColumn *errorPlusColumn, const AbstractColumn *errorMinusColumn, int indexMin, int indexMax, double& yMin, double& yMax, bool includeErrorBars) const;
	bool minMaxX(int indexMin, int indexMax, double& yMin, double& yMax, bool includeErrorBars = true) const;
	bool minMaxY(int indexMin, int indexMax, double& yMin, double& yMax, bool includeErrorBars = true) const;
	bool activateCurve(QPointF mouseScenePos, double maxDist = -1);
	void setHover(bool on);

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)

	BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType)
	BASIC_D_ACCESSOR_DECL(bool, lineSkipGaps, LineSkipGaps)
	BASIC_D_ACCESSOR_DECL(bool, lineIncreasingXOnly, LineIncreasingXOnly)
	BASIC_D_ACCESSOR_DECL(int, lineInterpolationPointsCount, LineInterpolationPointsCount)
	CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
	BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)

	BASIC_D_ACCESSOR_DECL(DropLineType, dropLineType, DropLineType)
	CLASS_D_ACCESSOR_DECL(QPen, dropLinePen, DropLinePen)
	BASIC_D_ACCESSOR_DECL(qreal, dropLineOpacity, DropLineOpacity)

	BASIC_D_ACCESSOR_DECL(Symbol::Style, symbolsStyle, SymbolsStyle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsOpacity, SymbolsOpacity)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsRotationAngle, SymbolsRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsSize, SymbolsSize)
	CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush)
	CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen)

	BASIC_D_ACCESSOR_DECL(ValuesType, valuesType, ValuesType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, valuesColumn, ValuesColumn)
	CLASS_D_ACCESSOR_DECL(QString, valuesColumnPath, ValuesColumnPath)
	BASIC_D_ACCESSOR_DECL(ValuesPosition, valuesPosition, ValuesPosition)
	BASIC_D_ACCESSOR_DECL(qreal, valuesDistance, ValuesDistance)
	BASIC_D_ACCESSOR_DECL(qreal, valuesRotationAngle, ValuesRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, valuesOpacity, ValuesOpacity)
	CLASS_D_ACCESSOR_DECL(QString, valuesPrefix, ValuesPrefix)
	CLASS_D_ACCESSOR_DECL(QString, valuesSuffix, ValuesSuffix)
	CLASS_D_ACCESSOR_DECL(QColor, valuesColor, ValuesColor)
	CLASS_D_ACCESSOR_DECL(QFont, valuesFont, ValuesFont)

	BASIC_D_ACCESSOR_DECL(FillingPosition, fillingPosition, FillingPosition)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, fillingType, FillingType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, fillingColorStyle, FillingColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, fillingImageStyle, FillingImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, fillingBrushStyle, FillingBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, fillingFirstColor, FillingFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, fillingSecondColor, FillingSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, fillingFileName, FillingFileName)
	BASIC_D_ACCESSOR_DECL(qreal, fillingOpacity, FillingOpacity)

	BASIC_D_ACCESSOR_DECL(ErrorType, xErrorType, XErrorType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorPlusColumn, XErrorPlusColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorMinusColumn, XErrorMinusColumn)
	BASIC_D_ACCESSOR_DECL(ErrorType, yErrorType, YErrorType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorPlusColumn, YErrorPlusColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorMinusColumn, YErrorMinusColumn)
	CLASS_D_ACCESSOR_DECL(QString, xErrorPlusColumnPath, XErrorPlusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, xErrorMinusColumnPath, XErrorMinusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yErrorPlusColumnPath, YErrorPlusColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yErrorMinusColumnPath, YErrorMinusColumnPath)

	BASIC_D_ACCESSOR_DECL(ErrorBarsType, errorBarsType, ErrorBarsType)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsCapSize, ErrorBarsCapSize)
	CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool on) override;
	void suppressRetransform(bool);
	bool isSourceDataChangedSinceLastRecalc() const;

	typedef XYCurvePrivate Private;

	void retransform() override;
	void recalcLogicalPoints();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

private slots:
	void updateValues();
	void updateErrorBars();
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void valuesColumnAboutToBeRemoved(const AbstractAspect*);
	void xErrorPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void xErrorMinusColumnAboutToBeRemoved(const AbstractAspect*);
	void yErrorPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void yErrorMinusColumnAboutToBeRemoved(const AbstractAspect*);
	void xColumnNameChanged();
	void yColumnNameChanged();
	void xErrorPlusColumnNameChanged();
	void xErrorMinusColumnNameChanged();
	void yErrorPlusColumnNameChanged();
	void yErrorMinusColumnNameChanged();
	void valuesColumnNameChanged();
	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();
	void navigateTo();

protected:
	XYCurve(const QString& name, XYCurvePrivate* dd, AspectType type);
	XYCurvePrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(XYCurve)
	void init();
	void initActions();
	XYCURVE_COLUMN_CONNECT(x)
	XYCURVE_COLUMN_CONNECT(y)
	XYCURVE_COLUMN_CONNECT(xErrorPlus)
	XYCURVE_COLUMN_CONNECT(xErrorMinus)
	XYCURVE_COLUMN_CONNECT(yErrorPlus)
	XYCURVE_COLUMN_CONNECT(yErrorMinus)
	XYCURVE_COLUMN_CONNECT(values)

	QAction* visibilityAction{nullptr};
	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

signals:
	//General-Tab
	void dataChanged(); //emitted when the actual curve data to be plotted was changed to re-adjust the plot
	void xDataChanged();
	void yDataChanged();
	void xErrorPlusDataChanged();
	void xErrorMinusDataChanged();
	void yErrorPlusDataChanged();
	void yErrorMinusDataChanged();
	void valuesDataChanged();
	void visibilityChanged(bool);

	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);

	//Line-Tab
	void lineTypeChanged(XYCurve::LineType);
	void lineSkipGapsChanged(bool);
	void lineIncreasingXOnlyChanged(bool);
	void lineInterpolationPointsCountChanged(int);
	void linePenChanged(const QPen&);
	void lineOpacityChanged(qreal);
	void dropLineTypeChanged(XYCurve::DropLineType);
	void dropLinePenChanged(const QPen&);
	void dropLineOpacityChanged(qreal);

	//Symbol-Tab
	void symbolsStyleChanged(Symbol::Style);
	void symbolsSizeChanged(qreal);
	void symbolsRotationAngleChanged(qreal);
	void symbolsOpacityChanged(qreal);
	void symbolsBrushChanged(QBrush);
	void symbolsPenChanged(const QPen&);

	//Values-Tab
	void valuesTypeChanged(XYCurve::ValuesType);
	void valuesColumnChanged(const AbstractColumn*);
	void valuesPositionChanged(XYCurve::ValuesPosition);
	void valuesDistanceChanged(qreal);
	void valuesRotationAngleChanged(qreal);
	void valuesOpacityChanged(qreal);
	void valuesPrefixChanged(QString);
	void valuesSuffixChanged(QString);
	void valuesFontChanged(QFont);
	void valuesColorChanged(QColor);

	//Filling
	void fillingPositionChanged(XYCurve::FillingPosition);
	void fillingTypeChanged(PlotArea::BackgroundType);
	void fillingColorStyleChanged(PlotArea::BackgroundColorStyle);
	void fillingImageStyleChanged(PlotArea::BackgroundImageStyle);
	void fillingBrushStyleChanged(Qt::BrushStyle);
	void fillingFirstColorChanged(QColor&);
	void fillingSecondColorChanged(QColor&);
	void fillingFileNameChanged(QString&);
	void fillingOpacityChanged(float);

	//Error bars
	void xErrorTypeChanged(XYCurve::ErrorType);
	void xErrorPlusColumnChanged(const AbstractColumn*);
	void xErrorMinusColumnChanged(const AbstractColumn*);
	void yErrorTypeChanged(XYCurve::ErrorType);
	void yErrorPlusColumnChanged(const AbstractColumn*);
	void yErrorMinusColumnChanged(const AbstractColumn*);
	void errorBarsCapSizeChanged(qreal);
	void errorBarsTypeChanged(XYCurve::ErrorBarsType);
	void errorBarsPenChanged(QPen);
	void errorBarsOpacityChanged(qreal);
};

#endif
