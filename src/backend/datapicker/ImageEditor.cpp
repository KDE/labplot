/*
    File                 : ImageEditor.cpp
    Project              : LabPlot
    Description          : Edit Image on the basis of input color attributes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015-2016 Alexander Semke (alexander.semke@web.de)
*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ImageEditor.h"
#include <QThreadPool>
#include <QElapsedTimer>
#include <QMutex>

extern "C" {
#include <gsl/gsl_math.h>
}

static const QRgb white = QColor(Qt::white).rgb();
static const QRgb black = QColor(Qt::black).rgb();
static const double colorScale = gsl_hypot3(255, 255, 255);

//Intensity, Foreground, Saturation and Value are from 0 to 100, Hue is from 0 to 360
static const int maxIntensity = 100;
static const int maxForeground = 100;
static const int maxHue = 360;
static const int maxSaturation = 100;
static const int maxValue = 100;

QMutex mutex;

class DiscretizeTask : public QRunnable {
public:
	DiscretizeTask(int start, int end, QImage* plotImage, QImage* originalImage, const DatapickerImage::EditorSettings& settings, QColor background) :
		m_start(start),
		m_end(end),
		m_plotImage(plotImage),
		m_originalImage(originalImage),
		m_settings(settings),
		m_background(std::move(background))
		{};

	void run() override {
		for (int y = m_start; y < m_end; ++y) {
			mutex.lock();
			QRgb* line = reinterpret_cast<QRgb*>(m_plotImage->scanLine(y));
			mutex.unlock();
			for (int x = 0; x < m_plotImage->width(); ++x) {
				int value = ImageEditor::discretizeHue(x, y, m_originalImage);
				if (!ImageEditor::pixelIsOn(value, DatapickerImage::ColorAttributes::Hue, m_settings))
					continue;

				value = ImageEditor::discretizeSaturation(x, y, m_originalImage);
				if (!ImageEditor::pixelIsOn(value, DatapickerImage::ColorAttributes::Saturation, m_settings))
					continue;

				value = ImageEditor::discretizeValue(x, y, m_originalImage);
				if (!ImageEditor::pixelIsOn(value, DatapickerImage::ColorAttributes::Value, m_settings))
					continue;

				value = ImageEditor::discretizeIntensity(x, y, m_originalImage);
				if (!ImageEditor::pixelIsOn(value, DatapickerImage::ColorAttributes::Saturation, m_settings))
					continue;

				value = ImageEditor::discretizeForeground(x, y, m_background, m_originalImage);
				if (!ImageEditor::pixelIsOn(value, DatapickerImage::ColorAttributes::Foreground, m_settings))
					continue;

				line[x] = black;
			}
		}
	}

private:
	int m_start;
	int m_end;
	QImage* m_plotImage;
	QImage* m_originalImage;
	DatapickerImage::EditorSettings m_settings;
	QColor m_background;
};

/*!
 *
 */
void ImageEditor::discretize(QImage* plotImage, QImage* originalImage,
                             const DatapickerImage::EditorSettings& settings, QColor background) {
	plotImage->fill(white);
	QThreadPool* pool = QThreadPool::globalInstance();
	int range = ceil(double(plotImage->height())/pool->maxThreadCount());
	for (int i = 0; i < pool->maxThreadCount(); ++i) {
		const int start = i*range;
		int end = (i+1)*range;
		if (end > plotImage->height()) end = plotImage->height();
		auto* task = new DiscretizeTask(start, end, plotImage, originalImage, settings, background);
		pool->start(task);
	}
	pool->waitForDone();
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
	ColorList::iterator itrC;
	ColorList colors;
	int x, y = 0;
	for (x = 0; x < plotImage->width(); ++x) {
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
	for (int i = 0; i <= colorAttributeMax(type); ++i)
		bins [i] = 0;

	for (int x = 0; x < originalImage->width(); ++x) {
		for (int y = 0; y < originalImage->height(); ++y) {
			int value = discretizeValueForeground(x, y, type, background, originalImage);
			bins[value] += 1;
		}
	}
}

int ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes type) {
	//Intensity, Foreground, Saturation and Value are from 0 to 100
	//Hue is from 0 to 360
	switch (type) {
	case DatapickerImage::ColorAttributes::None:
		return 0;
	case DatapickerImage::ColorAttributes::Intensity:
		return 100;
	case DatapickerImage::ColorAttributes::Foreground:
		return 100;
	case DatapickerImage::ColorAttributes::Hue:
		return 360;
	case DatapickerImage::ColorAttributes::Saturation:
		return 100;
	case DatapickerImage::ColorAttributes::Value:
	default:
		return 100;
	}
}

bool ImageEditor::colorCompare(QRgb color1, QRgb color2) {
	const long MASK = 0xf0f0f0f0;
	return (color1 & MASK) == (color2 & MASK);
}

