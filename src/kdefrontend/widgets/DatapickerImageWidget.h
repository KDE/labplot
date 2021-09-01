/*
    File                 : DatapickerImageWidget.h
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke (alexander.semke@web.de)

*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef DATAPICKERIMAGEWIDGET_H
#define DATAPICKERIMAGEWIDGET_H

#include <QGraphicsView>

#include "ui_datapickerimagewidget.h"
#include "backend/datapicker/DatapickerImage.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

class SymbolWidget;
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

class DatapickerImageWidget : public BaseDock {
	Q_OBJECT

public:
	explicit DatapickerImageWidget(QWidget*);

	void setImages(QList<DatapickerImage*>);
	void load();
	void updateLocale() override;

private:
	Ui::DatapickerImageWidget ui;

	DatapickerImage* m_image;
	QList<DatapickerImage*> m_imagesList;
	SymbolWidget* symbolWidget{nullptr};

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

	//symbol properties
	void pointsVisibilityChanged(bool);

	//SLOTs for changes triggered in DatapickerImageWidget
	void imageFileNameChanged(const QString&);
	void imageRotationAngleChanged(float);
	void imageAxisPointsChanged(const DatapickerImage::ReferencePoints&);
	void imageEditorSettingsChanged(const DatapickerImage::EditorSettings&);
	void imageMinSegmentLengthChanged(const int);
	void updateSymbolWidgets();
	void handleWidgetActions();
	void symbolVisibleChanged(bool);
};

#endif //DATAPICKERIMAGEWIDGET_H
