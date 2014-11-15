/***************************************************************************
    File                 : XYCurve.h
    Project              : LabPlot/SciDAVis
    Description          : A xy-curve
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010-2013 Alexander Semke (alexander.semke*web.de)
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

#ifndef XYCURVE_H
#define XYCURVE_H

#include "backend/worksheet/AbstractWorksheetElement.h"
#include "backend/lib/macros.h"
#include "backend/core/AbstractColumn.h"

#include <QFont>
#include <QPen>

class XYCurvePrivate;
class XYCurve: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum LineType {NoLine, Line, StartHorizontal, StartVertical, MidpointHorizontal, MidpointVertical, Segments2, Segments3,
					   SplineCubicNatural, SplineCubicPeriodic, SplineAkimaNatural, SplineAkimaPeriodic};
		enum DropLineType {NoDropLine, DropLineX, DropLineY, DropLineXY, DropLineXZeroBaseline, DropLineXMinBaseline, DropLineXMaxBaseline};
		enum ValuesType {NoValues, ValuesX, ValuesY, ValuesXY, ValuesXYBracketed, ValuesCustomColumn};
		enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
		enum ErrorType {NoError, SymmetricError, AsymmetricError};
		enum ErrorBarsType {ErrorBarsSimple, ErrorBarsWithEnds};

		explicit XYCurve(const QString &name);
		virtual ~XYCurve();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
		QString& xColumnPath() const;
		QString& yColumnPath() const;

		BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType)
		BASIC_D_ACCESSOR_DECL(int, lineInterpolationPointsCount, LineInterpolationPointsCount)
		CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
		BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)

		BASIC_D_ACCESSOR_DECL(DropLineType, dropLineType, DropLineType)
		CLASS_D_ACCESSOR_DECL(QPen, dropLinePen, DropLinePen)
		BASIC_D_ACCESSOR_DECL(qreal, dropLineOpacity, DropLineOpacity)

		BASIC_D_ACCESSOR_DECL(qreal, symbolsOpacity, SymbolsOpacity)
		BASIC_D_ACCESSOR_DECL(qreal, symbolsRotationAngle, SymbolsRotationAngle)
		BASIC_D_ACCESSOR_DECL(qreal, symbolsSize, SymbolsSize)
		CLASS_D_ACCESSOR_DECL(QString, symbolsTypeId, SymbolsTypeId)
		CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush)
		CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen)

		BASIC_D_ACCESSOR_DECL(ValuesType, valuesType, ValuesType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, valuesColumn, ValuesColumn)
		QString& valuesColumnPath() const;
		BASIC_D_ACCESSOR_DECL(ValuesPosition, valuesPosition, ValuesPosition)
		BASIC_D_ACCESSOR_DECL(qreal, valuesDistance, ValuesDistance)
		BASIC_D_ACCESSOR_DECL(qreal, valuesRotationAngle, ValuesRotationAngle)
		BASIC_D_ACCESSOR_DECL(qreal, valuesOpacity, ValuesOpacity)
		CLASS_D_ACCESSOR_DECL(QString, valuesPrefix, ValuesPrefix)
		CLASS_D_ACCESSOR_DECL(QString, valuesSuffix, ValuesSuffix)
		CLASS_D_ACCESSOR_DECL(QColor, valuesColor, ValuesColor)
		CLASS_D_ACCESSOR_DECL(QFont, valuesFont, ValuesFont)

		BASIC_D_ACCESSOR_DECL(ErrorType, xErrorType, XErrorType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorPlusColumn, XErrorPlusColumn)
		QString& xErrorPlusColumnPath() const;
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorMinusColumn, XErrorMinusColumn)
		QString& xErrorMinusColumnPath() const;
		BASIC_D_ACCESSOR_DECL(ErrorType, yErrorType, YErrorType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorPlusColumn, YErrorPlusColumn)
		QString& yErrorPlusColumnPath() const;
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorMinusColumn, YErrorMinusColumn)
		QString& yErrorMinusColumnPath() const;
		BASIC_D_ACCESSOR_DECL(ErrorBarsType, errorBarsType, ErrorBarsType)
		BASIC_D_ACCESSOR_DECL(qreal, errorBarsCapSize, ErrorBarsCapSize)
		CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
		BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool on);

		typedef AbstractWorksheetElement BaseClass;
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
		void curveVisibilityChanged();

	protected:
		XYCurve(const QString& name, XYCurvePrivate* dd);
		XYCurvePrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(XYCurve)
		void init();
		void initActions();

		QAction* visibilityAction;

	signals:
		//General-Tab
		void dataChanged();
		void xDataChanged();
		void yDataChanged();
		void visibilityChanged();

		friend class XYCurveSetXColumnCmd;
		friend class XYCurveSetYColumnCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);

		//Line-Tab
		friend class XYCurveSetLineTypeCmd;
		friend class XYCurveSetLineInterpolationPointsCountCmd;
		friend class XYCurveSetLinePenCmd;
		friend class XYCurveSetLineOpacityCmd;
		friend class XYCurveSetDropLineTypeCmd;
		friend class XYCurveSetDropLinePenCmd;
		friend class XYCurveSetDropLineOpacityCmd;
		void lineTypeChanged(XYCurve::LineType);
		void lineInterpolationPointsCountChanged(int);
		void linePenChanged(const QPen&);
		void lineOpacityChanged(qreal);
		void dropLineTypeChanged(XYCurve::DropLineType);
		void dropLinePenChanged(const QPen&);
		void dropLineOpacityChanged(qreal);

		//Symbol-Tab
		friend class XYCurveSetSymbolsTypeIdCmd;
		friend class XYCurveSetSymbolsSizeCmd;
		friend class XYCurveSetSymbolsRotationAngleCmd;
		friend class XYCurveSetSymbolsOpacityCmd;
		friend class XYCurveSetSymbolsAspectRatioCmd;
		friend class XYCurveSetSymbolsBrushCmd;
		friend class XYCurveSetSymbolsPenCmd;
		void symbolsTypeIdChanged(QString);
		void symbolsSizeChanged(qreal);
		void symbolsRotationAngleChanged(qreal);
		void symbolsOpacityChanged(qreal);
		void symbolsAspectRatioChanged(qreal);
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

		//"Error bars"-Tab
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
