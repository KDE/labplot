/*
	File                 : ImageDock.h
	Project              : LabPlot
	Description          : widget for image properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEDOCK_H
#define IMAGEDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "ui_imagedock.h"

class Image;
class LineWidget;
class KConfig;

class ImageDock : public BaseDock {
	Q_OBJECT

public:
	explicit ImageDock(QWidget*);
	void setImages(QList<Image*>);
	void updateLocale() override;
	void retranslateUi() override;
	void updateUnits() override;

private:
	Ui::ImageDock ui;
	QList<Image*> m_imageList;
	Image* m_image{nullptr};
	LineWidget* borderLineWidget{nullptr};

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	// SLOTs for changes triggered in ImageDock
	void selectFile();
	void fileNameChanged();
	void embeddedChanged(int);
	void opacityChanged(int);

	// geometry
	void widthChanged(double);
	void heightChanged(double);
	void keepRatioChanged(int);
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void positionXLogicalChanged(double);
	void positionXLogicalDateTimeChanged(qint64);
	void positionYLogicalChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);
	void rotationChanged(int);

	void lockChanged(bool);
	void bindingChanged(bool checked);

	// SLOTs for changes triggered in Image
	void imageFileNameChanged(const QString&);
	void imageEmbeddedChanged(bool);
	void imageOpacityChanged(float);
	void imageWidthChanged(int);
	void imageHeightChanged(int);
	void imageKeepRatioChanged(bool);

	void imagePositionChanged(const WorksheetElement::PositionWrapper&);
	void imagePositionLogicalChanged(QPointF);
	void imageCoordinateBindingEnabledChanged(bool);
	void imageHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment);
	void imageVerticalAlignmentChanged(WorksheetElement::VerticalAlignment);
	void imageRotationAngleChanged(qreal);

	void imageLockChanged(bool);

Q_SIGNALS:
	void info(const QString&);

	friend class WorksheetElementTest;
};

#endif // WORKSHEETDOCK_H
