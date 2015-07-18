
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

#include "Image.h"
#include "ImagePrivate.h"
#include "backend/core/ImageEditor.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/CustomItem.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/datapicker/ImageView.h"
#include "backend/worksheet/Segments.h"
#include "backend/core/Datapicker.h"
#include "backend/core/Transform.h"

#include <QDesktopWidget>
#include <QMenu>
#include "KIcon"
#include <KConfigGroup>
#include <KLocale>


Image::Image(AbstractScriptingEngine* engine, const QString& name, bool loading)
    : AbstractPart(name), scripted(engine), d(new ImagePrivate(this)),
      plotImageType(Image::OriginalImage),
      isLoaded(false),
      m_imageEditor(new ImageEditor()),
      m_segments(new Segments(this)),
      m_transform(new Transform()){

    connect(this, SIGNAL(aspectAdded(const AbstractAspect*)),
            this, SLOT(handleAspectAdded(const AbstractAspect*)));
    connect(this, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
            this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
    connect(this, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
            this, SLOT(handleAspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)) );

    if (!loading)
        init();
}

Image::~Image() {
    delete d;
}

void Image::init() {
    KConfig config;
    KConfigGroup group = config.group( "Image" );
    d->fileName = group.readEntry("FileName", QString());
    d->rotationAngle = group.readEntry("RotationAngle", 0.0);
    d->minSegmentLength = group.readEntry("MinSegmentLength", 30);
    d->pointSeparation = group.readEntry("PointSeparation", 30);
    d->axisPoints.type = (Image::GraphType) group.readEntry("GraphType", (int) Image::Cartesian);
    d->settings.type = (Image::ColorAttributes) group.readEntry("ColorAttributesType", (int) Image::Intensity);
    d->settings.foregroundThresholdHigh = group.readEntry("ForegroundThresholdHigh", 10);
    d->settings.foregroundThresholdLow = group.readEntry("ForegroundThresholdLow", 0);
    d->settings.hueThresholdHigh = group.readEntry("HueThresholdHigh", 360);
    d->settings.hueThresholdLow = group.readEntry("HueThresholdLow", 180);
    d->settings.intensityThresholdHigh = group.readEntry("IntensityThresholdHigh", 50);
    d->settings.intensityThresholdLow = group.readEntry("IntensityThresholdLow", 0);
    d->settings.saturationThresholdHigh = group.readEntry("SaturationThresholdHigh", 100);
    d->settings.saturationThresholdLow = group.readEntry("SaturationThresholdLow", 50);
    d->settings.valueThresholdHigh = group.readEntry("ValueThresholdHigh", 50);
    d->settings.valueThresholdLow = group.readEntry("ValueThresholdLow", 0);
    d->plotErrors.x = (Image::ErrorType) group.readEntry("PlotErrorTypeX", (int) Image::NoError);
    d->plotErrors.y = (Image::ErrorType) group.readEntry("PlotErrorTypeY", (int) Image::NoError);
    d->plotPointsType = (Image::PointsType) group.readEntry("PlotPointsType", (int) Image::AxisPoints);
}

QIcon Image::icon() const {
    return KIcon("image-x-generic");
}

QMenu* Image::createContextMenu() {
    QMenu* menu = new QMenu(0);
    emit requestProjectContextMenu(menu);
    return menu;
}

QWidget* Image::view() const {
    if (!m_view) {
        m_view = new ImageView(const_cast<Image *>(this));
        connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
    }
    return m_view;
}



void Image::handleAspectAdded(const AbstractAspect* aspect) {
    const WorksheetElement* addedElement = qobject_cast<const WorksheetElement*>(aspect);
    if (addedElement) {
        if (aspect->parentAspect() == this){
            QGraphicsItem *item = addedElement->graphicsItem();
            Q_ASSERT(item != NULL);
            d->m_scene->addItem(item);

            qreal zVal = 0;
            QList<WorksheetElement *> childElements = children<WorksheetElement>(IncludeHidden);
            foreach(WorksheetElement *elem, childElements) {
                elem->graphicsItem()->setZValue(zVal++);
            }
        }
    }
}

