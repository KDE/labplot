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
#include "backend/core/AbstractColumn.h"

#include <QFont>
#include <QPen>

class XYCurvePrivate;
class XYCurve: public WorksheetElement {
	Q_OBJECT

	public:
		enum LineType {NoLine, Line, StartHorizontal, StartVertical, MidpointHorizontal, MidpointVertical, Segments2, Segments3,
					   SplineCubicNatural, SplineCubicPeriodic, SplineAkimaNatural, SplineAkimaPeriodic};
		enum DropLineType {NoDropLine, DropLineX, DropLineY, DropLineXY, DropLineXZeroBaseline, DropLineXMinBaseline, DropLineXMaxBaseline};
		enum ValuesType {NoValues, ValuesX, ValuesY, ValuesXY, ValuesXYBracketed, ValuesCustomColumn};
		enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
		enum ErrorType {NoError, SymmetricError, AsymmetricError};
		enum FillingPosition {NoFilling, FillingAbove, FillingBelow, FillingZeroBaseline, FillingLeft, FillingRight};
		enum ErrorBarsType {ErrorBarsSimple, ErrorBarsWithEnds};
		enum DataSourceType {DataSourceSpreadsheet, DataSourceCurve};

		explicit XYCurve(const QString &name);
		virtual ~XYCurve();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);
		virtual void loadThemeConfig(const KConfig& config);
		virtual void saveThemeConfig(const KConfig& config);

		BASIC_D_ACCESSOR_DECL(DataSourceType, dataSourceType, DataSourceType)
		POINTER_D_ACCESSOR_DECL(const XYCurve, dataSourceCurve, DataSourceCurve)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
		const QString& dataSourceCurvePath() const;
		const QString& xColumnPath() const;
		const QString& yColumnPath() const;

		BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType)
		BASIC_D_ACCESSOR_DECL(bool, lineSkipGaps, LineSkipGaps)
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
		const QString& valuesColumnPath() const;
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
		const QString& xErrorPlusColumnPath() const;
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorMinusColumn, XErrorMinusColumn)
		const QString& xErrorMinusColumnPath() const;
		BASIC_D_ACCESSOR_DECL(ErrorType, yErrorType, YErrorType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorPlusColumn, YErrorPlusColumn)
		const QString& yErrorPlusColumnPath() const;
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorMinusColumn, YErrorMinusColumn)
		const QString& yErrorMinusColumnPath() const;
		BASIC_D_ACCESSOR_DECL(ErrorBarsType, errorBarsType, ErrorBarsType)
		BASIC_D_ACCESSOR_DECL(qreal, errorBarsCapSize, ErrorBarsCapSize)
		CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
		BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool on);
		void suppressRetransform(bool);

		typedef WorksheetElement BaseClass;
		typedef XYCurvePrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

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

		//SLOTs for changes triggered via QActions in the context menu
		void visibilityChanged();
		void navigateTo();

	protected:
		XYCurve(const QString& name, XYCurvePrivate* dd);
		XYCurvePrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(XYCurve)
		void init();
		void initActions();

		QAction* visibilityAction;
		QAction* navigateToAction;

	signals:
		//General-Tab
		void dataChanged();
		void xDataChanged();
		void yDataChanged();
		void visibilityChanged(bool);

		friend class XYCurveSetDataSourceTypeCmd;
		friend class XYCurveSetDataSourceCurveCmd;
		friend class XYCurveSetXColumnCmd;
		friend class XYCurveSetYColumnCmd;
		void dataSourceTypeChanged(XYCurve::DataSourceType);
		void dataSourceCurveChanged(const XYCurve*);
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);

		//Line-Tab
		friend class XYCurveSetLineTypeCmd;
		friend class XYCurveSetLineSkipGapsCmd;
		friend class XYCurveSetLineInterpolationPointsCountCmd;
		friend class XYCurveSetLinePenCmd;
		friend class XYCurveSetLineOpacityCmd;
		friend class XYCurveSetDropLineTypeCmd;
		friend class XYCurveSetDropLinePenCmd;
		friend class XYCurveSetDropLineOpacityCmd;
		void lineTypeChanged(XYCurve::LineType);
		void lineSkipGapsChanged(bool);
		void lineInterpolationPointsCountChanged(int);
		void linePenChanged(const QPen&);
		void lineOpacityChanged(qreal);
		void dropLineTypeChanged(XYCurve::DropLineType);
		void dropLinePenChanged(const QPen&);
		void dropLineOpacityChanged(qreal);

		//Symbol-Tab
		friend class XYCurveSetSymbolsStyleCmd;
		friend class XYCurveSetSymbolsSizeCmd;
		friend class XYCurveSetSymbolsRotationAngleCmd;
		friend class XYCurveSetSymbolsOpacityCmd;
		friend class XYCurveSetSymbolsBrushCmd;
		friend class XYCurveSetSymbolsPenCmd;
		void symbolsStyleChanged(Symbol::Style);
		void symbolsSizeChanged(qreal);
		void symbolsRotationAngleChanged(qreal);
		void symbolsOpacityChanged(qreal);
		void symbolsBrushChanged(QBrush);
		void symbolsPenChanged(const QPen&);

		//Values-Tab
		friend class XYCurveSetValuesColumnCmd;
		friend class XYCurveSetValuesTypeCmd;
		friend class XYCurveSetValuesPositionCmd;
		friend class XYCurveSetValuesDistanceCmd;
		friend class XYCurveSetValuesRotationAngleCmd;
		friend class XYCurveSetValuesOpacityCmd;
		friend class XYCurveSetValuesPrefixCmd;
		friend class XYCurveSetValuesSuffixCmd;
		friend class XYCurveSetValuesFontCmd;
		friend class XYCurveSetValuesColorCmd;
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
		friend class XYCurveSetFillingPositionCmd;
		friend class XYCurveSetFillingTypeCmd;
		friend class XYCurveSetFillingColorStyleCmd;
		friend class XYCurveSetFillingImageStyleCmd;
		friend class XYCurveSetFillingBrushStyleCmd;
		friend class XYCurveSetFillingFirstColorCmd;
		friend class XYCurveSetFillingSecondColorCmd;
		friend class XYCurveSetFillingFileNameCmd;
		friend class XYCurveSetFillingOpacityCmd;
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
		friend class XYCurveSetXErrorTypeCmd;
		friend class XYCurveSetXErrorPlusColumnCmd;
		friend class XYCurveSetXErrorMinusColumnCmd;
		friend class XYCurveSetYErrorTypeCmd;
		friend class XYCurveSetYErrorPlusColumnCmd;
		friend class XYCurveSetYErrorMinusColumnCmd;
		friend class XYCurveSetErrorBarsCapSizeCmd;
		friend class XYCurveSetErrorBarsTypeCmd;
		friend class XYCurveSetErrorBarsPenCmd;
		friend class XYCurveSetErrorBarsOpacityCmd;
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
