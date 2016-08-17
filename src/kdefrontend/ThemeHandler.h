/***************************************************************************
    File                 : ThemeHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
	Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
	Copyright            : (C) 2012-2013 by Alexander Semke (alexander.semke@web.de)

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

#include <QtGui/QWidget>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
class QHBoxLayout;
class QToolButton;
class QSpacerItem;
class QLabel;
class KConfig;

class ThemeHandler : public QWidget{
	Q_OBJECT

	public:
		ThemeHandler(QWidget* parent);
		static QStringList themes();
		static const QString themeConfigPath(const QString&);

	private:
		QList<QString> dirNames;
		QStringList m_themeList;
		QString m_themeImgPath;

		QPushButton* pbLoadTheme;
		QPushButton* pbSaveTheme;

	private slots:
		void loadSelected(QString str);
		void showPanel();
		void saveMenu();
		void saveNewSelected(const QString&);
		void saveDefaults();

	public slots:
		void saveThemeEnable(bool enable);

	signals:
		void loadThemeRequested(KConfig& config);
		void saveThemeRequested(KConfig& config);
		void info(const QString&);
		void loadPreviewPanel(QStringList,QString);
};

#endif
