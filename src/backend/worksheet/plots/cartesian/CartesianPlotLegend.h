/*
    File                 : CartesianPlotLegend.h
    Project              : LabPlot
    Description          : Legend for the cartesian plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2013-2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTLEGEND_H
#define CARTESIANPLOTLEGEND_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/lib/macros.h"

class CartesianPlotLegendPrivate;
class TextLabel;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CartesianPlotLegend : public WorksheetElement {
#else
class CartesianPlotLegend : public WorksheetElement {
#endif
	Q_OBJECT
	Q_ENUMS(HorizontalPosition)
	Q_ENUMS(VerticalPosition)

public:
	explicit CartesianPlotLegend(const QString &name);
	~CartesianPlotLegend() override;

	void finalizeAdd() override;
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig& config) override;

	TextLabel* title();

	CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont)
	CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor)
	BASIC_D_ACCESSOR_DECL(bool, labelColumnMajor, LabelColumnMajor)
	BASIC_D_ACCESSOR_DECL(float, lineSymbolWidth, LineSymbolWidth)

	BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
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

private:
	Q_DECLARE_PRIVATE(CartesianPlotLegend)
	void init();
	void initActions();
	QAction* visibilityAction{nullptr};

private Q_SLOTS:
	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChangedSlot();

Q_SIGNALS:
	void labelFontChanged(QFont&);
	void labelColorChanged(QColor&);
	void labelColumnMajorChanged(bool);
	void lineSymbolWidthChanged(float);
	void backgroundTypeChanged(WorksheetElement::BackgroundType);
	void backgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void backgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle);
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
};

#endif
