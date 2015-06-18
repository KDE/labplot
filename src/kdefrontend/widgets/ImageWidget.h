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
    //delete it
	void dataChanged(bool);

private slots:
    void rotationChanged(double);
    void updateLogicalPositions();
    void selectFile();
    void fileNameChanged();

    void imageFileNameChanged(const QString&);
    void imageRotationAngleChanged(float);
    void handleAspectRemoved();
    void handleAspectAdded();
};

#endif
