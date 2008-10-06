/***************************************************************************
    File                 : ColorButton.cpp
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
#include "ColorButton.h"

#include <QPalette>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

ColorButton::ColorButton(QWidget *parent) : QWidget(parent)
{
	init();
}

void ColorButton::init()
{
	int btn_size = 28;
	selectButton = new QPushButton(QPixmap(":/palette.xpm"), QString(), this);
	selectButton->setMinimumWidth(btn_size);
	selectButton->setMinimumHeight(btn_size);

	display = new QFrame(this);
	display->setLineWidth(2);
	display->setFrameStyle (QFrame::Panel | QFrame::Sunken);
	display->setMinimumHeight(btn_size);
	display->setMinimumWidth(2*btn_size);
	display->setAutoFillBackground(true);
	setColor(QColor(Qt::white));

	QHBoxLayout *l = new QHBoxLayout(this);
	l->setMargin( 0 );
	l->addWidget( display );
	l->addWidget( selectButton );

	setMaximumWidth(3*btn_size);
	setMaximumHeight(btn_size);

	connect(selectButton, SIGNAL(clicked()), this, SIGNAL(clicked()));
}

void ColorButton::setColor(const QColor& c)
{
	QPalette pal;
	pal.setColor(QPalette::Window, c);
	display->setPalette(pal);
}

QColor ColorButton::color() const
{
	return display->palette().color(QPalette::Window);
}

QSize ColorButton::sizeHint () const
{
	return QSize(4*btn_size, btn_size);
}
