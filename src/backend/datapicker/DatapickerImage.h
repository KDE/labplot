/***************************************************************************
  File                 : DatapickerImage.h
  Project              : LabPlot
  Description          : Worksheet for Datapicker
  --------------------------------------------------------------------
  Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#ifndef DATAPICKERIMAGE_H
#define DATAPICKERIMAGE_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/lib/macros.h"
#include <QImage>
#include <QVector3D>

class DatapickerImagePrivate;
class ImageEditor;
class Segments;

class QGraphicsScene;
class QGraphicsPixmapItem;

class DatapickerImage: public AbstractPart, public scripted {
	Q_OBJECT

public:
	DatapickerImage(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
	~DatapickerImage();

	enum GraphType { Cartesian, PolarInDegree, PolarInRadians, LogarithmicX, LogarithmicY, Ternary};
	enum ColorAttributes { None, Intensity, Foreground, Hue, Saturation, Value };
	enum PlotImageType { NoImage, OriginalImage, ProcessedImage };
	enum PointsType { AxisPoints, CurvePoints, SegmentPoints };

	struct ReferencePoints {
		GraphType type;
		QPointF scenePos[3];
		QVector3D logicalPos[3];
		double ternaryScale;
	};

	struct EditorSettings {
		ColorAttributes type;
		int intensityThresholdLow;
		int intensityThresholdHigh;
		int foregroundThresholdLow;
		int foregroundThresholdHigh;
		int hueThresholdLow;
		int hueThresholdHigh;
		int saturationThresholdLow;
		int saturationThresholdHigh;
		int valueThresholdLow;
		int valueThresholdHigh;
	};

	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();
	void createContextMenu(QMenu*);
	virtual QWidget* view() const;

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

	QRectF pageRect() const;
	void setPageRect(const QRectF&);
	QGraphicsScene *scene() const;
	void setPrinting(bool) const;
	void setSelectedInView(const bool);

	void setPlotImageType(const DatapickerImage::PlotImageType);
	DatapickerImage::PlotImageType plotImageType();

	bool isLoaded;
	QImage originalPlotImage;
	QImage processedPlotImage;
	QGraphicsPixmapItem* m_magnificationWindow;

	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::ReferencePoints, axisPoints, AxisPoints)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::EditorSettings, settings, Settings)
	BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(PointsType, plotPointsType, PlotPointsType)
	BASIC_D_ACCESSOR_DECL(int, pointSeparation, PointSeparation)
	BASIC_D_ACCESSOR_DECL(int, minSegmentLength, minSegmentLength)

	typedef DatapickerImagePrivate Private;

private:
	void init();
	void initSceneParameters();

	DatapickerImagePrivate* const d;
	friend class DatapickerImagePrivate;
	Segments* m_segments;
	ImageEditor* m_editor;

signals:
	void requestProjectContextMenu(QMenu*);
	void requestUpdate();

	void fileNameChanged(const QString&);
	void rotationAngleChanged(float);
	void axisPointsChanged(const DatapickerImage::ReferencePoints&);
	void settingsChanged(const DatapickerImage::EditorSettings&);
	void minSegmentLengthChanged(const int);
	friend class DatapickerImageSetFileNameCmd;
	friend class DatapickerImageSetRotationAngleCmd;
	friend class DatapickerImageSetAxisPointsCmd;
	friend class DatapickerImageSetSettingsCmd;
	friend class DatapickerImageSetMinSegmentLengthCmd;
};
#endif
