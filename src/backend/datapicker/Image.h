/***************************************************************************
    File                 : Image.h
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

#ifndef IMAGE_H
#define IMAGE_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/lib/macros.h"
#include <QGraphicsScene>
#include <QVector3D>

class ImagePrivate;
class ImageEditor;
class Segments;

class Image: public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Image(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
        ~Image();

        enum GraphType { Cartesian, PolarInDegree, PolarInRadians, LogarithmicX, LogarithmicY, Ternary};
        enum ColorAttributes { None, Intensity, Foreground, Hue, Saturation, Value };
        enum PlotImageType { OriginalImage, ProcessedImage };
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
		virtual QWidget* view() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		QRectF pageRect() const;
		void setPageRect(const QRectF&);
		QGraphicsScene *scene() const;
		void update();
		void setPrinting(bool) const;
        void setSelectedInView(const bool);
        void setPlotImageType(const Image::PlotImageType&);
        void setSegmentVisible(bool);

        bool isLoaded;
        QImage originalPlotImage;
        QImage processedPlotImage;
        PlotImageType plotImageType;
        QGraphicsPixmapItem* m_magnificationWindow;

        CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
        CLASS_D_ACCESSOR_DECL(Image::ReferencePoints, axisPoints, AxisPoints)
        CLASS_D_ACCESSOR_DECL(Image::EditorSettings, settings, Settings)
        BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
        BASIC_D_ACCESSOR_DECL(PointsType, plotPointsType, PlotPointsType)
        BASIC_D_ACCESSOR_DECL(int, pointSeparation, PointSeparation)
        BASIC_D_ACCESSOR_DECL(int, minSegmentLength, minSegmentLength)

		typedef ImagePrivate Private;

    private:
		void init();
        void initSceneParameters();

		ImagePrivate* const d;
		friend class ImagePrivate;
        Segments* m_segments;
        ImageEditor* m_editor;

    signals:
		void requestProjectContextMenu(QMenu*);
		void requestUpdate();

        void fileNameChanged(const QString&);
        void rotationAngleChanged(float);
        void axisPointsChanged(const Image::ReferencePoints&);
        void settingsChanged(const Image::EditorSettings&);
        void minSegmentLengthChanged(const int);
        friend class ImageSetFileNameCmd;
        friend class ImageSetRotationAngleCmd;
        friend class ImageSetAxisPointsCmd;
        friend class ImageSetSettingsCmd;
        friend class ImageSetMinSegmentLengthCmd;
};
#endif
