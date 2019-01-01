/***************************************************************************
    File                 : ColorMapRenderer.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : colormap renderer class

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
#include "ColorMapRenderer.h"

#include <KDebug>
#include <QFile>
#include <QPainter>

QPixmap ColorMapRenderer::pixmap(const QString& fileName) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		kDebug()<<"file "<<fileName<<" not found"<<endl;
		return QPixmap();
	}

	QColor rgb;
	QList<QColor> list_rgb;
	int  red, green, blue;
	QTextStream in(&file);

	while (!in.atEnd()) {
        in.readLine();
 		in >> red >> green >> blue;
		rgb.setRgb( red, green, blue );
		list_rgb.append(rgb);
//		kDebug()<<red<<"\t"<<green<<"\t"<<blue<<endl;
	}

	int height = list_rgb.size();
	int width = 80;
// 	kDebug()<<height<<"line read."<<endl;
	QPixmap pixmap(width, height);
	QPainter p( &pixmap );
	for (int i = 0; i != height; ++i) {
		rgb = list_rgb.at(i);
		p.setPen( rgb );
		p.drawLine( QPoint(0, height-i), QPoint(width, height-i) );
	}

	return pixmap;
}
