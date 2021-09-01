/*
    File                 : ImageEditor.h
    Project              : LabPlot
    Description          : Edit Image on the basis of input color attributes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QList>

#include <backend/datapicker/DatapickerImage.h>

class QColor;

class ImageEditor {
public:
	static void discretize(QImage*, QImage*, const DatapickerImage::EditorSettings&, QColor);
	static bool processedPixelIsOn(const QImage&, int, int);
	static QRgb findBackgroundColor(const QImage*);
	static int colorAttributeMax(DatapickerImage::ColorAttributes);
	static void uploadHistogram(int*, QImage*, QColor, DatapickerImage::ColorAttributes);
	static int discretizeValueForeground(int, int, DatapickerImage::ColorAttributes, const QColor, const QImage*);
	static bool pixelIsOn(int, DatapickerImage::ColorAttributes, const DatapickerImage::EditorSettings&);

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
	typedef QVector<ColorEntry> ColorList;
};
#endif // IMAGEEDITOR_H
