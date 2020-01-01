/***************************************************************************
    File                 : ImageDock.h
    Project              : LabPlot
    Description          : widget for image properties
    --------------------------------------------------------------------
    Copyright            : (C) 2019 by Alexander Semke (alexander.semke@web.de)

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
	void imageDescriptionChanged(const AbstractAspect*);
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
