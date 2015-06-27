#include "Image.h"
#include "ImagePrivate.h"
#include "backend/worksheet/CustomItem.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/ImageView.h"
#include "backend/core/Transform.h"
#include "backend/core/Datapicker.h"
#include "backend/core/ImageEditor.h"

#include <QMenu>
#include "KIcon"
#include <KConfigGroup>
#include <KLocale>


Image::Image(AbstractScriptingEngine* engine, const QString& name, bool loading)
    : AbstractPart(name), scripted(engine), d(new ImagePrivate(this)),
      isLoaded(false), m_imageEditor(new ImageEditor(this)), m_transform(new Transform()){

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
    d->plotFileName = group.readEntry("PlotFileName", QString());
    d->rotationAngle = group.readEntry("RotationAngle", 0.0);
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
    d->plotErrorTypes.x = (Image::ErrorType) group.readEntry("PlotErrorTypeX", (int) Image::SymmetricError);
    d->plotErrorTypes.y = (Image::ErrorType) group.readEntry("PlotErrorTypeY", (int) Image::NoError);
    d->plotPointsType = (Image::PointsType) group.readEntry("PlotPointsType", (int) Image::AxisPoints);
    plotImageType = (Image::PlotImageType) group.readEntry("PlotImageType", (int) Image::OriginalImage);
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
    const CustomItem* addedElement = qobject_cast<const CustomItem*>(aspect);
    if (addedElement) {
        if (aspect->parentAspect() == this){
            QGraphicsItem *item = addedElement->graphicsItem();
            Q_ASSERT(item != NULL);
            d->m_scene->addItem(item);

            qreal zVal = 0;
            QList<CustomItem *> childElements = children<CustomItem>(IncludeHidden);
            foreach(CustomItem *elem, childElements) {
                elem->graphicsItem()->setZValue(zVal++);
            }
        }
    }
}

void Image::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    const CustomItem *removedElement = qobject_cast<const CustomItem*>(aspect);
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

void Image::setSelectedInView(const bool b){
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
    m_imageEditor->discretize(&processedPlotImage, newSettings);
    emit requestUpdate();
}

void Image::setPlotImageType(const Image::PlotImageType& type) {
    plotImageType = type;
    emit requestUpdate();
}

void Image::updateData(const CustomItem *item, int row) {
    emit updateLogicalPositions();

    QPointF data;
    Datapicker* datapicker = dynamic_cast<Datapicker*>(parentAspect());
    int colCount = 0;
    if (datapicker) {
        data = m_transform->mapSceneToLogical(item->position().point, axisPoints());
        datapicker->addDataToDatasheet(data.x(), colCount++, row, "x");
        datapicker->addDataToDatasheet(data.y(), colCount++, row, "y");

        if (plotErrorTypes().x != NoError) {
            data = m_transform->mapSceneToLogical(QPointF(item->itemErrorBar().plusDeltaX, 0), axisPoints());
            datapicker->addDataToDatasheet(data.x(), colCount++, row, "+delta_x");
            if (plotErrorTypes().x == AsymmetricError) {
                data = m_transform->mapSceneToLogical(QPointF(item->itemErrorBar().minusDeltaX, 0), axisPoints());
                datapicker->addDataToDatasheet(data.x(), colCount++, row, "-delta_x");
            }
        }

        if (plotErrorTypes().y != NoError) {
            data = m_transform->mapSceneToLogical(QPointF(item->itemErrorBar().plusDeltaY, 0), axisPoints());
            datapicker->addDataToDatasheet(data.y(), colCount++, row, "+delta_y");
            if (plotErrorTypes().y == AsymmetricError) {
                data = m_transform->mapSceneToLogical(QPointF(item->itemErrorBar().minusDeltaY, 0), axisPoints());
                datapicker->addDataToDatasheet(data.y(), colCount++, row, "-delta_y");
            }
        }
    }
}

void Image::updateAllData() {
    QList<CustomItem*> childElements = children<CustomItem>(AbstractAspect::IncludeHidden);
    if (childElements.count() > 3) {
        //remove axis points
        childElements.removeAt(0);
        childElements.removeAt(1);
        childElements.removeAt(2);

        int row = 0;
        foreach(CustomItem* elem, childElements)
            updateData(elem, row++);
    }
}

/* =============================== getter methods for background options ================================= */
CLASS_D_READER_IMPL(Image, QString, plotFileName, plotFileName)
CLASS_D_READER_IMPL(Image, Image::ReferencePoints, axisPoints, axisPoints)
CLASS_D_READER_IMPL(Image, Image::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(Image, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(Image, Image::ErrorTypes, plotErrorTypes, plotErrorTypes)
BASIC_D_READER_IMPL(Image, Image::PointsType, plotPointsType, plotPointsType)

/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetPlotFileName, QString, plotFileName, updateFileName)
void Image::setPlotFileName(const QString& fileName) {
    if (fileName!= d->plotFileName)
        exec(new ImageSetPlotFileNameCmd(d, fileName, i18n("%1: set image")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetRotationAngle, float, rotationAngle, update)
void Image::setRotationAngle(float angle) {
    if (angle != d->rotationAngle)
        exec(new ImageSetRotationAngleCmd(d, angle, i18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_S(Image, SetPlotErrorTypes, Image::ErrorTypes, plotErrorTypes)
void Image::setPlotErrorTypes(const ErrorTypes types) {
    if (types.x != d->plotErrorTypes.x || types.y != d->plotErrorTypes.y)
        exec(new ImageSetPlotErrorTypesCmd(d, types, i18n("%1: set Error Type")));
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

//Private implementation

ImagePrivate::ImagePrivate(Image *owner):q(owner),
    pageRect(0, 0, 1500, 1500),
    m_scene(new QGraphicsScene(pageRect)){
}

QString ImagePrivate::name() const{
    return q->name();
}

void ImagePrivate::updatePageRect() {
    m_scene->setSceneRect(pageRect);
}

void ImagePrivate::update(){
    q->update();
}

ImagePrivate::~ImagePrivate(){
    delete m_scene;
}

void ImagePrivate::updateFileName() {
    q->removeAllChildren();
    const QString& fileName = plotFileName.trimmed();
    if ( !fileName.isEmpty() ) {
        q->originalPlotImage.load(fileName);

        q->processedPlotImage = q->originalPlotImage;
        q->m_imageEditor->discretize(&q->processedPlotImage, settings);

        QRect rect = q->originalPlotImage.rect();
        rect.translate(-rect.width()/2,-rect.height()/2);
        m_scene->setSceneRect(rect);

        q->isLoaded = true;
    } else {
        q->isLoaded = false;
    }
    q->update();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Image::save(QXmlStreamWriter* writer) const{
    writer->writeStartElement( "image" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);
    //background properties
    writer->writeStartElement( "background" );
    writer->writeAttribute( "fileName", d->plotFileName );
    writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
    writer->writeEndElement();

    //serialize all children
    QList<CustomItem *> childElements = children<CustomItem>(IncludeHidden);
    foreach(CustomItem *elem, childElements)
        elem->save(writer);

    writer->writeEndElement();
}

//! Load from XML
bool Image::load(XmlStreamReader* reader){
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
            d->plotFileName = str;

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
