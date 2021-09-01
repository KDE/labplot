/*
    File                 : ThemeHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Prakriti Bhardwaj (p_bhardwaj14@informatik.uni-kl.de)
    SPDX-FileCopyrightText: 2016 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef THEMEHANDLER_H
#define THEMEHANDLER_H

#include <QWidget>

class QPushButton;
class KConfig;

class ThemeHandler : public QWidget {
	Q_OBJECT

public:
	explicit ThemeHandler(QWidget*);
	static QStringList themeList();
	static QStringList themes();
	static const QString themeFilePath(const QString&);

public  slots:
	void setCurrentTheme(const QString&);

private:
	QList<QString> m_dirNames;
	QStringList m_themeList;
	QString m_currentTheme;
	QString m_currentLocalTheme;

        QPushButton* m_pbLoadTheme;
//      QPushButton* m_pbSaveTheme;
// 	QPushButton* pbPublishTheme;

private slots:
	void loadSelected(const QString&);
	void showPanel();
// 	void saveMenu();
// 	void saveNewSelected(const QString&);
// 	void publishThemes();

signals:
	void loadThemeRequested(const QString&);
	void saveThemeRequested(KConfig&);
	void info(const QString&);
	void loadPreviewPanel(QStringList, QString);
};

#endif
