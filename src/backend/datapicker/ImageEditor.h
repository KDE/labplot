/***************************************************************************
    File                 : ImageEditor.h
    Project              : LabPlot
    Description          : Edit Image on the basis of input color attributes
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QList>

#include <backend/datapicker/DatapickerImage.h>

class QColor;

class ImageEditor {
public:
	static void discretize(QImage*, QImage*, DatapickerImage::EditorSettings, QColor);
	static bool processedPixelIsOn(const QImage&, int, int);
	static QRgb findBackgroundColor(const QImage*);
	static int colorAttributeMax(DatapickerImage::ColorAttributes);
	static void uploadHistogram(int*, QImage*, QColor, DatapickerImage::ColorAttributes);
	static int discretizeValueForeground(int, int, DatapickerImage::ColorAttributes, const QColor, const QImage*);
	static bool pixelIsOn(int, DatapickerImage::ColorAttributes, DatapickerImage::EditorSettings);

	static int discretizeHue(int, int, const QImage*);
	static int discretizeSaturation(int, int, const QImage*);
	static int discretizeValue(int, int, const QImage*);
	static int discretizeIntensity(int, int, const QImage*);
	static int discretizeForeground(int, int, const QColor, const QImage*);

private:
	static bool colorCompare(QRgb color1, QRgb color2);
	static bool pixelIsOn(int, int, int);

	struct ColorEntry {
		QColor color;
		int count;
	};
	typedef QList<ColorEntry> ColorList;
};
#endif // IMAGEEDITOR_H
