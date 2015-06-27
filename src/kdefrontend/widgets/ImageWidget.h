#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "ui_imagewidget.h"
#include "backend/worksheet/Image.h"

class CustomItem;
class CustomItemWidget;
class QxtSpanSlider;

class ImageWidget: public QWidget{
	Q_OBJECT

public:
    explicit ImageWidget(QWidget *);


    void setImages(QList<Image*>);
	void load();

    QxtSpanSlider* ssIntensity;
    QxtSpanSlider* ssForeground;
    QxtSpanSlider* ssHue;
    QxtSpanSlider* ssSaturation;
    QxtSpanSlider* ssValue;

private:
	Ui::ImageWidget ui;
	Image *m_image;
    QList<CustomItem*> m_itemsList;
    QList<Image*> m_imagesList;
	bool m_initializing;
    CustomItemWidget* customItemWidget;

    void initConnections();

signals:
    //delete it
	void dataChanged(bool);

private slots:
    void rotationChanged(double);
    void updateLogicalPositions();
    void selectFile();
    void fileNameChanged();
    void xErrorTypeChanged(int);
    void yErrorTypeChanged(int);

    void imageFileNameChanged(const QString&);
    void imageRotationAngleChanged(float);
    void handleAspectRemoved();
    void handleAspectAdded();
    void handleWidgetActions();

    void plotImageTypeChanged(int);
    void plotErrorTypeChanged(Image::ErrorTypes);
    void intensitySpanChanged(int, int);
    void foregroundSpanChanged(int, int);
    void hueSpanChanged(int, int);
    void saturationSpanChanged(int, int);
    void valueSpanChanged(int, int);
    void rbClicked();
};

#endif
