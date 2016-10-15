/***************************************************************************
    File                 : TeXRenderer.h
    Project              : LabPlot
    Description          : TeX renderer class
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 by Alexander Semke (alexander.semke@web.de)

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
#ifndef TEXRENDERER_H
#define TEXRENDERER_H

class QColor;
class QImage;
class QString;
class QTemporaryFile;

class TeXRenderer {

public:
	static QImage renderImageLaTeX( const QString&, const QColor& fontColor, const int fontSize=12,  const int dpi=300);
	static QImage imageFromPDF(const QTemporaryFile&, const int dpi, const QString& engine);
	static QImage imageFromDVI(const QTemporaryFile&, const int dpi);
};

#endif
