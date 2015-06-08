#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "ui_imagewidget.h"
#include "backend/worksheet/Image.h"

class CustomItem;
class CustomItemWidget;

class ImageWidget: public QWidget{
	Q_OBJECT

public:
    explicit ImageWidget(QWidget *);


    void setImages(QList<Image*>);
	void load();

private:
	Ui::ImageWidget ui;
	Image *m_image;
    QList<CustomItem*> m_itemsList;
    QList<Image*> m_imagesList;
	bool m_initializing;
    CustomItemWidget* customItemWidget;

    void initConnections();

signals:
	void dataChanged(bool);

private slots:

	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void rotationChanged(int);
	void visibilityChanged(bool);
    void updateLogicalPositions();

    void imagePositionChanged(const Image::PositionWrapper&);
    void imageRotationAngleChanged(float);
    void imageVisibleChanged(bool);
    void handleAspectRemoved();
    void handleAspectAdded();
};

#endif
