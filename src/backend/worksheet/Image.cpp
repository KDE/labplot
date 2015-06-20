#include "Image.h"
#include "ImagePrivate.h"
#include "WorksheetElement.h"
#include "backend/worksheet/CustomItem.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/datapicker/ImageView.h"
#include "backend/core/Transform.h"
#include "backend/core/Datapicker.h"

#include <QMenu>
#include "KIcon"
#include <KConfigGroup>
#include <KLocale>


Image::Image(AbstractScriptingEngine* engine, const QString& name, bool loading)
        : AbstractPart(name), scripted(engine), isLoaded(false),
        d(new ImagePrivate(this)), m_transform(new Transform(this)){

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
    d->imageFileName = group.readEntry("ImageFileName", QString());
    d->drawPoints = group.readEntry("DrawPoints", false);
    d->rotationAngle = group.readEntry("RotationAngle", 0.0);
    d->points.type = (Image::GraphType) group.readEntry("GraphType", (int) Image::Cartesian);
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
    int count = childCount<WorksheetElement>(IncludeHidden);
	if (addedElement) {
		if (aspect->parentAspect() == this){
			QGraphicsItem *item = addedElement->graphicsItem();
			Q_ASSERT(item != NULL);
            d->m_scene->addItem(item);

            if (count <= 3) {
                d->points.scenePos[count - 1].setX(item->pos().x());
                d->points.scenePos[count - 1].setY(item->pos().y());
                if (count == 3)
                    d->drawPoints = false;
            } else {
                emit updateLogicalPositions();
                QPointF point = m_transform->mapSceneToLogical(item->pos());
                emit addDataToSheet(point, count - 4);
            }

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

void Image::update(){
	emit requestUpdate();
}

/* =============================== getter methods for background options ================================= */
CLASS_D_READER_IMPL(Image, QString, imageFileName, imageFileName)
CLASS_D_READER_IMPL(Image, Image::ReferencePoints, points, points)
BASIC_D_READER_IMPL(Image, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(Image, bool, drawPoints, drawPoints)

/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetImageFileName, QString, imageFileName, updateFileName)
void Image::setImageFileName(const QString& fileName) {
    if (fileName!= d->imageFileName)
        exec(new ImageSetImageFileNameCmd(d, fileName, i18n("%1: set image")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetRotationAngle, float, rotationAngle, update)
void Image::setRotationAngle(float angle) {
    if (angle != d->rotationAngle)
        exec(new ImageSetRotationAngleCmd(d, angle, i18n("%1: set rotation angle")));
}

void Image::setPrinting(bool on) const {
    QList<WorksheetElement*> childElements = children<WorksheetElement>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
    foreach(WorksheetElement* elem, childElements)
        elem->setPrinting(on);
}

void Image::setPoints(const Image::ReferencePoints& points) {
    d->points = points;
}

void Image::setDrawPoints(const bool value) {
    d->drawPoints = value;
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

void ImagePrivate::updateFileName(){
    q->removeAllChildren();
    const QString& fileName = imageFileName.trimmed();
    if ( !fileName.isEmpty() ){
        q->imagePixmap.load(fileName);
        QRect rect = q->imagePixmap.rect();
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
    writer->writeAttribute( "fileName", d->imageFileName );
    writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
    writer->writeAttribute( "drawPoints", QString::number(d->drawPoints) );
    writer->writeEndElement();

    //serialize all children
    QList<WorksheetElement *> childElements = children<WorksheetElement>(IncludeHidden);
    foreach(WorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement();
}

//! Load from XML
bool Image::load(XmlStreamReader* reader){
    if(!reader->isStartElement() || reader->name() != "image"){
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
        }else if (reader->name() == "background"){
            attribs = reader->attributes();

            str = attribs.value("fileName").toString();
            d->imageFileName = str;

            str = attribs.value("drawPoints").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'drawPoints'"));
            else
                d->drawPoints = str.toInt();

            str = attribs.value("rotationAngle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("rotationAngle"));
            else
                d->rotationAngle = str.toFloat();
        }else if(reader->name() == "customItem"){
            //change it with
            CustomItem* customItem = new CustomItem("");
            if (!customItem->load(reader)){
                delete customItem;
                return false;
            }else{
                addChild(customItem);
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    d->updateFileName();
    return true;
}
