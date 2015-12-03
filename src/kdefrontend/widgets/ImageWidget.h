/***************************************************************************
    File                 : ImageWidget.h
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "ui_imagewidget.h"
#include "backend/datapicker/DatapickerImage.h"

class CustomItem;
class CustomItemWidget;
class QxtSpanSlider;

class ImageWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImageWidget(QWidget*);

    void setImages(QList<DatapickerImage*>);
    void load();

private:
    Ui::ImageWidget ui;
    void initConnections();

    DatapickerImage* m_image;
    QList<DatapickerImage*> m_imagesList;
    bool m_initializing;
    CustomItemWidget* customItemWidget;

    QxtSpanSlider* ssIntensity;
    QxtSpanSlider* ssForeground;
    QxtSpanSlider* ssHue;
    QxtSpanSlider* ssSaturation;
    QxtSpanSlider* ssValue;

private slots:
    //SLOTs for changes triggered in ImageWidget
    //"General"-tab
    void nameChanged();
    void commentChanged();
    void fileNameChanged();
    void selectFile();
    void plotImageTypeChanged(int);

    //"Edit image"-tab
    void rotationChanged(double);
    void intensitySpanChanged(int, int);
    void foregroundSpanChanged(int, int);
    void hueSpanChanged(int, int);
    void saturationSpanChanged(int, int);
    void valueSpanChanged(int, int);
    void rbClicked();

    void minSegmentLengthChanged(int);
    void pointSeparationChanged(int);
    void graphTypeChanged();
    void ternaryScaleChanged(double);
    void logicalPositionChanged();

    //SLOTs for changes triggered in ImageWidget
    void imageDescriptionChanged(const AbstractAspect*);
    void imageFileNameChanged(const QString&);
    void imageRotationAngleChanged(float);
    void imageAxisPointsChanged(const DatapickerImage::ReferencePoints&);
    void imageEditorSettingsChanged(const DatapickerImage::EditorSettings&);
    void imageMinSegmentLengthChanged(const int);
    void updateCustomItemList();
    void handleWidgetActions();
};

#endif //IMAGEWIDGET_H
