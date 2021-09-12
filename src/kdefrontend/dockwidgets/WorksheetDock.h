/*
    File                 : WorksheetDock.h
    Project              : LabPlot
    Description          : widget for worksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
    SPDX-FileCopyrightText: 2010-2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETDOCK_H
#define WORKSHEETDOCK_H

#include "backend/worksheet/Worksheet.h"
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

	void worksheetBackgroundTypeChanged(WorksheetElement::BackgroundType);
	void worksheetBackgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void worksheetBackgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle);
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
