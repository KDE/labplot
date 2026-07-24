/*
	File                 : DatapickerImageWidget.h
	Project              : LabPlot
	Description          : widget for datapicker properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERIMAGEWIDGET_H
#define DATAPICKERIMAGEWIDGET_H

#include <QGraphicsView>

#include "backend/datapicker/DatapickerImage.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_datapickerimagewidget.h"

class SymbolWidget;
class SpanSlider;

class HistogramView : public QGraphicsView {
	Q_OBJECT

public:
	explicit HistogramView(QWidget*, int);
	void setScalePixmap(const QString&);
	int* bins{nullptr};

public Q_SLOTS:
	void setSpan(int, int);

private:
	void resizeEvent(QResizeEvent* event) override;
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

	void setImages(const QList<DatapickerImage*>&);
	void load();
	void updateLocale() override;
	void retranslateUi() override;
	void updateXPositionWidgets(bool datetime);

private:
	Ui::DatapickerImageWidget ui;

	DatapickerImage* m_image;
	QList<DatapickerImage*> m_imagesList;
	SymbolWidget* symbolWidget{nullptr};

	SpanSlider* ssIntensity;
	SpanSlider* ssForeground;
	SpanSlider* ssHue;
	SpanSlider* ssSaturation;
	SpanSlider* ssValue;

	HistogramView* gvIntensity;
	HistogramView* gvForeground;
	HistogramView* gvHue;
	HistogramView* gvSaturation;
	HistogramView* gvValue;

private Q_SLOTS:
	// SLOTs for changes triggered in DatapickerImageWidget
	//"General"-tab
	void embeddedChanged(bool embedded);
	void fileNameChanged();
	void relativeChanged(bool checked);
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
	void graphTypeChanged(int);
	void ternaryScaleChanged(double);
	void logicalPositionChanged();
	void dateTimeUsageChanged(bool checked);

	// symbol properties
	void pointsVisibilityChanged(bool);

	// SLOTs for changes triggered in DatapickerImageWidget
	void imageFileNameChanged(const QString&);
	void imageRotationAngleChanged(float);
	void imageAxisPointsChanged(const DatapickerImage::ReferencePoints&);
	void imageEditorSettingsChanged(const DatapickerImage::EditorSettings&);
	void imageMinSegmentLengthChanged(const int);
	void updateSymbolWidgets();
	void handleWidgetActions();
	void symbolVisibleChanged(bool);
	void imageReferencePointSelected(int);
	void imageEmbeddedChanged(bool embedded);
	void imageRelativeChanged(bool);

	void updateFileRelativePathCheckBoxEnable();

	friend class DatapickerTest;
};

#endif // DATAPICKERIMAGEWIDGET_H
