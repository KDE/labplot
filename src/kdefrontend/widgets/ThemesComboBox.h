/***************************************************************************
    File                 : ThemesComboBox.h
    Project              : LabPlot
    Description          : Preview of all themes in a QComboBox
    --------------------------------------------------------------------
    Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)

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

#ifndef THEMESCOMBOBOX_H
#define THEMESCOMBOBOX_H

#include <QComboBox>

class QGroupBox;
class ThemesWidget;

class ThemesComboBox : public QComboBox {
	Q_OBJECT

	public:
		explicit ThemesComboBox(QWidget* parent = 0);
		void setCurrentTheme(const QString&);

		void showPopup() override;
		void hidePopup() override;

	private:
		QGroupBox* m_groupBox;
		ThemesWidget* m_view;
		bool eventFilter(QObject*, QEvent*) override;

	private slots:
		void handleThemeChanged(const QString&);

	signals:
		void currentThemeChanged(const QString&);
};

#endif
