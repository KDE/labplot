/***************************************************************************
    File                 : BoxPlot.h
    Project              : LabPlot
    Description          : Box Plot
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)
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

#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

class BoxPlotPrivate;
class AbstractColumn;

class BoxPlot : public WorksheetElement {
	Q_OBJECT

public:
	enum WhiskersType {MinMax, IQR, STDDEV};

	explicit BoxPlot(const QString&);
	~BoxPlot() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	//general
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	QString& dataColumnPath() const;

	//box filling
	BASIC_D_ACCESSOR_DECL(bool, fillingEnabled, FillingEnabled)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, fillingType, FillingType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, fillingColorStyle, FillingColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, fillingImageStyle, FillingImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, fillingBrushStyle, FillingBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, fillingFirstColor, FillingFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, fillingSecondColor, FillingSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, fillingFileName, FillingFileName)
	BASIC_D_ACCESSOR_DECL(qreal, fillingOpacity, FillingOpacity)

	//box border
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	//median line
	CLASS_D_ACCESSOR_DECL(QPen, medianLinePen, MedianLinePen)
	BASIC_D_ACCESSOR_DECL(qreal, medianLineOpacity, MedianLineOpacity)

	//markers
	BASIC_D_ACCESSOR_DECL(Symbol::Style, symbolOutliersStyle, SymbolOutliersStyle)
	BASIC_D_ACCESSOR_DECL(Symbol::Style, symbolMeanStyle, SymbolMeanStyle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsOpacity, SymbolsOpacity)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsRotationAngle, SymbolsRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, symbolsSize, SymbolsSize)
	CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush)
	CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen)

	//whiskers
	BASIC_D_ACCESSOR_DECL(BoxPlot::WhiskersType, whiskersType, WhiskersType)
	CLASS_D_ACCESSOR_DECL(QPen, whiskersPen, WhiskersPen)
	BASIC_D_ACCESSOR_DECL(qreal, whiskersOpacity, WhiskersOpacity)
	BASIC_D_ACCESSOR_DECL(double, whiskersCapSize, WhiskersCapSize)

	void setVisible(bool on) override;
	bool isVisible() const override;

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	double xMinimum() const;
	double xMaximum() const;
	double yMinimum() const;
	double yMaximum() const;

	typedef BoxPlotPrivate Private;

protected:
	BoxPlotPrivate* const d_ptr;
	BoxPlot(const QString& name, BoxPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(BoxPlot)
	void init();
	void initActions();
	void initMenus();

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};
	QAction* visibilityAction{nullptr};
	QMenu* orientationMenu{nullptr};

public slots:
	void recalc();

private slots:
	//SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void visibilityChangedSlot();

	void dataColumnAboutToBeRemoved(const AbstractAspect*);

signals:
	//General-Tab
	void dataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void visibilityChanged(bool);

	//box filling
	void fillingEnabledChanged(bool);
	void fillingTypeChanged(PlotArea::BackgroundType);
	void fillingColorStyleChanged(PlotArea::BackgroundColorStyle);
	void fillingImageStyleChanged(PlotArea::BackgroundImageStyle);
	void fillingBrushStyleChanged(Qt::BrushStyle);
	void fillingFirstColorChanged(QColor&);
	void fillingSecondColorChanged(QColor&);
	void fillingFileNameChanged(QString&);
	void fillingOpacityChanged(float);

	//box border
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);

	//median line
	void medianLinePenChanged(QPen&);
	void medianLineOpacityChanged(float);

	//symbols for outliers and for the mean
	void symbolOutliersStyleChanged(Symbol::Style);
	void symbolMeanStyleChanged(Symbol::Style);
	void symbolsSizeChanged(qreal);
	void symbolsRotationAngleChanged(qreal);
	void symbolsOpacityChanged(qreal);
	void symbolsBrushChanged(QBrush);
	void symbolsPenChanged(const QPen&);

	//whiskers
	void whiskersTypeChanged(BoxPlot::WhiskersType);
	void whiskersPenChanged(QPen&);
	void whiskersOpacityChanged(float);
	void whiskersCapSizeChanged(double);
};

#endif
