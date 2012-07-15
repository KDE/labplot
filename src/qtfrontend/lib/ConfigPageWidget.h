/***************************************************************************
    File                 : ConfigPageWidget.h
    Project              : SciDAVis
    Description          : Widget for configuration pages that has an apply slot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef CONFIG_PAGE_WIDGET
#define CONFIG_PAGE_WIDGET

#include <QWidget>

//! Widget for configuration pages that has an apply slot
class ConfigPageWidget : public QWidget
{
	Q_OBJECT

	public:
		ConfigPageWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);

	public slots:
		virtual void apply() = 0;
};

#endif // ifndef CONFIG_PAGE_WIDGET

