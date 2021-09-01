/*
    File                 : ImageDock.h
    Project              : LabPlot
    Description          : widget for image properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019-2020 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEDOCK_H
#define IMAGEDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/worksheet/TextLabel.h"
#include "ui_imagedock.h"

class AbstractAspect;
class Image;
class KConfig;

class ImageDock : public BaseDock {
	Q_OBJECT

public:
	explicit ImageDock(QWidget*);
	void setImages(QList<Image*>);
	void updateLocale() override;
	void updateUnits() override;

private:
	Ui::ImageDock ui;
	QList<Image*> m_imageList;
	Image* m_image{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	//SLOTs for changes triggered in ImageDock
	void selectFile();
	void fileNameChanged();
	void opacityChanged(int);

	//geometry
	void sizeChanged(int);
	void widthChanged(double);
	void heightChanged(double);
	void keepRatioChanged(int);
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);
	void rotationChanged(int);

	//border
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);

	void visibilityChanged(bool);

	//SLOTs for changes triggered in Image
	void imageFileNameChanged(const QString&);
	void imageOpacityChanged(float);
	void imageWidthChanged(int);
	void imageHeightChanged(int);
	void imageKeepRatioChanged(bool);

	void imagePositionChanged(const WorksheetElement::PositionWrapper&);
	void imageHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment);
	void imageVerticalAlignmentChanged(WorksheetElement::VerticalAlignment);
	void imageRotationAngleChanged(qreal);

	void imageBorderPenChanged(const QPen&);
	void imageBorderOpacityChanged(float);

	void imageVisibleChanged(bool);

signals:
	void info(const QString&);
};

#endif // WORKSHEETDOCK_H
