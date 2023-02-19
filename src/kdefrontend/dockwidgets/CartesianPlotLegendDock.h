/*
	File                 : CartesianPlotLegendDock.h
	Project              : LabPlot
	Description          : widget for cartesian legend properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTLEGENDDOCK_H
#define CARTESIANPLOTLEGENDDOCK_H

#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_cartesianplotlegenddock.h"

class BackgroundWidget;
class LabelWidget;
class LineWidget;
class KConfig;

class CartesianPlotLegendDock : public BaseDock {
	Q_OBJECT

public:
	explicit CartesianPlotLegendDock(QWidget*);

	void setLegends(QList<CartesianPlotLegend*>);
	void activateTitleTab() const;
	void updateLocale() override;
	void updateUnits() override;

private:
	Ui::CartesianPlotLegendDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* borderLineWidget{nullptr};
	LabelWidget* labelWidget{nullptr};
	QList<CartesianPlotLegend*> m_legendList;
	CartesianPlotLegend* m_legend{nullptr};

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void init();
	void retranslateUi();

	// SLOTs for changes triggered in CartesianPlotLegendDock
	//"General"-tab
	void visibilityChanged(bool);
	void labelFontChanged(const QFont&);
	void labelColorChanged(const QColor&);
	void labelOrderChanged(int);
	void lineSymbolWidthChanged(Common::ExpressionValue);
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(Common::ExpressionValue);
	void customPositionYChanged(Common::ExpressionValue);
	void horizontalAlignmentChanged(int index);
	void verticalAlignmentChanged(int index);
	void rotationChanged(int value);
	void bindingChanged(bool checked);

	// "Background"-tab
	void borderCornerRadiusChanged(Common::ExpressionValue);

	//"Layout"-tab
	void layoutTopMarginChanged(Common::ExpressionValue);
	void layoutBottomMarginChanged(Common::ExpressionValue);
	void layoutRightMarginChanged(Common::ExpressionValue);
	void layoutLeftMarginChanged(Common::ExpressionValue);
	void layoutHorizontalSpacingChanged(Common::ExpressionValue);
	void layoutVerticalSpacingChanged(Common::ExpressionValue);
	void layoutColumnCountChanged(int);

	// SLOTs for changes triggered in CartesianPlotLegend
	void legendLabelFontChanged(QFont&);
	void legendLabelColorChanged(QColor&);
	void legendLabelOrderChanged(bool);
	void legendLineSymbolWidthChanged(float);
	void legendPositionChanged(const CartesianPlotLegend::PositionWrapper&);
	void legendRotationAngleChanged(qreal);
	void legendVisibilityChanged(bool);
	void legendPositionLogicalChanged(QPointF);
	void legendHorizontalAlignmentChanged(const WorksheetElement::HorizontalAlignment);
	void legendVerticalAlignmentChanged(const WorksheetElement::VerticalAlignment);

	void legendBorderCornerRadiusChanged(float);

	void legendLayoutTopMarginChanged(float);
	void legendLayoutBottomMarginChanged(float);
	void legendLayoutLeftMarginChanged(float);
	void legendLayoutRightMarginChanged(float);
	void legendLayoutVerticalSpacingChanged(float);
	void legendLayoutHorizontalSpacingChanged(float);
	void legendLayoutColumnCountChanged(int);

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
