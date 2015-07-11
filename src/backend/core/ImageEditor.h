#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QColor>
#include <QList>

#include <backend/worksheet/Image.h>

class ImageEditor {
    public:

        ImageEditor();
        ~ImageEditor();

        int colorAttributeMax(Image::ColorAttributes) const;

        QRgb backgroundColor(const QImage*);

        bool colorCompare(QRgb color1, QRgb color2);

        int discretizeValueNotForeground(int, int, Image::ColorAttributes) const;
        int discretizeValueForeground(int, int, int, int, int) const;

        void discretize(QImage*, QImage*, Image::EditorSettings);

        bool pixelIsOn(int, Image::EditorSettings) const;
        bool processedPixelIsOn(const QImage&, int, int) const;

    private:

        struct ColorEntry {
            QColor color;
            int count;
        };

        typedef QList<ColorEntry> ColorList;

        QImage* m_originalImage;
        bool pixelIsOn(int, int, int) const;
};


#endif // IMAGEEDITOR_H
