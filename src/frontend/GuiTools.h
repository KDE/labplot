/*
	File                 : GuiTools.h
	Project              : LabPlot
	Description          :  contains several static functions which are used on frequently throughout the kde frontend.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011 Alexander Semke <alexander.semke*web.de  (use @ for *)>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QPen>

class QComboBox;
class QColor;
class QLineEdit;
class QMenu;
class QActionGroup;
class QAction;

class GuiTools {
public:
	static bool isDarkMode();

	static void updateBrushStyles(QComboBox*, const QColor&);
	static void updatePenStyles(QComboBox*, const QColor&);
	static void updatePenStyles(QMenu*, QActionGroup*, const QColor&);
	static void selectPenStyleAction(QActionGroup*, Qt::PenStyle);
	static Qt::PenStyle penStyleFromAction(QActionGroup*, QAction*);
	static void addSymbolStyles(QComboBox*);

	static void fillColorMenu(QMenu*, QActionGroup*);
	static void selectColorAction(QActionGroup*, const QColor&);
	static QColor& colorFromAction(QActionGroup*, QAction*);

	static QPair<float, float> dpi(const QWidget*);

	static void highlight(QWidget*, bool);

	static QString openImageFile(const QString&);
	static QImage importPDFFile(const QString&);
	static QImage imageFromPDFData(const QByteArray&, double zoomFactor = 1.);

	static QString replaceExtension(const QString& fileName, const QString& extension);
};

#endif // GUITOOLS_H
