/***************************************************************************
	File                 : GeneralTestView.h
	Project              : LabPlot
	Description          : View class for Hypothesis Tests'
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 205 Alexander Semke >alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef GENERALTESTVIEW_H
#define GENERALTESTVIEW_H

#include <QLocale>
#include <QWidget>

class Column;
class GeneralTest;
class AbstractAspect;
class QPrinter;

class GeneralTestView : public QWidget {
	Q_OBJECT

public:
	explicit GeneralTestView(GeneralTest*);
	~GeneralTestView() override;

	bool exportDisplay();
	bool executePrintView();
	bool previewPrintView();

private:
	void initializeActions();
	void initializeMenus();
	void setupConnections();

	void exportDataToFile(const QString& path, bool exportHeader, const QString& separator, QLocale::Language language) const;
	void
	exportDataToLaTeX(const QString& path, bool exportHeaders, bool gridLines, bool captions, bool latexHeaders, bool skipEmptyRows, bool exportEntire) const;

	GeneralTest* m_test;

public Q_SLOTS:
	void renderToPrinter(QPrinter*) const;
};

#endif // GENERALTESTVIEW_H
