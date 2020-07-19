/***************************************************************************
    File                 : WorksheetDock.h
    Project              : LabPlot
    Description          : widget for worksheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2010-2020 by Alexander Semke (alexander.semke@web.de)

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

#ifndef WORKSHEETDOCK_H
#define WORKSHEETDOCK_H

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_worksheetdock.h"

class AbstractAspect;
class ThemeHandler;
class Worksheet;

class WorksheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit WorksheetDock(QWidget*);
	void setWorksheets(QList<Worksheet*>);
	void updateLocale() override;
	void updateUnits() override;

private:
	Ui::WorksheetDock ui;
	QList<Worksheet*> m_worksheetList;
	Worksheet* m_worksheet{nullptr};
	ThemeHandler* m_themeHandler;

	void updatePaperSize();

	void load();
	void loadConfig(KConfig&);

private slots:
	void retranslateUi();

	//SLOTs for changes triggered in WorksheetDock
	//"General"-tab
	void scaleContentChanged(bool);
	void sizeChanged(int);
	void sizeChanged();
	void orientationChanged(int);

	//"Background"-tab
	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void backgroundOpacityChanged(int);
	void selectFile();
	void fileNameChanged();

	//"Layout"-tab
	void layoutChanged(int);
	void layoutTopMarginChanged(double);
	void layoutBottomMarginChanged(double);
	void layoutRightMarginChanged(double);
	void layoutLeftMarginChanged(double);
	void layoutHorizontalSpacingChanged(double);
	void layoutVerticalSpacingChanged(double);
	void layoutRowCountChanged(int);
	void layoutColumnCountChanged(int);

	//SLOTs for changes triggered in Worksheet
	void worksheetDescriptionChanged(const AbstractAspect*);
	void worksheetScaleContentChanged(bool);
	void worksheetPageRectChanged(const QRectF&);

	void worksheetBackgroundTypeChanged(PlotArea::BackgroundType);
	void worksheetBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void worksheetBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void worksheetBackgroundBrushStyleChanged(Qt::BrushStyle);
	void worksheetBackgroundFirstColorChanged(const QColor&);
	void worksheetBackgroundSecondColorChanged(const QColor&);
	void worksheetBackgroundFileNameChanged(const QString&);
	void worksheetBackgroundOpacityChanged(float);
	void worksheetLayoutChanged(Worksheet::Layout);
	void worksheetLayoutTopMarginChanged(float);
	void worksheetLayoutBottomMarginChanged(float);
	void worksheetLayoutLeftMarginChanged(float);
	void worksheetLayoutRightMarginChanged(float);
	void worksheetLayoutVerticalSpacingChanged(float);
	void worksheetLayoutHorizontalSpacingChanged(float);
	void worksheetLayoutRowCountChanged(int);
	void worksheetLayoutColumnCountChanged(int);

	//save/load templates and themes
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);
	void loadTheme(const QString&);

signals:
	void info(const QString&);
};

#endif // WORKSHEETDOCK_H
