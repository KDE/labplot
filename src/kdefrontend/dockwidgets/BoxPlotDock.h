/***************************************************************************
    File                 : BoxPlotDock.h
    Project              : LabPlot
    Description          : Dock widget for the box plot
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

#ifndef BOXPLOTDOCK_H
#define BOXPLOTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "ui_boxplotdock.h"

class AbstractAspect;
class AspectTreeModel;
class BoxPlot;
class TreeViewComboBox;
class KConfig;

class BoxPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit BoxPlotDock(QWidget *);
	void setBoxPlots(QList<BoxPlot*>);
	void updateLocale() override;

private:
	Ui::BoxPlotDock ui;
	TreeViewComboBox* cbDataColumn{nullptr};
	QList<BoxPlot*> m_boxPlots;
	BoxPlot* m_boxPlot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;
	int m_dataLayoutIndex;

	void setModel();
	void loadConfig(KConfig&);
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);
	void updateSymbolWidgets();

private slots:
	//SLOTs for changes triggered in BoxPlotDock

	//"General"-tab
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&) const;
	void visibilityChanged(bool) const;

	//"Box"-tab
	//box filling
	void fillingEnabledChanged(int) const;
	void boxFillingTypeChanged(int) const;
	void boxFillingColorStyleChanged(int) const;
	void boxFillingImageStyleChanged(int) const;
	void boxFillingBrushStyleChanged(int) const;
	void boxFillingFirstColorChanged(const QColor&) const;
	void boxFillingSecondColorChanged(const QColor&) const;
	void selectFile();
	void fileNameChanged() const;
	void boxFillingOpacityChanged(int) const;

	//box border
	void borderStyleChanged(int) const;
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double) const;
	void borderOpacityChanged(int) const;

	//median line
	void medianLineStyleChanged(int) const;
	void medianLineColorChanged(const QColor&);
	void medianLineWidthChanged(double) const;
	void medianLineOpacityChanged(int) const;

	//makers
	void symbolOutliersStyleChanged(int);
	void symbolMeanStyleChanged(int);
	void symbolsSizeChanged(double) const;
	void symbolsRotationChanged(int) const;
	void symbolsOpacityChanged(int) const;
	void symbolsFillingStyleChanged(int) const;
	void symbolsFillingColorChanged(const QColor&);
	void symbolsBorderStyleChanged(int) const;
	void symbolsBorderColorChanged(const QColor&);
	void symbolsBorderWidthChanged(double) const;

	//whiskers
	void whiskersTypeChanged(int) const;
	void whiskersStyleChanged(int) const;
	void whiskersCapSizeChanged(double) const;
	void whiskersColorChanged(const QColor&);
	void whiskersWidthChanged(double) const;
	void whiskersOpacityChanged(int) const;

	//SLOTs for changes triggered in BoxPlot
	//general
	void plotDescriptionChanged(const AbstractAspect*);
	void plotDataColumnChanged(const AbstractColumn*);
	void plotVisibilityChanged(bool);

	//box filling
	void plotFillingEnabledChanged(bool);
	void plotFillingTypeChanged(PlotArea::BackgroundType);
	void plotFillingColorStyleChanged(PlotArea::BackgroundColorStyle);
	void plotFillingImageStyleChanged(PlotArea::BackgroundImageStyle);
	void plotFillingBrushStyleChanged(Qt::BrushStyle);
	void plotFillingFirstColorChanged(QColor&);
	void plotFillingSecondColorChanged(QColor&);
	void plotFillingFileNameChanged(QString&);
	void plotFillingOpacityChanged(double);

	//box border
	void plotBorderPenChanged(QPen&);
	void plotBorderOpacityChanged(float);

	//median line
	void plotMedianLinePenChanged(QPen&);
	void plotMedianLineOpacityChanged(float);

	//markers
	void plotSymbolOutliersStyleChanged(Symbol::Style);
	void plotSymbolMeanStyleChanged(Symbol::Style);
	void plotSymbolsSizeChanged(qreal);
	void plotSymbolsRotationAngleChanged(qreal);
	void plotSymbolsOpacityChanged(qreal);
	void plotSymbolsBrushChanged(const QBrush&);
	void plotSymbolsPenChanged(const QPen&);

	//whiskers
	void plotWhiskersTypeChanged(BoxPlot::WhiskersType);
	void plotWhiskersPenChanged(QPen&);
	void plotWhiskersCapSizeChanged(double);
	void plotWhiskersOpacityChanged(float);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
