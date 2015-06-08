#include "Image.h"
#include "Worksheet.h"
#include "ImagePrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/CustomItem.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>


Image::Image(const QString& name, QString filename):WorksheetElement(name),
    d_ptr(new ImagePrivate(this)), m_filename(filename) {

    init();
    //setImage();
}

void Image::init() {
    Q_D(Image);

    KConfig config;
	KConfigGroup group;
    group = config.group("Image");
    d->rotationAngle = group.readEntry("Rotation", 0.0);
    d->position.horizontalPosition = (HorizontalPosition) group.readEntry("PositionX", (int)Image::hPositionCustom);
    d->position.verticalPosition = (VerticalPosition) group.readEntry("PositionY", (int) Image::vPositionCustom);
    d->position.point.setX( group.readEntry("PositionXValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );
    d->position.point.setY( group.readEntry("PositionYValue", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter)) );

    this->initActions();
}

void Image::initActions() {
    visibilityAction = new QAction(i18n("visible"), this);
    visibilityAction->setCheckable(true);
    connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));

    setReferencePointsAction = new QAction(i18n("Set Reference Point"), this);
    connect(setReferencePointsAction, SIGNAL(triggered()), this, SLOT(setReferencePoints()));

    setCurvePointsAction = new QAction(i18n("Set Curve Points"), this);
    setCurvePointsAction->setEnabled(false);
    connect(setCurvePointsAction, SIGNAL(triggered()), this, SLOT(setCurvePoints()));

    connect(this, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)), this,SLOT(handleAspectRemoved()));
    connect(this, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
}

void Image::handleAspectAdded(const AbstractAspect *aspect) {
    Q_D(Image);
    CustomItem* item = dynamic_cast<CustomItem*>(const_cast<AbstractAspect*>(aspect));
    if (item) {
        item->setParentGraphicsItem(d);
        int itemCount = childCount<CustomItem>(IncludeHidden);
        //enable curve tracing
        if (itemCount == 3) {
            d->selectPoint = false;
            setCurvePointsAction->setEnabled(true);
        }
        //calculate logical position of added aspect should be independent with its origin
        if (itemCount > 3) {
            //update logical value of ref point
            //send item->position().point;
        }
    }
}

void Image::handleAspectRemoved() {
    Q_D(Image);
    //disable curve tracing
    int count = childCount<CustomItem>(IncludeHidden);
    if (count < 3) {
        setCurvePointsAction->setEnabled(false);
        if (count)
            d->selectPoint = true;
    }
}

Image::~Image() {
}

QGraphicsItem* Image::graphicsItem() const{
	return d_ptr;
}

void Image::retransform(){
    Q_D(Image);
	d->retransform();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Image::icon() const{
    return  KIcon("");
}

QMenu* Image::createContextMenu(){
	QMenu *menu = WorksheetElement::createContextMenu();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* firstAction = menu->actions().first();
#else
    QAction* firstAction = menu->actions().at(1);
#endif

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
    menu->insertAction(firstAction, setReferencePointsAction);
    menu->insertAction(firstAction, setCurvePointsAction);
    return menu;
}

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(Image, Image::PositionWrapper, position, position);
CLASS_SHARED_D_READER_IMPL(Image, Image::ReferencePoints, points, points);
BASIC_SHARED_D_READER_IMPL(Image, float, rotationAngle, rotationAngle);

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetPosition, Image::PositionWrapper, position, retransform);
void Image::setPosition(const PositionWrapper& pos) {
    Q_D(Image);
	if (pos.point!=d->position.point || pos.horizontalPosition!=d->position.horizontalPosition || pos.verticalPosition!=d->position.verticalPosition)
        exec(new ImageSetPositionCmd(d, pos, i18n("%1: set position")));
}

/*!
	sets the position without undo/redo-stuff
*/
void Image::setPosition(const QPointF& point) {
    Q_D(Image);
	if (point != d->position.point){
		d->position.point = point;
		retransform();
    }
}

void Image::setLogicalPoints(const Image::ReferencePoints& points) {
    Q_D(Image);
    d->points = points;
}