int ImageEditor::discretizeHue(int x, int y, const QImage* originalImage) {
	const QColor color(originalImage->pixel(x,y));
	const int h = color.hue();
	int value = h * maxHue / 359;

	if (value < 0) //QColor::hue() can return -1
		value = 0;
	if (maxHue < value)
		value = maxHue;

	return value;
}

int ImageEditor::discretizeSaturation(int x, int y, const QImage* originalImage) {
	const QColor color(originalImage->pixel(x,y));
	const int s = color.saturation();
	int value = s * maxSaturation / 255;

	if (maxSaturation < value)
		value = maxSaturation;

	return value;
}

int ImageEditor::discretizeValue(int x, int y, const QImage* originalImage) {
	const QColor color(originalImage->pixel(x,y));
	const int v = color.value();
	int value = v * maxValue / 255;

	if (maxValue < value)
		value = maxValue;

	return value;
}

int ImageEditor::discretizeIntensity(int x, int y, const QImage* originalImage) {
	const QRgb color = originalImage->pixel(x,y);
	const int r = qRed(color);
	const int g = qGreen(color);
	const int b = qBlue(color);

	const double intensity = gsl_hypot3(r, g, b);
	int value = (int) (intensity * maxIntensity / colorScale + 0.5);

	if (maxIntensity < value)
		value = maxIntensity;

	return value;
}

int ImageEditor::discretizeForeground(int x, int y, const QColor background, const QImage* originalImage) {
	const QRgb color = originalImage->pixel(x,y);
	const int r = qRed(color);
	const int g = qGreen(color);
	const int b = qBlue(color);
	const int rBg = background.red();
	const int gBg = background.green();
	const int bBg = background.blue();
	const double distance = gsl_hypot3(r - rBg, g - gBg, b - bBg);
	int value = (int) (distance * maxForeground / colorScale + 0.5);

	if (maxForeground < value)
		value = maxForeground;

	return value;
}

int ImageEditor::discretizeValueForeground(int x, int y, DatapickerImage::ColorAttributes type,
        const QColor background, const QImage* originalImage) {
	const QColor color(originalImage->pixel(x,y));

	// convert hue from 0 to 359, saturation from 0 to 255, value from 0 to 255
	int value = 0;
	switch (type) {
	case DatapickerImage::ColorAttributes::None:
		break;
	case DatapickerImage::ColorAttributes::Intensity: {
		const int r = color.red();
		const int g = color.green();
		const int b = color.blue();
		const double intensity = gsl_hypot3(r, g, b);
		value = (int) (intensity * maxIntensity / colorScale + 0.5);
		if (maxIntensity < value)
			value = maxIntensity;
		break;
	}
	case DatapickerImage::ColorAttributes::Foreground: {
		const int r = color.red();
		const int g = color.green();
		const int b = color.blue();
		const int rBg = background.red();
		const int gBg = background.green();
		const int bBg = background.blue();
		const double distance = gsl_hypot3(r - rBg, g - gBg, b - bBg);
		value = (int) (distance * maxForeground / colorScale + 0.5);
		if (maxForeground < value)
			value = maxForeground;
		break;
	}
	case DatapickerImage::ColorAttributes::Hue: {
		const int h = color.hue();
		value = h * maxHue / 359;
		if (value < 0)
			value = 0;
		if (maxHue < value)
			value = maxHue;
		break;
	}
	case DatapickerImage::ColorAttributes::Saturation: {
		const int s = color.saturation();
		value = s * maxSaturation / 255;
		if (maxSaturation < value)
			value = maxSaturation;
		break;
	}
	case DatapickerImage::ColorAttributes::Value: {
		const int v = color.value();
		value = v * maxValue / 255;
		if (maxValue < value)
			value = maxValue;
		break;
	}
	}

	return value;
}

bool ImageEditor::pixelIsOn(int value, int low, int high) {
	if (low < high)
		return ((low <= value) && (value <= high));
	else
		return ((low <= value) || (value <= high));
}

bool ImageEditor::pixelIsOn( int value, DatapickerImage::ColorAttributes type, const DatapickerImage::EditorSettings& settings ) {
	switch (type) {
	case DatapickerImage::ColorAttributes::None:
		break;
	case DatapickerImage::ColorAttributes::Intensity:
		return pixelIsOn(value, settings.intensityThresholdLow, settings.intensityThresholdHigh);
	case DatapickerImage::ColorAttributes::Foreground:
		return pixelIsOn(value, settings.foregroundThresholdLow, settings.foregroundThresholdHigh);
	case DatapickerImage::ColorAttributes::Hue:
		return pixelIsOn(value, settings.hueThresholdLow, settings.hueThresholdHigh);
	case DatapickerImage::ColorAttributes::Saturation:
		return pixelIsOn(value, settings.saturationThresholdLow, settings.saturationThresholdHigh);
	case DatapickerImage::ColorAttributes::Value:
		return pixelIsOn(value, settings.valueThresholdLow, settings.valueThresholdHigh);
	}

	return false;
}
