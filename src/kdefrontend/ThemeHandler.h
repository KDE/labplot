/***************************************************************************
    File                 : ThemeHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Prakriti Bhardwaj (p_bhardwaj14@informatik.uni-kl.de)
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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

#ifndef THEMEHANDLER_H
#define THEMEHANDLER_H

#include <QWidget>

class QPushButton;
class KConfig;

class ThemeHandler : public QWidget {
	Q_OBJECT

	public:
		explicit ThemeHandler(QWidget*);
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
// 		QPushButton* pbPublishTheme;

	private slots:
		void loadSelected(const QString&);
		void showPanel();
// 		void saveMenu();
// 		void saveNewSelected(const QString&);
// 		void publishThemes();

	signals:
		void loadThemeRequested(const QString&);
		void saveThemeRequested(KConfig&);
		void info(const QString&);
		void loadPreviewPanel(QStringList, QString);
};

#endif
