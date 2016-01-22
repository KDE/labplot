/***************************************************************************
    File                 : ImageEditor.cpp
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

#include "ImageEditor.h"
#include "math.h"
#include <QElapsedTimer>
#include <QDebug>

/*!
 *
 */
void ImageEditor::discretize(QImage* plotImage, QImage* originalImage,
                             DatapickerImage::EditorSettings settings, QColor background) {
//    QElapsedTimer timer;
//    timer.start();
	for (int x = 0; x < plotImage->width(); x++) {
		for (int y = 0; y < plotImage->height(); y++) {
			bool on = true;
			int value;
			DatapickerImage::ColorAttributes type;
			for (int i = DatapickerImage::Intensity; i <= DatapickerImage::Value; i++) {
				type = (DatapickerImage::ColorAttributes) i;
				value = discretizeValueForeground(x, y, type, background, originalImage);

				if (!pixelIsOn(value, type, settings))
					on = false;
			}

			if (on)
				plotImage->setPixel(x, y, QColor(Qt::black).rgb());
			else
				plotImage->setPixel(x, y, QColor(Qt::white).rgb());
		}
	}
//    qDebug() << "Pixmap updated in " << timer.elapsed() << "ms";
}

bool ImageEditor::processedPixelIsOn(const QImage& plotImage, int x, int y) {
	if ((x < 0) || (plotImage.width() <= x) || (y < 0) || (plotImage.height() <= y))
		return false;

	// pixel is on if it is closer to black than white in gray scale. this test must be performed
	// on little endian and big endian systems, with or without alpha bits (which are typically high bits)
	const int BLACK_WHITE_THRESHOLD = 255 / 2; // put threshold in middle of range
	int gray = qGray(plotImage.pixel(x, y));
	return (gray < BLACK_WHITE_THRESHOLD);
}

//##############################################################################
//#####################  private helper functions  #############################
//##############################################################################
QRgb ImageEditor::findBackgroundColor(const QImage* plotImage) {
	QList<ColorEntry>::iterator itrC;
	ColorList colors;
	int x, y = 0;
	for (x = 0; x < plotImage->width(); x++) {
		ColorEntry c;
		c.color = plotImage->pixel(x,y);
		c.count = 0;

		bool found = false;
		for (itrC = colors.begin(); itrC != colors.end(); ++itrC) {
			if (colorCompare(c.color.rgb(), (*itrC).color.rgb())) {
				found = true;
				++(*itrC).count;
				break;
			}
		}
		if (!found)
			colors.append(c);

		if (++y >= plotImage->height())
			y = 0;
	}

	ColorEntry cMax;
	cMax.count = 0;
	for (itrC = colors.begin(); itrC != colors.end(); ++itrC) {
		if ((*itrC).count > cMax.count)
			cMax = (*itrC);
	}

	return cMax.color.rgb();
}

void ImageEditor::uploadHistogram(int* bins, QImage* originalImage, QColor background, DatapickerImage::ColorAttributes type) {
	//reset bin
	for (int i = 0; i <= colorAttributeMax(type); i++)
		bins [i] = 0;

	for (int x = 0; x < originalImage->width(); x++) {
		for (int y = 0; y < originalImage->height(); y++) {
			int value = discretizeValueForeground(x, y, type, background, originalImage);
			bins[value] += 1;
		}
	}
}

int ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes type) {
	//Intensity, Foreground, Saturation and Value are from 0 to 100
	//Hue is from 0 to 360
	switch (type) {
	case DatapickerImage::None:
		return 0;
	case DatapickerImage::Intensity:
		return 100;
	case DatapickerImage::Foreground:
		return 100;
	case DatapickerImage::Hue:
		return 360;
	case DatapickerImage::Saturation:
		return 100;
	case DatapickerImage::Value:
	default:
		return 100;
	}
}

bool ImageEditor::colorCompare(QRgb color1, QRgb color2) {
	const long MASK = 0xf0f0f0f0;
	return (color1 & MASK) == (color2 & MASK);
}

int ImageEditor::discretizeValueForeground(int x, int y, DatapickerImage::ColorAttributes type,
        QColor background, QImage* originalImage) {
	const QColor color(originalImage->pixel(x,y));
	const int attributeMax = colorAttributeMax(type);

	// convert hue from 0 to 359, saturation from 0 to 255, value from 0 to 255
	int value = 0;
	switch (type) {
	case DatapickerImage::None:
		break;
	case DatapickerImage::Intensity: {
		const int r = color.red();
		const int g = color.green();
		const int b = color.blue();
		const double intensity = sqrt ((double) (r * r + g * g + b * b));
		value = (int) (intensity * attributeMax / sqrt((double) (255 * 255 + 255 * 255 + 255 * 255)) + 0.5);
		break;
	}
	case DatapickerImage::Foreground: {
		const int r = color.red();
		const int g = color.green();
		const int b = color.blue();
		const int rBg = background.red();
		const int gBg = background.green();
		const int bBg = background.blue();
		const double distance = sqrt ((double) ((r - rBg) * (r - rBg) + (g - gBg) * (g - gBg) + (b - bBg) * (b - bBg)));
		value = (int) (distance * attributeMax / sqrt((double) (255 * 255 + 255 * 255 + 255 * 255)) + 0.5);
		break;
	}
	case DatapickerImage::Hue: {
		const int h = color.hue();
		value = h * attributeMax / 359;
		break;
	}
	case DatapickerImage::Saturation: {
		const int s = color.saturation();
		value = s * attributeMax / 255;
		break;
	}
	case DatapickerImage::Value: {
		const int v = color.value();
		value = v * attributeMax / 255;
		break;
	}
	}

	if (value < 0)
		value = 0;
	if (attributeMax < value)
		value = attributeMax;

	return value;
}

bool ImageEditor::pixelIsOn(int value, int low, int high) {
	if (low < high)
		return ((low <= value) && (value <= high));
	else
		return ((low <= value) || (value <= high));
}

bool ImageEditor::pixelIsOn( int value, DatapickerImage::ColorAttributes type,DatapickerImage::EditorSettings settings ) {
	switch (type) {
	case DatapickerImage::None:
		break;
	case DatapickerImage::Intensity:
		return pixelIsOn(value, settings.intensityThresholdLow, settings.intensityThresholdHigh);
	case DatapickerImage::Foreground:
		return pixelIsOn(value, settings.foregroundThresholdLow, settings.foregroundThresholdHigh);
	case DatapickerImage::Hue:
		return pixelIsOn(value, settings.hueThresholdLow, settings.hueThresholdHigh);
	case DatapickerImage::Saturation:
		return pixelIsOn(value, settings.saturationThresholdLow, settings.saturationThresholdHigh);
	case DatapickerImage::Value:
		return pixelIsOn(value, settings.valueThresholdLow, settings.valueThresholdHigh);
	}

	return false;
}
