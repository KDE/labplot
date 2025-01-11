/*
	File                 : WorksheetDock.h
	Project              : LabPlot
	Description          : widget for worksheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-FileCopyrightText: 2010-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETDOCK_H
#define WORKSHEETDOCK_H

#include "backend/worksheet/Worksheet.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_worksheetdock.h"

class AbstractAspect;
class BackgroundWidget;
class ThemeHandler;
class Worksheet;

class WorksheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit WorksheetDock(QWidget*);
	void setWorksheets(QList<Worksheet*>);
	void updateLocale() override;
	void updateUnits() override;

	enum class SizeType {
		ViewSize = 0,
		StandardPage,
		Custom,
	};

private:
	Ui::WorksheetDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	QList<Worksheet*> m_worksheetList;
	Worksheet* m_worksheet{nullptr};
	ThemeHandler* m_themeHandler;

	void updatePaperSize();

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in WorksheetDock
	//"General"-tab
	void scaleContentChanged(bool);
	void sizeTypeChanged(int);
	void pageChanged(int);
	void sizeChanged();
	void orientationChanged(int);

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

	// SLOTs for changes triggered in Worksheet
	void worksheetDescriptionChanged(const AbstractAspect*);
	void worksheetScaleContentChanged(bool);
	void worksheetUseViewSizeChanged(bool);
	void worksheetPageRectChanged(const QRectF&);

	void worksheetLayoutChanged(Worksheet::Layout);
	void worksheetLayoutTopMarginChanged(double);
	void worksheetLayoutBottomMarginChanged(double);
	void worksheetLayoutLeftMarginChanged(double);
	void worksheetLayoutRightMarginChanged(double);
	void worksheetLayoutVerticalSpacingChanged(double);
	void worksheetLayoutHorizontalSpacingChanged(double);
	void worksheetLayoutRowCountChanged(int);
	void worksheetLayoutColumnCountChanged(int);

	// save/load templates and themes
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);
	void loadTheme(const QString&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // WORKSHEETDOCK_H
