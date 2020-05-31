/***************************************************************************
    File                 : TemplateHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of templates
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Copyright            : (C) 2012-2019 by Alexander Semke (alexander.semke@web.de)

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

#ifndef TEMPLATEHANDLER_H
#define TEMPLATEHANDLER_H

#include <QWidget>

class QToolButton;
class KConfig;

class TemplateHandler : public QWidget {
	Q_OBJECT

public:
	enum class ClassName {Spreadsheet, Matrix, Worksheet, CartesianPlot, CartesianPlotLegend, Histogram, XYCurve, Axis, CustomPoint};

	TemplateHandler(QWidget* parent, ClassName);

private:
	void retranslateUi();

	QString m_dirName;
	ClassName m_className;
	QList<QString> m_subDirNames;

	QToolButton* m_tbLoad;
	QToolButton* m_tbSave;
	QToolButton* m_tbSaveDefault;
	QToolButton* m_tbCopy;
	QToolButton* m_tbPaste;

private slots:
	void loadMenu();
	void saveMenu();
	void loadMenuSelected(QAction*);
	void saveMenuSelected(QAction*);
	void saveNewSelected(const QString&);
	void saveDefaults();

signals:
	void loadConfigRequested(KConfig&);
	void saveConfigRequested(KConfig&);
	void info(const QString&);
};

#endif
