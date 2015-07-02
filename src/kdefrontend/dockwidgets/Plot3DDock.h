/***************************************************************************
    File                 : Plot3DDock.h
    Project              : LabPlot
    Description          : widget for 3D plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#ifndef PLOT3DDOCK_H
#define PLOT3DDOCK_H

#include "backend/worksheet/plots/PlotArea.h"
#include "ui_plot3ddock.h"

class Plot3D;
class KUrl;
class AbstractColumn;
class Matrix;

class Plot3DDock: public QWidget {
	Q_OBJECT

	public:
		explicit Plot3DDock(QWidget* parent);
		void setPlots(const QList<Plot3D*>& plots);

	private:
		Ui::Plot3DDock ui;
		Plot3D* m_plot;
		QList<Plot3D*> m_plotsList;
		bool m_initializing;

		void hideDataSource(bool hide = true);
		void hideFileUrl(bool hide = true);
		void hideTriangleInfo(bool hide = true);
		AbstractColumn* getColumn(const QModelIndex& index) const;
		Matrix* getMatrix(const QModelIndex& index) const;

		void load();
		void loadConfig(KConfig&);

	private slots:
		//SLOTs for changes triggered in Plot3DDock
		void retranslateUi();

		//"General"-tab
		void nameChanged();
		void commentChanged();
		void onVisualizationTypeChanged(int index);
		void onDataSourceChanged(int index);
		void onFileChanged(const KUrl& path);
		void onTreeViewIndexChanged(const QModelIndex& index);
		void onShowAxes(bool show);

		//"Background"-tab
		void backgroundTypeChanged(int);
		void backgroundColorStyleChanged(int);
		void backgroundImageStyleChanged(int);
		void backgroundBrushStyleChanged(int);
		void backgroundFirstColorChanged(const QColor&);
		void backgroundSecondColorChanged(const QColor&);
		void backgroundOpacityChanged(int);
		void backgroundSelectFile();
		void backgroundFileNameChanged();

		//SLOTs for changes triggered in Plot3D
		//"General"-tab
		void plotDescriptionChanged(const AbstractAspect*);

		//"Background"-tab
		void plotBackgroundTypeChanged(PlotArea::BackgroundType);
		void plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void plotBackgroundBrushStyleChanged(Qt::BrushStyle);
		void plotBackgroundFirstColorChanged(const QColor&);
		void plotBackgroundSecondColorChanged(const QColor&);
		void plotBackgroundFileNameChanged(const QString&);
		void plotBackgroundOpacityChanged(float);

		//saving/loading
		void loadConfigFromTemplate(KConfig&);
		void saveConfigAsTemplate(KConfig&);

		// Axes tab
		void onAxesTypeChanged(int type);
		void onAxesLabelFontChanged(int size);
		void onAxesLabelColorChanged(const QColor& color);
};

#endif
