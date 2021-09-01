/*
    File                 : BoxPlot.h
    Project              : LabPlot
    Description          : Box Plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "backend/worksheet/plots/cartesian/Curve.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

class BoxPlotPrivate;
class AbstractColumn;
class Symbol;

class BoxPlot : public WorksheetElement, Curve {
	Q_OBJECT

public:
	enum class Ordering {None, MedianAscending, MedianDescending, MeanAscending, MeanDescending};
	enum class WhiskersType {MinMax, IQR, SD, SD_3, MAD, MAD_3, PERCENTILES_10_90, PERCENTILES_5_95, PERCENTILES_1_99};

	explicit BoxPlot(const QString&);
	~BoxPlot() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	//reimplemented from Curve
	bool activateCurve(QPointF mouseScenePos, double maxDist = -1) override;
	void setHover(bool on) override;

	//general
	BASIC_D_ACCESSOR_DECL(QVector<const AbstractColumn*>, dataColumns, DataColumns)
	QVector<QString>& dataColumnPaths() const;
	BASIC_D_ACCESSOR_DECL(BoxPlot::Ordering, ordering, Ordering)
	BASIC_D_ACCESSOR_DECL(BoxPlot::Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(bool, variableWidth, VariableWidth)
	BASIC_D_ACCESSOR_DECL(double, widthFactor, WidthFactor)
	BASIC_D_ACCESSOR_DECL(bool, notchesEnabled, NotchesEnabled)

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

	//symbols
	Symbol* symbolMean() const;
	Symbol* symbolMedian() const;
	Symbol* symbolOutlier() const;
	Symbol* symbolFarOut() const;
	Symbol* symbolData() const;
	BASIC_D_ACCESSOR_DECL(bool, jitteringEnabled, JitteringEnabled)

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
	void dataColumnsChanged(const QVector<const AbstractColumn*>&);
	void orderingChanged(BoxPlot::Ordering);
	void orientationChanged(BoxPlot::Orientation);
	void variableWidthChanged(bool);
	void widthFactorChanged(double);
	void visibilityChanged(bool);
	void notchesEnabledChanged(bool);

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

	//symbols
	void jitteringEnabledChanged(bool);

	//whiskers
	void whiskersTypeChanged(BoxPlot::WhiskersType);
	void whiskersPenChanged(QPen&);
	void whiskersOpacityChanged(float);
	void whiskersCapSizeChanged(double);
};

#endif
