/***************************************************************************
    File                 : ColorButton.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : A button used for color selection
                           
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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QWidget>
class QPushButton;
class QHBoxLayout;
class QFrame;

//! A button used for color selection
/**
 * This button contains two widgets:
 * 1) A frame reflecting the current color
 * 2) A button showing a color wheel to select the color
 */
class ColorButton : public QWidget
{
	Q_OBJECT

public:
	//! Constructor
	ColorButton(QWidget *parent = 0);
	//! Set the color of the display part
	void setColor(const QColor& c);
	//! Get the color of the display part
	QColor color() const;
	QSize sizeHint() const;

private:
	QPushButton *selectButton;
	QFrame *display;

signals:
	//! Signal clicked: This is emitted when the selection button is clicked
	void clicked();

protected:
	//! Initialize the widget (called from constructor)
	void init();

private:
	int btn_size;
};

#endif
