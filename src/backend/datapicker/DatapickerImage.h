/*
	File                 : DatapickerImage.h
	Project              : LabPlot
	Description          : Worksheet for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERIMAGE_H
#define DATAPICKERIMAGE_H

#include "Vector3D.h"
#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QPen>

class DatapickerImagePrivate;
class DatapickerImageView;
class DatapickerPoint;
class Segments;

class QGraphicsScene;
class QGraphicsPixmapItem;

class DatapickerImage : public AbstractPart {
	Q_OBJECT

public:
	explicit DatapickerImage(const QString& name, bool loading = false);
	~DatapickerImage() override;
	bool addChild(AbstractAspect* child) override;

	enum class GraphType { Linear, PolarInDegree, PolarInRadians, LnX, LnY, Ternary, LnXY, Log10XY, Log10X, Log10Y };
	enum class ColorAttributes { None, Intensity, Foreground, Hue, Saturation, Value };
	enum class PlotImageType { NoImage, OriginalImage, ProcessedImage };
	enum class PointsType { AxisPoints, CurvePoints, SegmentPoints };

	struct ReferencePoints {
		void clearPoints();
		GraphType type{GraphType::Linear};
		QPointF scenePos[3];
		Vector3D logicalPos[3];
		double ternaryScale{1.0};
		bool datetime{false}; // Datetime for the x axis
	};

	struct EditorSettings {
		int hueThresholdLow{0};
		int hueThresholdHigh{360};
		int saturationThresholdLow{0};
		int saturationThresholdHigh{100};
		int valueThresholdLow{0};
		int valueThresholdHigh{100};
		int intensityThresholdLow{0};
		int intensityThresholdHigh{100};
		int foregroundThresholdLow{20};
		int foregroundThresholdHigh{100};
	};

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void createContextMenu(QMenu*);
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QRectF pageRect() const;
	void setPageRect(const QRectF&);
	QGraphicsScene* scene() const;
	void setPrinting(bool) const;
	void setSelectedInView(const bool);
	void setSegmentsHoverEvent(const bool);
	int currentSelectedReferencePoint() const;

	void setPlotImageType(const DatapickerImage::PlotImageType);
	DatapickerImage::PlotImageType plotImageType() const;
	void setImage(const QImage&, const QString& filename, bool embedded);
	void setImage(const QString&, bool embedded);

	static QString graphTypeToString(const GraphType);
	static GraphType stringToGraphType(const QString&);

	bool isLoaded{false};
	QImage originalPlotImage;
	QImage processedPlotImage;
	QColor background;
	int* foregroundBins;
	int* hueBins;
	int* saturationBins;
	int* valueBins;
	int* intensityBins;

	QGraphicsPixmapItem* m_magnificationWindow{nullptr};

	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	BASIC_D_ACCESSOR_DECL(bool, isRelativeFilePath, RelativeFilePath)
	BASIC_D_ACCESSOR_DECL(bool, embedded, Embedded)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::ReferencePoints, axisPoints, AxisPoints)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::EditorSettings, settings, Settings)
	BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(PointsType, plotPointsType, PlotPointsType)
	BASIC_D_ACCESSOR_DECL(int, pointSeparation, PointSeparation)
	BASIC_D_ACCESSOR_DECL(int, minSegmentLength, minSegmentLength)

	void clearReferencePoints();

	Symbol* symbol() const;
	BASIC_D_ACCESSOR_DECL(bool, pointVisibility, PointVisibility)

	typedef DatapickerImagePrivate Private;

public Q_SLOTS:
	void referencePointSelected(const DatapickerPoint*);

private:
	void init();
	void childAdded(const AbstractAspect* child);
	void datapickerPointChanged(const DatapickerPoint*);
	void childRemoved(const AbstractAspect* child);

	Q_DECLARE_PRIVATE(DatapickerImage)
	DatapickerImagePrivate* const d_ptr;

	mutable DatapickerImageView* m_view{nullptr};
	friend class DatapickerImagePrivate;
	Segments* m_segments;
	int m_currentRefPoint{-1};

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
	void requestUpdate();
	void requestUpdateActions();

	void relativeFilePathChanged(bool);
	void fileNameChanged(const QString&);
	void embeddedChanged(bool);
	void rotationAngleChanged(float);
	void axisPointsChanged(const DatapickerImage::ReferencePoints&);
	void axisPointsRemoved();
	void settingsChanged(const DatapickerImage::EditorSettings&);
	void minSegmentLengthChanged(const int);
	void pointStyleChanged(Symbol::Style);
	void pointSizeChanged(qreal);
	void pointRotationAngleChanged(qreal);
	void pointOpacityChanged(qreal);
	void pointBrushChanged(QBrush);
	void pointPenChanged(const QPen&);
	void pointVisibilityChanged(bool);
	void referencePointSelected(int index);
};
#endif