STD_SETTER_CMD_IMPL_F_S(Image, SetRotationAngle, float, rotationAngle, recalcShapeAndBoundingRect);
void Image::setRotationAngle(float angle) {
    Q_D(Image);
	if (angle != d->rotationAngle)
        exec(new ImageSetRotationAngleCmd(d, angle, i18n("%1: set rotation angle")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL_F(Image, SetVisible, bool, swapVisible, retransform);
void Image::setVisible(bool on) {
    Q_D(Image);
    exec(new ImageSetVisibleCmd(d, on, on ? i18n("%1: set visible") : i18n("%1: set invisible")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Image, SetSelectPoint, bool, swapSelectPoint);
void Image::setSelectPoint(bool on) {
    Q_D(Image);
    exec(new ImageSetSelectPointCmd(d, on, on ? i18n("%1: select point") : i18n("%1: don't select point")));
}

bool Image::isVisible() const {
    Q_D(const Image);
	return d->isVisible();
}

void Image::setPrinting(bool on) {
    Q_D(Image);
	d->m_printing = on;
}

void Image::setImage() {
    Q_D(Image);
    d->graphImage.load(m_filename);
    setPosition(QPointF(d->graphImage.width(),d->graphImage.height()));
}

//SLOTs
void Image::visibilityChanged(){
    Q_D(const Image);
    this->setVisible(!d->isVisible());
}

void Image::setReferencePoints() {
    beginMacro(i18n(""));
    QList<CustomItem*> childElements = children<CustomItem>(AbstractAspect::IncludeHidden);
    foreach(CustomItem* elem, childElements)
            elem->remove();
    setSelectPoint(true);
    endMacro();
}

void Image::setCurvePoints() {
    setSelectPoint(true);
}


// Private
ImagePrivate::ImagePrivate(Image *owner)
        : suppressItemChangeEvent(false),
		  suppressRetransform(false),
		  m_printing(false),
          selectPoint(false),
          q(owner){
    //init defaults
    points.type = Image::Cartesian;

    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

QString ImagePrivate::name() const{
	return q->name();
}

/*!
    calculates the position and the bounding box of the image. Called on geometry.
 */
void ImagePrivate::retransform(){
	if (suppressRetransform)
		return;

    if (position.horizontalPosition != Image::hPositionCustom
        || position.verticalPosition != Image::vPositionCustom)
		updatePosition();

	suppressItemChangeEvent=true;
    setPos(position.point);
	suppressItemChangeEvent=false;

    boundingRectangle.setX(-graphImage.width());
    boundingRectangle.setY(-graphImage.height());
    boundingRectangle.setWidth(graphImage.width()*2);
    boundingRectangle.setHeight(graphImage.height()*2);
    recalcShapeAndBoundingRect();

	emit(q->changed());
}

void ImagePrivate::updatePosition(){
	//determine the parent item
	QRectF parentRect;
    parentRect = scene()->sceneRect();

    if (position.horizontalPosition != Image::hPositionCustom){
        if (position.horizontalPosition == Image::hPositionLeft)
			position.point.setX( parentRect.x() );
        else if (position.horizontalPosition == Image::hPositionCenter)
			position.point.setX( parentRect.x() + parentRect.width()/2 );
        else if (position.horizontalPosition == Image::hPositionRight)
			position.point.setX( parentRect.x() + parentRect.width() );
	}

    if (position.verticalPosition != Image::vPositionCustom){
        if (position.verticalPosition == Image::vPositionTop)
			position.point.setY( parentRect.y() );
        else if (position.verticalPosition == Image::vPositionCenter)
			position.point.setY( parentRect.y() + parentRect.height()/2 );
        else if (position.verticalPosition == Image::vPositionBottom)
			position.point.setY( parentRect.y() + parentRect.height() );
	}

	emit q->positionChanged(position);
}

bool ImagePrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	emit q->changed();
    emit q->visibleChanged(on);
	return oldValue;
}

bool ImagePrivate::swapSelectPoint(bool on){
    bool oldValue = selectPoint;
    selectPoint = on;
    return oldValue;
}

QRectF ImagePrivate::boundingRect() const{
	return transformedBoundingRectangle;
}

QPainterPath ImagePrivate::shape() const{
    return imageShape;
}

void ImagePrivate::recalcShapeAndBoundingRect(){
	prepareGeometryChange();

    QMatrix matrix;
    matrix.rotate(-rotationAngle);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
    imageShape = QPainterPath();
    imageShape.addRect(transformedBoundingRectangle);
    imageShape = matrix.map(imageShape);
}

void ImagePrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();
    painter->rotate(-rotationAngle);
    painter->drawImage(boundingRect(),graphImage);
	painter->restore();

	if (isSelected() && !m_printing){
		painter->setPen(q->selectedPen);
		painter->setOpacity(q->selectedOpacity);
        painter->drawPath(imageShape);
	}
}

QVariant ImagePrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
        Image::PositionWrapper tempPosition;
        tempPosition.point = value.toPointF();
        tempPosition.horizontalPosition = Image::hPositionCustom;
        tempPosition.verticalPosition = Image::vPositionCustom;
		emit q->positionChanged(tempPosition);
     }

	return QGraphicsItem::itemChange(change, value);
}

void ImagePrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (selectPoint) {
        int i = q->childCount<CustomItem>(AbstractAspect::IncludeHidden);
        CustomItem* item = new CustomItem(i18n("item"));
        item->setPosition(event->pos());
        item->setHidden(true);
        q->addChild(item);
        if ( i < 3 ) {
            points.scenePos[i].setX(event->pos().x());
            points.scenePos[i].setY(event->pos().y());
        }
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void ImagePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    q->createContextMenu()->exec(event->screenPos());
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Image::save(QXmlStreamWriter* writer) const{
    Q_D(const Image);

    writer->writeStartElement( "image" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

	//geometry
    writer->writeStartElement( "geometry" );
    writer->writeAttribute( "x", QString::number(d->position.point.x()) );
    writer->writeAttribute( "y", QString::number(d->position.point.y()) );
    writer->writeAttribute( "horizontalPosition", QString::number(d->position.horizontalPosition) );
	writer->writeAttribute( "verticalPosition", QString::number(d->position.verticalPosition) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
    writer->writeEndElement();
    writer->writeEndElement(); // close "Image" section
}

//! Load from XML
bool Image::load(XmlStreamReader* reader){
    Q_D(Image);

    if(!reader->isStartElement() || reader->name() != "image"){
        reader->raiseError(i18n("no Image element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;
    QRectF rect;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "image")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
		}else if (reader->name() == "geometry"){
            attribs = reader->attributes();

            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                d->position.point.setX(str.toDouble());

            str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                d->position.point.setY(str.toDouble());

            str = attribs.value("horizontalPosition").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalPosition'"));
            else
                d->position.horizontalPosition = (Image::HorizontalPosition)str.toInt();

            str = attribs.value("verticalPosition").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPosition'"));
            else
                d->position.verticalPosition = (Image::VerticalPosition)str.toInt();

            str = attribs.value("rotationAngle").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rotationAngle'"));
            else
                d->rotationAngle = str.toInt();

			str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    retransform();
    return true;
}