void Image::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    const WorksheetElement *removedElement = qobject_cast<const WorksheetElement*>(aspect);
    if (removedElement) {
        QGraphicsItem *item = removedElement->graphicsItem();
        Q_ASSERT(item != NULL);
        d->m_scene->removeItem(item);
    }
}

void Image::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child){
    Q_UNUSED(parent);
    Q_UNUSED(before);
    Q_UNUSED(child);
}

void Image::setSelectedInView(const bool b) {
    if (b)
        emit childAspectSelectedInView(this);
    else
        emit childAspectDeselectedInView(this);
}

QGraphicsScene *Image::scene() const {
    return d->m_scene;
}

QRectF Image::pageRect() const {
    return d->m_scene->sceneRect();
}

void Image::update() {
    emit requestUpdate();
}

void Image::setPlotImageType(const Image::PlotImageType& type) {
    plotImageType = type;
    emit requestUpdate();
}

void Image::setSegmentVisible(bool on) {
    m_segments->setSegmentsVisible(on);
}

void Image::initSceneParameters() {
    setRotationAngle(0.0);
    setminSegmentLength(30);
    setPointSeparation(30);

    ReferencePoints axisPoints;
    axisPoints.type = Image::Cartesian;
    setAxisPoints(axisPoints);

    EditorSettings settings;
    settings.type = Image::Intensity;
    settings.foregroundThresholdHigh = 10;
    settings.foregroundThresholdLow = 0;
    settings.hueThresholdHigh = 360;
    settings.hueThresholdLow = 180;
    settings.intensityThresholdHigh = 50;
    settings.intensityThresholdLow = 0;
    settings.saturationThresholdHigh = 100;
    settings.saturationThresholdLow = 50;
    settings.valueThresholdHigh = 50;
    settings.valueThresholdLow = 0;
    setSettings(settings);

    Errors plotErrors;
    plotErrors.x = Image::NoError;
    plotErrors.y = Image::NoError;
    setPlotErrors(plotErrors);

    PointsType plotPointsType = Image::AxisPoints;
    setPlotPointsType(plotPointsType);
}

void Image::updateData(const CustomItem *item) {
    Datapicker* datapicker = dynamic_cast<Datapicker*>(parentAspect());
    int row = childCount<CustomItem>(AbstractAspect::IncludeHidden) - 4;

    QPointF data;
    if (datapicker) {
        data = m_transform->mapSceneToLogical(item->position().point, axisPoints());
        datapicker->addDataToSheet(data.x(), row, Datapicker::PositionX);
        datapicker->addDataToSheet(data.y(), row, Datapicker::PositionY);

        if (plotErrors().x != Image::NoError) {
            data = m_transform->mapSceneLengthToLogical(QPointF(item->itemErrorBar().plusDeltaX, 0), axisPoints());
            datapicker->addDataToSheet(qAbs(data.x()), row, Datapicker::PlusDeltaX);

            if (plotErrors().x == Image::AsymmetricError) {
                data = m_transform->mapSceneLengthToLogical(QPointF(item->itemErrorBar().minusDeltaX, 0), axisPoints());
                datapicker->addDataToSheet(qAbs(data.x()), row, Datapicker::MinusDeltaX);
            }
        }

        if (plotErrors().y != Image::NoError) {
            data = m_transform->mapSceneLengthToLogical(QPointF(0, item->itemErrorBar().plusDeltaY), axisPoints());
            datapicker->addDataToSheet(qAbs(data.y()), row, Datapicker::PlusDeltaY);

            if (plotErrors().y == Image::AsymmetricError) {
                data = m_transform->mapSceneLengthToLogical(QPointF(0, item->itemErrorBar().minusDeltaY), axisPoints());
                datapicker->addDataToSheet(qAbs(data.y()), row, Datapicker::MinusDeltaY);
            }
        }
    }
}

