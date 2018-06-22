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
#include "backend/core/AbstractColumn.h"
#include "backend/lib/macros.h"

class HistogramPrivate;

class Histogram: public WorksheetElement {
	Q_OBJECT

public:
	enum BinsOption {Number,Width,SquareRoot,RiceRule,SturgisRule};
	enum ValuesType {NoValues, ValuesY, ValuesYBracketed, ValuesCustomColumn};
	enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
	enum FillingPosition {NoFilling, FillingAbove, FillingBelow, FillingZeroBaseline, FillingLeft, FillingRight};
	enum HistogramType {Ordinary,Cumulative, AvgShift};
	enum BarsType {Vertical, Horizontal};

	struct HistogramData {
		HistogramData() : type(Ordinary),binsOption(Number), binValue(10) {};

		HistogramType type;
		BinsOption binsOption;
		int binValue;
	};
	explicit Histogram(const QString &name);
	//size_t bins;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(HistogramData, histogramData, HistogramData)

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
	CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
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

	void setVisible(bool on) override;
	bool isVisible() const override;
	void setPrinting(bool on) override;
	void suppressRetransform(bool);
	double getYMaximum() const;
	double getYMinimum() const;
	double getXMaximum() const;
	double getXMinimum() const;
	void setHistogramType(Histogram::HistogramType);
	void setBarsType(Histogram::BarsType);
	Histogram::HistogramType getHistrogramType();
	Histogram::BarsType getBarsType();
	void setbinsOption(Histogram::BinsOption);
	void setBinValue(int);

	typedef WorksheetElement BaseClass;
	typedef HistogramPrivate Private;

	bool isSourceDataChangedSinceLastPlot() const;

public slots:
	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

private slots:
	void updateValues();
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void valuesColumnAboutToBeRemoved(const AbstractAspect*);
	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChangedSlot();

	void handleSourceDataChanged();
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
	void HistogramDataChanged();
	void xHistogramDataChanged();
	void yHistogramDataChanged();
	void visibilityChanged(bool);

	friend class HistogramSetXColumnCmd;
	friend class HistogramSetYColumnCmd;
	friend class HistogramSetLinePenCmd;
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

	void linePenChanged(const QPen&);

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

	friend class HistogramSetDataCmd;
	void histogramDataChanged(const Histogram::HistogramData&);
	void sourceDataChangedSinceLastPlot();
};

#endif
