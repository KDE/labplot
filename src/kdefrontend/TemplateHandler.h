/*
    File                 : TemplateHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of templates
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    SPDX-FileCopyrightText: 2012-2019 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef TEMPLATEHANDLER_H
#define TEMPLATEHANDLER_H

#include <QWidget>

class QMenu;
class QToolButton;
class KConfig;

class TemplateHandler : public QWidget {
	Q_OBJECT

public:
	enum class ClassName {Spreadsheet, Matrix, Worksheet, CartesianPlot, CartesianPlotLegend, Histogram, XYCurve, Axis, CustomPoint};

	TemplateHandler(QWidget* parent, ClassName);
	void setToolButtonStyle(Qt::ToolButtonStyle);

private:
	void retranslateUi();
	bool eventFilter(QObject*, QEvent*) override;

	QString m_dirName;
	ClassName m_className;
	QList<QString> m_subDirNames;

	QMenu* m_textPositionMenu{nullptr};
	QToolButton* m_tbLoad;
	QToolButton* m_tbSave;
	QToolButton* m_tbSaveDefault;
// 	QToolButton* m_tbCopy;
// 	QToolButton* m_tbPaste;

private slots:
	void loadMenu();
	void saveMenu();
	void loadMenuSelected(QAction*);
	void saveMenuSelected(QAction*);
	void saveNewSelected(const QString&);
	void saveDefaults();
	void updateTextPosition(QAction*);

signals:
	void loadConfigRequested(KConfig&);
	void saveConfigRequested(KConfig&);
	void info(const QString&);
};

#endif
