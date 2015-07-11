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

QMenu *Image::createContextMenu() {
    // 	QMenu *menu = AbstractPart::createContextMenu();
    //     Q_ASSERT(menu);
    QMenu* menu = new QMenu(0);
    emit requestProjectContextMenu(menu);
    return menu;
}

QWidget *Image::view() const {
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

void Image::discretize(const EditorSettings& newSettings) {
    d->settings = newSettings;
    m_imageEditor->discretize(&processedPlotImage, &originalPlotImage, newSettings);
    m_segments->makeSegments(processedPlotImage);
    if (plotPointsType() == Image::SegmentPoints)
        m_segments->setSegmentsVisible(true);
    emit requestUpdate();
}

void Image::setPlotImageType(const Image::PlotImageType& type) {
    plotImageType = type;
    emit requestUpdate();
}

void Image::updateAxisPoints() {
    emit requestUpdateAxisPoints();
}

void Image::setSegmentVisible(bool on) {
    m_segments->setSegmentsVisible(on);
}


void Image::updateData(const CustomItem *item) {
    updateAxisPoints();

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
    if (fileName!= d->fileName)
        exec(new ImageSetFileNameCmd(d, fileName, i18n("%1: set image")));
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

void Image::setPrinting(bool on) const {
    QList<CustomItem*> childElements = children<CustomItem>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
    foreach(CustomItem* elem, childElements)
        elem->setPrinting(on);
}

void Image::setPlotPointsType(const PointsType pointsType) {
    d->plotPointsType = pointsType;
}

void Image::setAxisPoints(const Image::ReferencePoints& points) {
    d->axisPoints = points;
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

void ImagePrivate::updatePageRect() {
    m_scene->setSceneRect(pageRect);
}

void ImagePrivate::update() {
    q->update();
}

ImagePrivate::~ImagePrivate() {
    delete m_scene;
}

void ImagePrivate::updateFileName() {
    WAIT_CURSOR;
    q->removeAllChildren();
	q->isLoaded = false;
    const QString& address = fileName.trimmed();

    if ( !address.isEmpty() ) {
        bool rc = q->originalPlotImage.load(address);
		if (rc) {
            q->processedPlotImage = q->originalPlotImage;
            q->m_imageEditor->discretize(&q->processedPlotImage, &q->originalPlotImage, settings);

            //resize the screen
            double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
            double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
            m_scene->setSceneRect(0, 0, w, h);

			//TODO: scene size was change -> reinitialize all the screen size dependent parameters
            q->init();
            q->m_segments->makeSegments(q->processedPlotImage);
            fileName = address;

			q->isLoaded = true;
		}
    }

    q->update();
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
    //background properties
    writer->writeStartElement( "background" );
    writer->writeAttribute( "fileName", d->fileName );
    writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
    writer->writeEndElement();

    //serialize all children
    QList<CustomItem *> childElements = children<CustomItem>(IncludeHidden);
    foreach(CustomItem *elem, childElements)
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
        if (reader->isEndElement() && reader->name() == "worksheet")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        } else if (reader->name() == "background") {
            attribs = reader->attributes();

            str = attribs.value("fileName").toString();
            d->fileName = str;

            str = attribs.value("rotationAngle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("rotationAngle"));
            else
                d->rotationAngle = str.toFloat();
        } else if(reader->name() == "customItem") {
            //change it with
            CustomItem* customItem = new CustomItem("");
            if (!customItem->load(reader)){
                delete customItem;
                return false;
            }else{
                addChild(customItem);
            }
        } else { // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    d->updateFileName();
    return true;
}
