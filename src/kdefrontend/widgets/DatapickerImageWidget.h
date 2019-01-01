/***************************************************************************
    File                 : DatapickerImageWidget.h
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

#ifndef DATAPICKERIMAGEWIDGET_H
#define DATAPICKERIMAGEWIDGET_H

#include <QGraphicsView>

#include "ui_datapickerimagewidget.h"
#include "backend/datapicker/DatapickerImage.h"

class QxtSpanSlider;

class HistogramView : public QGraphicsView {
	Q_OBJECT

public:
	explicit HistogramView(QWidget*, int);
	void setScalePixmap(const QString&);
	int *bins{nullptr};

public slots:
	void setSpan(int, int);

private:
	void resizeEvent(QResizeEvent *event) override;
	void drawBackground(QPainter*, const QRectF&) override;
	QGraphicsRectItem* m_lowerSlider;
	QGraphicsRectItem* m_upperSlider;
	QGraphicsScene* m_scene;
	int m_range;
};

class DatapickerImageWidget : public QWidget {
	Q_OBJECT

public:
	explicit DatapickerImageWidget(QWidget*);

	void setImages(QList<DatapickerImage*>);
	void load();

private:
	Ui::DatapickerImageWidget ui;
	void init();
	void initConnections();

	DatapickerImage* m_image;
	QList<DatapickerImage*> m_imagesList;
	bool m_initializing;

	QxtSpanSlider* ssIntensity;
	QxtSpanSlider* ssForeground;
	QxtSpanSlider* ssHue;
	QxtSpanSlider* ssSaturation;
	QxtSpanSlider* ssValue;

	HistogramView* gvIntensity;
	HistogramView* gvForeground;
	HistogramView* gvHue;
	HistogramView* gvSaturation;
	HistogramView* gvValue;

private slots:
	//SLOTs for changes triggered in DatapickerImageWidget
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

	void minSegmentLengthChanged(int);
	void pointSeparationChanged(int);
	void graphTypeChanged();
	void ternaryScaleChanged(double);
	void logicalPositionChanged();

	//symbol propeties
	void pointsStyleChanged(int);
	void pointsSizeChanged(double);
	void pointsRotationChanged(int);
	void pointsOpacityChanged(int);
	void pointsFillingStyleChanged(int);
	void pointsFillingColorChanged(const QColor&);
	void pointsBorderStyleChanged(int);
	void pointsBorderColorChanged(const QColor&);
	void pointsBorderWidthChanged(double);
	void pointsVisibilityChanged(bool);


	//SLOTs for changes triggered in DatapickerImageWidget
	void imageDescriptionChanged(const AbstractAspect*);
	void imageFileNameChanged(const QString&);
	void imageRotationAngleChanged(float);
	void imageAxisPointsChanged(const DatapickerImage::ReferencePoints&);
	void imageEditorSettingsChanged(const DatapickerImage::EditorSettings&);
	void imageMinSegmentLengthChanged(const int);
	void updateSymbolWidgets();
	void handleWidgetActions();
	//symbol
	void symbolStyleChanged(Symbol::Style);
	void symbolSizeChanged(qreal);
	void symbolRotationAngleChanged(qreal);
	void symbolOpacityChanged(qreal);
	void symbolBrushChanged(const QBrush&);
	void symbolPenChanged(const QPen&);
	void symbolVisibleChanged(bool);
};

#endif //DATAPICKERIMAGEWIDGET_H
