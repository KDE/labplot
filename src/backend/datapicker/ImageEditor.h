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

#include <QColor>
#include <QList>

#include <backend/datapicker/Image.h>

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
