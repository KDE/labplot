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

#include <QColor>

class QString;
class QImage;
class QTemporaryFile;

class TeXRenderer {
public:
	struct Formatting {
		QColor fontColor;
		QColor backgroundColor;
		int fontSize;
		QString fontFamily;
		int dpi;
	};

	static QImage renderImageLaTeX(const QString&, bool* success, const TeXRenderer::Formatting&);
	static QImage imageFromPDF(const QTemporaryFile&, const int dpi, const QString& engine, bool* success);
	static QImage imageFromDVI(const QTemporaryFile&, const int dpi, bool* success);
	static bool enabled();
	static bool executableExists(const QString&);
};

#endif
