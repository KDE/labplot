/***************************************************************************
    File                 : Histogram.h
    Project              : LabPlot
    Description          : Histogram
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)

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

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/macros.h"
#include "backend/core/AbstractColumn.h"

#include <QFont>
#include <QPen>

class HistogramPrivate;
class Histogram: public WorksheetElement {
	Q_OBJECT

	public:
		enum ValuesType {NoValues, ValuesX, ValuesY, ValuesXY, ValuesXYBracketed, ValuesCustomColumn};
		enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
		enum FillingPosition {NoFilling, FillingAbove, FillingBelow, FillingZeroBaseline, FillingLeft, FillingRight};
		
		explicit Histogram(const QString &name);
		virtual ~Histogram();

        virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
		QString& xColumnPath() const;
		QString& yColumnPath() const;
		
		BASIC_D_ACCESSOR_DECL(float, xMin, XMin)
		BASIC_D_ACCESSOR_DECL(float, xMax, XMax)
		BASIC_D_ACCESSOR_DECL(float, yMin, YMin)
		BASIC_D_ACCESSOR_DECL(float, yMax, YMax)


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

		BASIC_D_ACCESSOR_DECL(FillingPosition, fillingPosition, FillingPosition)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, fillingType, FillingType)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, fillingColorStyle, FillingColorStyle)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, fillingImageStyle, FillingImageStyle)
		BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, fillingBrushStyle, FillingBrushStyle)
		CLASS_D_ACCESSOR_DECL(QColor, fillingFirstColor, FillingFirstColor)
		CLASS_D_ACCESSOR_DECL(QColor, fillingSecondColor, FillingSecondColor)
		CLASS_D_ACCESSOR_DECL(QString, fillingFileName, FillingFileName)
		BASIC_D_ACCESSOR_DECL(qreal, fillingOpacity, FillingOpacity)
		
		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool on);
		void suppressRetransform(bool);

		typedef WorksheetElement BaseClass;
		typedef HistogramPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);
		void scaleAutoX();
		void scaleAutoY();
		
	private slots:
		void updateValues();
		void xColumnAboutToBeRemoved(const AbstractAspect*);
		void yColumnAboutToBeRemoved(const AbstractAspect*);
		void valuesColumnAboutToBeRemoved(const AbstractAspect*);
		//SLOTs for changes triggered via QActions in the context menu
		void visibilityChanged();

	protected:
		Histogram(const QString& name, HistogramPrivate* dd);
		HistogramPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(Histogram)
		void init();
		void initActions();

		QAction* visibilityAction;

	signals:
		//General-Tab
		void dataChanged();
		void xHistogramDataChanged();
		void yHistogramDataChanged();
		void visibilityChanged(bool);

		friend class HistogramSetXColumnCmd;
		friend class HistogramSetYColumnCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);

		//Values-Tab
		friend class HistogramSetValuesColumnCmd;
		friend class HistogramSetValuesTypeCmd;
		friend class HistogramSetValuesPositionCmd;
		friend class HistogramSetValuesDistanceCmd;
		friend class HistogramSetValuesRotationAngleCmd;
		friend class HistogramSetValuesOpacityCmd;
		friend class HistogramSetValuesPrefixCmd;
		friend class HistogramSetValuesSuffixCmd;
		friend class HistogramSetValuesFontCmd;
		friend class HistogramSetValuesColorCmd;
		void valuesTypeChanged(Histogram::ValuesType);
		void valuesColumnChanged(const AbstractColumn*);
		void valuesPositionChanged(Histogram::ValuesPosition);
		void valuesDistanceChanged(qreal);
		void valuesRotationAngleChanged(qreal);
		void valuesOpacityChanged(qreal);
		void valuesPrefixChanged(QString);
		void valuesSuffixChanged(QString);
		void valuesFontChanged(QFont);
		void valuesColorChanged(QColor);

		//Filling
		friend class HistogramSetFillingPositionCmd;
		friend class HistogramSetFillingTypeCmd;
		friend class HistogramSetFillingColorStyleCmd;
		friend class HistogramSetFillingImageStyleCmd;
		friend class HistogramSetFillingBrushStyleCmd;
		friend class HistogramSetFillingFirstColorCmd;
		friend class HistogramSetFillingSecondColorCmd;
		friend class HistogramSetFillingFileNameCmd;
		friend class HistogramSetFillingOpacityCmd;
		void fillingPositionChanged(Histogram::FillingPosition);
		void fillingTypeChanged(PlotArea::BackgroundType);
		void fillingColorStyleChanged(PlotArea::BackgroundColorStyle);
		void fillingImageStyleChanged(PlotArea::BackgroundImageStyle);
		void fillingBrushStyleChanged(Qt::BrushStyle);
		void fillingFirstColorChanged(QColor&);
		void fillingSecondColorChanged(QColor&);
		void fillingFileNameChanged(QString&);
		void fillingOpacityChanged(float);

};

#endif
