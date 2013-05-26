/***************************************************************************
    File                 : XYCurve.h
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as line and/or symbols
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

class XYCurvePrivate;
class XYCurve: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum LineType {NoLine, Line, StartHorizontal, StartVertical, MidpointHorizontal, MidpointVertical, Segments2, Segments3, 
					   SplineCubicNatural, SplineCubicPeriodic, SplineAkimaNatural, SplineAkimaPeriodic};
		enum DropLineType {NoDropLine, DropLineX, DropLineY, DropLineXY};
		enum ValuesType {NoValues, ValuesX, ValuesY, ValuesXY, ValuesXYBracketed, ValuesCustomColumn};
		enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
		enum ErrorType {NoError, SymmetricError, AsymmetricError};
		enum ErrorBarsType {ErrorBars, ErrorBarsWithEnds};

		static QStringList lineTypeStrings();
		static QStringList dropLineTypeStrings();
		static QStringList valuesTypeStrings();
		static QStringList valuesPositionStrings();
		
		XYCurve(const QString &name);
		virtual ~XYCurve();

		virtual QIcon icon() const;
		virtual QGraphicsItem *graphicsItem() const;
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
		QString& xColumnName() const;
		QString& yColumnName() const;
		QString& xColumnParentName() const;
		QString& yColumnParentName() const;

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
		BASIC_D_ACCESSOR_DECL(qreal, symbolsAspectRatio, SymbolsAspectRatio)
		CLASS_D_ACCESSOR_DECL(QString, symbolsTypeId, SymbolsTypeId)
		CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush)
		CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen)
		
		BASIC_D_ACCESSOR_DECL(ValuesType, valuesType, ValuesType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, valuesColumn, ValuesColumn)
		QString& valuesColumnName() const;
		QString& valuesColumnParentName() const;		
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
		QString& xErrorPlusColumnName() const;
		QString& xErrorPlusColumnParentName() const;	
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xErrorMinusColumn, XErrorMinusColumn)
		QString& xErrorMinusColumnName() const;
		QString& xErrorMinusColumnParentName() const;
		BASIC_D_ACCESSOR_DECL(ErrorType, yErrorType, YErrorType)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorPlusColumn, YErrorPlusColumn)
		QString& yErrorPlusColumnName() const;
		QString& yErrorPlusColumnParentName() const;		
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yErrorMinusColumn, YErrorMinusColumn)
		QString& yErrorMinusColumnName() const;
		QString& yErrorMinusColumnParentName() const;		
		BASIC_D_ACCESSOR_DECL(ErrorBarsType, errorBarsType, ErrorBarsType)
		CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
		BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)
		
		//TODO: signal/slot connections with columns

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

		typedef AbstractWorksheetElement BaseClass;
		typedef XYCurvePrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	private slots:
		void updateValues();
		void updateErrorBars();
		void xColumnAboutToBeRemoved();
		void yColumnAboutToBeRemoved();
		void valuesColumnAboutToBeRemoved();
		void xErrorPlusColumnAboutToBeRemoved();
		void xErrorMinusColumnAboutToBeRemoved();
		void yErrorPlusColumnAboutToBeRemoved();
		void yErrorMinusColumnAboutToBeRemoved();

	protected:
		XYCurve(const QString &name, XYCurvePrivate *dd);
		XYCurvePrivate * const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(XYCurve)
		void init();

	signals:
		//general
		void xDataChanged();
		void yDataChanged();

		friend class XYCurveSetXColumnCmd;
		friend class XYCurveSetYColumnCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);
		
		//values
		friend class XYCurveSetValuesColumnCmd;
		void valuesColumnChanged(const AbstractColumn*);

		//error bars
		friend class XYCurveSetXErrorPlusColumnCmd;
		friend class XYCurveSetXErrorMinusColumnCmd;
		friend class XYCurveSetYErrorPlusColumnCmd;
		friend class XYCurveSetYErrorMinusColumnCmd;
		void xErrorPlusColumnChanged(const AbstractColumn*);
		void xErrorMinusColumnChanged(const AbstractColumn*);
		void yErrorPlusColumnChanged(const AbstractColumn*);
		void yErrorMinusColumnChanged(const AbstractColumn*);		
};

#endif
