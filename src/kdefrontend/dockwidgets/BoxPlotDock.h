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

	void setModel();
	void loadConfig(KConfig&);
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);

private slots:
	//SLOTs for changes triggered in BoxPlotDock

	//"General"-tab
	void dataColumnChanged(const QModelIndex&);
	void visibilityChanged(bool);

	//"Box"-tab
	//box filling
	void fillingEnabledChanged(int);
	void boxFillingTypeChanged(int);
	void boxFillingColorStyleChanged(int);
	void boxFillingImageStyleChanged(int);
	void boxFillingBrushStyleChanged(int);
	void boxFillingFirstColorChanged(const QColor&);
	void boxFillingSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void boxFillingOpacityChanged(int);

	//box border
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);

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

	//whiskers


	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