/* =============================== getter methods for background options ================================= */
CLASS_D_READER_IMPL(Image, QString, fileName, fileName)
CLASS_D_READER_IMPL(Image, Image::ReferencePoints, axisPoints, axisPoints)
CLASS_D_READER_IMPL(Image, Image::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(Image, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(Image, Image::Errors, plotErrors, plotErrors)
BASIC_D_READER_IMPL(Image, Image::PointsType, plotPointsType, plotPointsType)
BASIC_D_READER_IMPL(Image, int, pointSeparation, pointSeparation)
BASIC_D_READER_IMPL(Image, int, minSegmentLength, minSegmentLength)

/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetFileName, QString, fileName, updateFileName)
void Image::setFileName(const QString& fileName) {
    if (fileName!= d->fileName) {
        beginMacro(i18n("%1: upload new image", name()));
        exec(new ImageSetFileNameCmd(d, fileName, i18n("%1: upload image")));
        endMacro();
    }
}

STD_SETTER_CMD_IMPL_F_S(Image, SetRotationAngle, float, rotationAngle, update)
void Image::setRotationAngle(float angle) {
    if (angle != d->rotationAngle)
        exec(new ImageSetRotationAngleCmd(d, angle, i18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_S(Image, SetPlotErrors, Image::Errors, plotErrors)
void Image::setPlotErrors(const Errors types) {
    if (types.x != d->plotErrors.x || types.y != d->plotErrors.y)
        exec(new ImageSetPlotErrorsCmd(d, types, i18n("%1: set Error Type")));
}

STD_SETTER_CMD_IMPL_S(Image, SetAxisPoints, Image::ReferencePoints, axisPoints)
void Image::setAxisPoints(const Image::ReferencePoints& points) {
    if (memcmp(&points, &d->axisPoints, sizeof(points)) != 0)
        exec(new ImageSetAxisPointsCmd(d, points, i18n("%1: set Axis points")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetSettings, Image::EditorSettings, settings, discretize)
void Image::setSettings(const Image::EditorSettings& editorSettings) {
    if (memcmp(&editorSettings, &d->settings, sizeof(editorSettings)) != 0)
        exec(new ImageSetSettingsCmd(d, editorSettings, i18n("%1: set editor settings")));
}

void Image::setPrinting(bool on) const {
    QList<CustomItem*> childElements = children<CustomItem>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
    foreach(CustomItem* elem, childElements)
        elem->setPrinting(on);
}

void Image::setPlotPointsType(const PointsType pointsType) {
    d->plotPointsType = pointsType;
}

void Image::setminSegmentLength(const int value) {
    d->minSegmentLength = value;
}

void Image::setPointSeparation(const int value) {
    d->pointSeparation = value;
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
ImagePrivate::ImagePrivate(Image *owner):q(owner),
    pageRect(0, 0, 1500, 1500),
    m_scene(new QGraphicsScene(pageRect)) {
}

QString ImagePrivate::name() const {
    return q->name();
}

bool ImagePrivate::uploadImage(const QString& address) {
    bool rc = q->originalPlotImage.load(address);
    if (rc) {
        q->processedPlotImage = q->originalPlotImage;
        discretize();

        //resize the screen
        double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
        double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
        m_scene->setSceneRect(0, 0, w, h);
        q->isLoaded = true;
    }
    return rc;
}

void ImagePrivate::update() {
    q->update();
}

void ImagePrivate::discretize() {
    q->m_imageEditor->discretize(&q->processedPlotImage, &q->originalPlotImage, settings);

    //update segments
    q->m_segments->makeSegments(q->processedPlotImage);
    if (plotPointsType == Image::SegmentPoints)
        q->m_segments->setSegmentsVisible(true);
    update();
}

ImagePrivate::~ImagePrivate() {
    delete m_scene;
}

void ImagePrivate::updateFileName() {
    WAIT_CURSOR;
    int childCount = q->childCount<CustomItem>(AbstractAspect::IncludeHidden);
    if (childCount)
        q->removeAllChildren();

    q->isLoaded = false;
    const QString& address = fileName.trimmed();

    if ( !address.isEmpty() ) {
        if (uploadImage(address)) {
            //TODO: scene size was change -> reinitialize all the screen size dependent parameters
            q->initSceneParameters();
            fileName = address;
        }
    }

    update();
    RESET_CURSOR;
}
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Image::save(QXmlStreamWriter* writer) const {
    writer->writeStartElement( "image" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);
    //general properties
    writer->writeStartElement( "general" );
    writer->writeAttribute( "fileName", d->fileName );
    writer->writeAttribute( "plotErrorTypeX", QString::number(d->plotErrors.x) );
    writer->writeAttribute( "plotErrorTypeY", QString::number(d->plotErrors.y) );
    writer->writeAttribute( "plotPointsType", QString::number(d->plotPointsType) );
    writer->writeEndElement();

    writer->writeStartElement( "axisPoint" );
    writer->writeAttribute( "graphType", QString::number(d->axisPoints.type) );
    writer->writeAttribute( "axisPointLogicalX1", QString::number(d->axisPoints.logicalPos[0].x()) );
    writer->writeAttribute( "axisPointLogicalY1", QString::number(d->axisPoints.logicalPos[0].y()) );
    writer->writeAttribute( "axisPointLogicalX2", QString::number(d->axisPoints.logicalPos[1].x()) );
    writer->writeAttribute( "axisPointLogicalY2", QString::number(d->axisPoints.logicalPos[1].y()) );
    writer->writeAttribute( "axisPointLogicalX3", QString::number(d->axisPoints.logicalPos[2].x()) );
    writer->writeAttribute( "axisPointLogicalY3", QString::number(d->axisPoints.logicalPos[2].y()) );
    writer->writeAttribute( "axisPointSceneX1", QString::number(d->axisPoints.scenePos[0].x()) );
    writer->writeAttribute( "axisPointSceneY1", QString::number(d->axisPoints.scenePos[0].y()) );
    writer->writeAttribute( "axisPointSceneX2", QString::number(d->axisPoints.scenePos[1].x()) );
    writer->writeAttribute( "axisPointSceneY2", QString::number(d->axisPoints.scenePos[1].y()) );
    writer->writeAttribute( "axisPointSceneX3", QString::number(d->axisPoints.scenePos[2].x()) );
    writer->writeAttribute( "axisPointSceneY3", QString::number(d->axisPoints.scenePos[2].y()) );
    writer->writeEndElement();

    //editor and segment settings
    writer->writeStartElement( "editorSettings" );
    writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
    writer->writeAttribute( "minSegmentLength", QString::number(d->minSegmentLength) );
    writer->writeAttribute( "pointSeparation", QString::number(d->pointSeparation) );
    writer->writeAttribute( "colorAttributesType", QString::number(d->settings.type) );
    writer->writeAttribute( "foregroundThresholdHigh", QString::number(d->settings.foregroundThresholdHigh) );
    writer->writeAttribute( "foregroundThresholdLow", QString::number(d->settings.foregroundThresholdLow) );
    writer->writeAttribute( "hueThresholdHigh", QString::number(d->settings.hueThresholdHigh) );
    writer->writeAttribute( "hueThresholdLow", QString::number(d->settings.hueThresholdLow) );
    writer->writeAttribute( "intensityThresholdHigh", QString::number(d->settings.intensityThresholdHigh) );
    writer->writeAttribute( "intensityThresholdLow", QString::number(d->settings.intensityThresholdLow) );
    writer->writeAttribute( "saturationThresholdHigh", QString::number(d->settings.saturationThresholdHigh) );
    writer->writeAttribute( "saturationThresholdLow", QString::number(d->settings.saturationThresholdLow) );
    writer->writeAttribute( "valueThresholdHigh", QString::number(d->settings.valueThresholdHigh) );
    writer->writeAttribute( "valueThresholdLow", QString::number(d->settings.valueThresholdLow) );
    writer->writeEndElement();

    //serialize all children
    QList<WorksheetElement *> childElements = children<WorksheetElement>(IncludeHidden);
    foreach(WorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement();
}

//! Load from XML
bool Image::load(XmlStreamReader* reader) {
    if(!reader->isStartElement() || reader->name() != "image") {
        reader->raiseError(i18n("no image element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "image")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        } else if (reader->name() == "general") {
            attribs = reader->attributes();

            str = attribs.value("fileName").toString();
            d->fileName = str;

            str = attribs.value("plotErrorTypeX").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("plotErrorTypeX"));
            else
                d->plotErrors.x = Image::ErrorType(str.toInt());

            str = attribs.value("plotErrorTypeY").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("plotErrorTypeY"));
            else
                d->plotErrors.y = Image::ErrorType(str.toInt());

            str = attribs.value("plotPointsType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("plotPointsType"));
            else
                d->plotPointsType = Image::PointsType(str.toInt());

        } else if (reader->name() == "axisPoint") {
            attribs = reader->attributes();

            str = attribs.value("graphType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("graphType"));
            else
                d->axisPoints.type = Image::GraphType(str.toInt());

            str = attribs.value("axisPointLogicalX1").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalX1"));
            else
                d->axisPoints.logicalPos[0].setX(str.toDouble());

            str = attribs.value("axisPointLogicalY1").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalY1"));
            else
                d->axisPoints.logicalPos[0].setY(str.toDouble());

            str = attribs.value("axisPointLogicalX2").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalX2"));
            else
                d->axisPoints.logicalPos[1].setX(str.toDouble());

            str = attribs.value("axisPointLogicalY2").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalY2"));
            else
                d->axisPoints.logicalPos[1].setY(str.toDouble());

            str = attribs.value("axisPointLogicalX3").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalX3"));
            else
                d->axisPoints.logicalPos[2].setX(str.toDouble());

            str = attribs.value("axisPointLogicalY3").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointLogicalY3"));
            else
                d->axisPoints.logicalPos[2].setY(str.toDouble());

            str = attribs.value("axisPointSceneX1").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneX1"));
            else
                d->axisPoints.scenePos[0].setX(str.toDouble());

            str = attribs.value("axisPointSceneY1").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneY1"));
            else
                d->axisPoints.scenePos[0].setY(str.toDouble());

            str = attribs.value("axisPointSceneX2").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneX2"));
            else
                d->axisPoints.scenePos[1].setX(str.toDouble());

            str = attribs.value("axisPointSceneY2").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneY2"));
            else
                d->axisPoints.scenePos[1].setY(str.toDouble());

            str = attribs.value("axisPointSceneX3").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneX3"));
            else
                d->axisPoints.scenePos[2].setX(str.toDouble());

            str = attribs.value("axisPointSceneY3").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("axisPointSceneY3"));
            else
                d->axisPoints.scenePos[2].setY(str.toDouble());

        } else if (reader->name() == "editorSettings") {
            attribs = reader->attributes();

            str = attribs.value("rotationAngle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("rotationAngle"));
            else
                d->rotationAngle = str.toFloat();

            str = attribs.value("minSegmentLength").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("minSegmentLength"));
            else
                d->minSegmentLength = str.toInt();

            str = attribs.value("pointSeparation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("pointSeparation"));
            else
                d->pointSeparation = str.toInt();

            str = attribs.value("colorAttributesType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("colorAttributesType"));
            else
                d->settings.type = Image::ColorAttributes(str.toInt());

            str = attribs.value("foregroundThresholdHigh").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("foregroundThresholdHigh"));
            else
                d->settings.foregroundThresholdHigh = str.toInt();

            str = attribs.value("foregroundThresholdLow").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("foregroundThresholdLow"));
            else
                d->settings.foregroundThresholdLow = str.toInt();

            str = attribs.value("hueThresholdHigh").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("hueThresholdHigh"));
            else
                d->settings.hueThresholdHigh = str.toInt();

            str = attribs.value("hueThresholdLow").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("hueThresholdLow"));
            else
                d->settings.hueThresholdLow = str.toInt();

            str = attribs.value("intensityThresholdHigh").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("intensityThresholdHigh"));
            else
                d->settings.intensityThresholdHigh = str.toInt();

            str = attribs.value("intensityThresholdLow").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("intensityThresholdLow"));
            else
                d->settings.intensityThresholdLow = str.toInt();

            str = attribs.value("saturationThresholdHigh").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("saturationThresholdHigh"));
            else
                d->settings.saturationThresholdHigh = str.toInt();

            str = attribs.value("saturationThresholdLow").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("saturationThresholdLow"));
            else
                d->settings.saturationThresholdLow = str.toInt();

            str = attribs.value("valueThresholdHigh").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("valueThresholdHigh"));
            else
                d->settings.valueThresholdHigh = str.toInt();

            str = attribs.value("valueThresholdLow").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("valueThresholdLow"));
            else
                d->settings.valueThresholdLow = str.toInt();

        } else if(reader->name() == "customItem") {
            CustomItem* customItem = new CustomItem("");
            customItem->setHidden(true);
            if (!customItem->load(reader)){
                delete customItem;
                return false;
            } else {
                addChild(customItem);
            }
        } else { // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    d->uploadImage(d->fileName);
    update();
    return true;
}
