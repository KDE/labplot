#include "ImageEditor.h"

#include "math.h"

ImageEditor::ImageEditor() {
}

ImageEditor::~ImageEditor() {
}

QRgb ImageEditor::backgroundColor(const QImage* plotImage) {
    QList<ColorEntry>::iterator itrC;
    ColorList colors;
    int x, y = 0;
    for (x = 0; x < plotImage->width(); x++) {
        ColorEntry c;
        c.color = plotImage->pixel(x,y);
        c.count = 0;

        bool found = false;
        for (itrC = colors.begin(); itrC != colors.end(); ++itrC) {
            if (colorCompare(c.color.rgb(), (*itrC).color.rgb())){
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

int ImageEditor::colorAttributeMax(Image::ColorAttributes type) const {
    //Intensity, Foreground, Saturation and Value are from 0 to 100
    //Hue is from 0 to 360
    switch (type) {
    case Image::None:
        return 0;
    case Image::Intensity:
        return 100;
    case Image::Foreground:
        return 100;
    case Image::Hue:
        return 360;
    case Image::Saturation:
        return 100;
    case Image::Value:
    default:
        return 100;
    }
}

bool ImageEditor::colorCompare(QRgb color1, QRgb color2) {
    const long MASK = 0xf0f0f0f0;
    return (color1 & MASK) == (color2 & MASK);
}

void ImageEditor::discretize(QImage* plotImage, QImage* originalImage, Image::EditorSettings settings) {
    m_originalImage = originalImage;
    QColor background;
    int rBg, gBg, bBg;
    int value;
    static Image::EditorSettings settingsLatest;
    settingsLatest = settings;

    for (int x = 0; x < plotImage->width(); x++) {
        for (int y = 0; y < plotImage->height(); y++) {
            if (settings.type == Image::Foreground) {
                background = backgroundColor(m_originalImage);
                rBg = background.red();
                gBg = background.green();
                bBg = background.blue();
                value = discretizeValueForeground(x, y, rBg, gBg, bBg);
            } else {
                value = discretizeValueNotForeground(x, y, settings.type);
            }
            if (pixelIsOn(value, settings))
                plotImage->setPixel(x, y, QColor(Qt::black).rgb());
            else
                plotImage->setPixel(x, y, QColor(Qt::white).rgb());
        }
        //change it
        qApp->processEvents();
        if (memcmp(&settings, &settingsLatest, sizeof(settingsLatest)) != 0)
            break;
    }
}

int ImageEditor::discretizeValueForeground(int x, int y, int rBg, int gBg, int bBg) const {
    int attributeMax = colorAttributeMax(Image::Foreground);

    QColor color(m_originalImage->pixel(x,y));
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    double distance;
    distance = sqrt ((double) ((r - rBg) * (r - rBg) + (g - gBg) * (g - gBg) + (b - bBg) * (b - bBg)));
    int value = (int) (distance * attributeMax / sqrt((double) (255 * 255 + 255 * 255 + 255 * 255)) + 0.5);

    if (value < 0)
        value = 0;
    if (attributeMax < value)
        value = attributeMax;

    return value;
}

int ImageEditor::discretizeValueNotForeground(int x, int y, Image::ColorAttributes type) const{
    QColor color(m_originalImage->pixel(x,y));
    int r, g, b, h, s, v;
    double intensity;
    int attributeMax = colorAttributeMax(type);

    // convert hue from 0 to 359, saturation from 0 to 255, value from 0 to 255
    int value = 0;
    switch (type) {
    case Image::None:
        break;
    case Image::Intensity:
        r = color.red();
        g = color.green();
        b = color.blue();
        intensity = sqrt ((double) (r * r + g * g + b * b));
        value = (int) (intensity * attributeMax / sqrt((double) (255 * 255 + 255 * 255 + 255 * 255)) + 0.5);
        break;
    case Image::Foreground:
        break;
    case Image::Hue:
        h = color.hue();
        value = h * attributeMax / 359;
        break;
    case Image::Saturation:
        s = color.saturation();
        value = s * attributeMax / 255;
        break;
    case Image::Value:
        v = color.value();
        value = v * attributeMax / 255;
        break;
    }

    if (value < 0)
        value = 0;
    if (attributeMax < value)
        value = attributeMax;

    return value;
}

bool ImageEditor::pixelIsOn(int value, int low, int high) const {
    if (low < high)
        return ((low <= value) && (value <= high));
    else
        return ((low <= value) || (value <= high));
}

bool ImageEditor::pixelIsOn(int value, Image::EditorSettings settings) const {
    switch (settings.type) {
    case Image::None:
        break;
    case Image::Intensity:
        return pixelIsOn(value, settings.intensityThresholdLow, settings.intensityThresholdHigh);
    case Image::Foreground:
        return pixelIsOn(value, settings.foregroundThresholdLow, settings.foregroundThresholdHigh);
    case Image::Hue:
        return pixelIsOn(value, settings.hueThresholdLow, settings.hueThresholdHigh);
    case Image::Saturation:
        return pixelIsOn(value, settings.saturationThresholdLow, settings.saturationThresholdHigh);
    case Image::Value:
        return pixelIsOn(value, settings.valueThresholdLow, settings.valueThresholdHigh);
    }
    return false;
}

bool ImageEditor::processedPixelIsOn(const QImage& plotImage, int x, int y) const {
    if ((x < 0) || (plotImage.width() <= x) || (y < 0) || (plotImage.height() <= y))
        return false;


    // pixel is on if it is closer to black than white in gray scale. this test must be performed
    // on little endian and big endian systems, with or without alpha bits (which are typically high bits)
    const int BLACK_WHITE_THRESHOLD = 255 / 2; // put threshold in middle of range
    int gray = qGray(plotImage.pixel(x, y));
    return (gray < BLACK_WHITE_THRESHOLD);
}
