/*
	File                 : CartesianPlotLegend.h
	Project              : LabPlot
	Description          : Legend for the cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTLEGEND_H
#define CARTESIANPLOTLEGEND_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class Background;
class CartesianPlotLegendPrivate;
class Line;
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
	explicit CartesianPlotLegend(const QString& name);
	~CartesianPlotLegend() override;

	void finalizeAdd() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig& config) override;

	TextLabel* title();

	CLASS_D_ACCESSOR_DECL(QFont, labelFont, LabelFont)
	CLASS_D_ACCESSOR_DECL(QColor, labelColor, LabelColor)
	BASIC_D_ACCESSOR_DECL(bool, labelColumnMajor, LabelColumnMajor)
	BASIC_D_ACCESSOR_DECL(qreal, lineSymbolWidth, LineSymbolWidth)

	Background* background() const;

	Line* borderLine() const;
	BASIC_D_ACCESSOR_DECL(qreal, borderCornerRadius, BorderCornerRadius)

	BASIC_D_ACCESSOR_DECL(qreal, layoutTopMargin, LayoutTopMargin)
	BASIC_D_ACCESSOR_DECL(qreal, layoutBottomMargin, LayoutBottomMargin)
	BASIC_D_ACCESSOR_DECL(qreal, layoutLeftMargin, LayoutLeftMargin)
	BASIC_D_ACCESSOR_DECL(qreal, layoutRightMargin, LayoutRightMargin)
	BASIC_D_ACCESSOR_DECL(qreal, layoutHorizontalSpacing, LayoutHorizontalSpacing)
	BASIC_D_ACCESSOR_DECL(qreal, layoutVerticalSpacing, LayoutVerticalSpacing)
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

Q_SIGNALS:
	void labelFontChanged(QFont&);
	void labelColorChanged(QColor&);
	void labelColumnMajorChanged(bool);
	void lineSymbolWidthChanged(float);
	void borderCornerRadiusChanged(float);
	void layoutTopMarginChanged(float);
	void layoutBottomMarginChanged(float);
	void layoutLeftMarginChanged(float);
	void layoutRightMarginChanged(float);
	void layoutVerticalSpacingChanged(float);
	void layoutHorizontalSpacingChanged(float);
	void layoutColumnCountChanged(int);
};

#endif
