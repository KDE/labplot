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
class SymbolWidget;
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
	QList<BoxPlot*> m_boxPlots;
	BoxPlot* m_boxPlot{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setModel();
	void loadConfig(KConfig&);
	void setDataColumns() const;
	void loadDataColumns();
	void updateSymbolWidgets();

private slots:
	//SLOTs for changes triggered in BoxPlotDock

	//"General"-tab
	void addDataColumn();
	void removeDataColumn();
	void dataColumnChanged(const QModelIndex&) const;
	void orientationChanged(int) const;
	void variableWidthChanged(bool) const;
	void notchesEnabledChanged(bool) const;
	void visibilityChanged(bool) const;

	//"Box"-tab
	void widthFactorChanged(int) const;

	//box filling
	void fillingEnabledChanged(int) const;
	void fillingTypeChanged(int) const;
	void fillingColorStyleChanged(int) const;
	void fillingImageStyleChanged(int) const;
	void fillingBrushStyleChanged(int) const;
	void fillingFirstColorChanged(const QColor&) const;
	void fillingSecondColorChanged(const QColor&) const;
	void selectFile();
	void fileNameChanged() const;
	void fillingOpacityChanged(int) const;

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
	void symbolCategoryChanged();

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
	void plotDataColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotOrientationChanged(BoxPlot::Orientation);
	void plotVariableWidthChanged(bool);
	void plotWidthFactorChanged(double);
	void plotNotchesEnabledChanged(bool);
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
