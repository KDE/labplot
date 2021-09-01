/*
    File                 : CartesianPlotLegend.h
    Project              : LabPlot
    Description          : Legend for the cartesian plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2013-2018 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTLEGEND_H
#define CARTESIANPLOTLEGEND_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/lib/macros.h"

class CartesianPlotLegendPrivate;
class TextLabel;

class CartesianPlotLegend : public WorksheetElement {
	Q_OBJECT
	Q_ENUMS(HorizontalPosition)
	Q_ENUMS(VerticalPosition)

public:
	CartesianPlotLegend(const QString &name);
	~CartesianPlotLegend() override;

	void finalizeAdd() override;
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig& config) override;

	void setVisible(bool) override;
	bool isVisible() const override;

	TextLabel* title();

	CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont)
	CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor)
	BASIC_D_ACCESSOR_DECL(bool, labelColumnMajor, LabelColumnMajor)
	CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(float, lineSymbolWidth, LineSymbolWidth)

	BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(float, borderCornerRadius, BorderCornerRadius)
	BASIC_D_ACCESSOR_DECL(float, borderOpacity, BorderOpacity)

	BASIC_D_ACCESSOR_DECL(float, layoutTopMargin, LayoutTopMargin)
	BASIC_D_ACCESSOR_DECL(float, layoutBottomMargin, LayoutBottomMargin)
	BASIC_D_ACCESSOR_DECL(float, layoutLeftMargin, LayoutLeftMargin)
	BASIC_D_ACCESSOR_DECL(float, layoutRightMargin, LayoutRightMargin)
	BASIC_D_ACCESSOR_DECL(float, layoutHorizontalSpacing, LayoutHorizontalSpacing)
	BASIC_D_ACCESSOR_DECL(float, layoutVerticalSpacing, LayoutVerticalSpacing)
	BASIC_D_ACCESSOR_DECL(int, layoutColumnCount, LayoutColumnCount)

	void retransform() override;
	void setZValue(qreal) override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	typedef CartesianPlotLegendPrivate Private;

protected:
	CartesianPlotLegend(const QString& name, CartesianPlotLegendPrivate* dd);
	CartesianPlotLegendPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(CartesianPlotLegend)
	void init();
	void initActions();
	QAction* visibilityAction{nullptr};

private slots:
	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChangedSlot();

signals:
	void labelFontChanged(QFont&);
	void labelColorChanged(QColor&);
	void labelColumnMajorChanged(bool);
	void lineSymbolWidthChanged(float);
	void positionChanged(const CartesianPlotLegend::PositionWrapper&);
	void rotationAngleChanged(qreal);
	void backgroundTypeChanged(PlotArea::BackgroundType);
	void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void backgroundBrushStyleChanged(Qt::BrushStyle);
	void backgroundFirstColorChanged(QColor&);
	void backgroundSecondColorChanged(QColor&);
	void backgroundFileNameChanged(QString&);
	void backgroundOpacityChanged(float);
	void borderPenChanged(QPen&);
	void borderCornerRadiusChanged(float);
	void borderOpacityChanged(float);
	void layoutTopMarginChanged(float);
	void layoutBottomMarginChanged(float);
	void layoutLeftMarginChanged(float);
	void layoutRightMarginChanged(float);
	void layoutVerticalSpacingChanged(float);
	void layoutHorizontalSpacingChanged(float);
	void layoutColumnCountChanged(int);

	void positionChanged(QPointF&);
	void visibilityChanged(bool);
};

#endif
