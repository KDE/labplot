/***************************************************************************
    File                 : CartesianPlotLegendDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2013 Alexander Semke (alexander.semke*web.de)
    							(use @ for *)
    Description          : widget for cartesian legend legend properties
                           
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

#ifndef CARTESIANPLOTLEGENDDOCK_H
#define CARTESIANPLOTLEGENDDOCK_H

#include <QList>
#include "ui_cartesianplotlegenddock.h"
#include "backend/worksheet/plots/PlotArea.h"
class CartesianPlotLegend;

class CartesianPlotLegendDock: public QWidget{
	Q_OBJECT
	
public:
	CartesianPlotLegendDock(QWidget *parent);
	void setLegends(QList<CartesianPlotLegend*>);
	
private:
	Ui::CartesianPlotLegendDock ui;
	QList<CartesianPlotLegend*> m_legendList;
	CartesianPlotLegend* m_legend;
	bool m_initializing;
	
private slots:
	void init();
	void retranslateUi();
  
	//SLOTs for changes triggered in CartesianPlotLegendDock
	//"General"-tab
	void nameChanged();
	void commentChanged();	
	void visibilityChanged(int);

	//"Background"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);
  	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);
	
	//"Layout"-tab
	void layoutTopMarginChanged(double);
	void layoutBottomMarginChanged(double);
	void layoutRightMarginChanged(double);
	void layoutLeftMarginChanged(double);
	void layoutHorizontalSpacingChanged(double);
	void layoutVerticalSpacingChanged(double);
	void layoutColumnCountChanged(int);
	
	//SLOTs for changes triggered in CartesianPlotLegend
	void legendBackgroundTypeChanged(PlotArea::BackgroundType);
	void legendBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void legendBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void legendBackgroundBrushStyleChanged(Qt::BrushStyle);
	void legendBackgroundFirstColorChanged(QColor&);
	void legendBackgroundSecondColorChanged(QColor&);
	void legendBackgroundFileNameChanged(QString&);
	void legendBackgroundOpacityChanged(float);

	void legendBorderPenChanged(QPen&);
	void legendBorderOpacityChanged(float);

	void legendLayoutTopMarginChanged(float);
	void legendLayoutBottomMarginChanged(float);
	void legendLayoutLeftMarginChanged(float);
	void legendLayoutRightMarginChanged(float);
	void legendLayoutVerticalSpacingChanged(float);
	void legendLayoutHorizontalSpacingChanged(float);
	void legendLayoutColumnCountChanged(int);
	
	//save/load
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif
