#include "ImageWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/CustomItem.h"
#include "kdefrontend/widgets/CustomItemWidget.h"

#include <QWidgetAction>
#include <QGridLayout>
#include <QDoubleSpinBox>


ImageWidget::ImageWidget(QWidget *parent): QWidget(parent) {
    ui.setupUi(this);

    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tSymbol);
    customItemWidget = new CustomItemWidget(ui.tSymbol);
    hboxLayout->addWidget(customItemWidget);
    customItemWidget->hidePositionWidgets();
    customItemWidget->setEnabled(false);

    //Positioning
	ui.cbPositionX->addItem(i18n("left"));
    ui.cbPositionX->addItem(i18n("center"));
	ui.cbPositionX->addItem(i18n("right"));
	ui.cbPositionX->addItem(i18n("custom"));


    ui.cbPositionY->addItem(i18n("top"));
    ui.cbPositionY->addItem(i18n("center"));
	ui.cbPositionY->addItem(i18n("bottom"));
    ui.cbPositionY->addItem(i18n("custom"));

    ui.cbGraphType->addItem(i18n("Cartesian (x, y)"));
    ui.cbGraphType->addItem(i18n("Polar (x, y°)"));
    ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), y)"));

    m_itemsList.clear();
	//SLOTS
	// geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );

	connect( ui.chbVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
}

void ImageWidget::setImages(QList<Image*> list){
    m_imagesList = list;
    m_image = list.first();

	this->load();
	initConnections();
}

void ImageWidget::initConnections() {
    connect( m_image, SIGNAL(positionChanged(Image::PositionWrapper)),
             this, SLOT(imagePositionChanged(Image::PositionWrapper)) );
    connect( m_image, SIGNAL(rotationAngleChanged(float)), this, SLOT(imageRotationAngleChanged(float)) );
    connect( m_image, SIGNAL(visibleChanged(bool)), this, SLOT(imageVisibleChanged(bool)) );
    connect( m_image, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)), this,SLOT(handleAspectRemoved()));
    connect( m_image, SIGNAL(aspectAdded(const AbstractAspect*)), this,SLOT(handleAspectAdded()));
    connect( m_image, SIGNAL(updateLogicalPositions()), this, SLOT(updateLogicalPositions()));
}

//**********************************************************
//****** SLOTs for changes triggered in ImageWidget ********
//**********************************************************

// geometry slots

/*!
	called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void ImageWidget::positionXChanged(int index){
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionX->count()-1 ){
		ui.sbPositionX->setEnabled(true);
	}else{
		ui.sbPositionX->setEnabled(false);
	}

	if (m_initializing)
		return;

    Image::PositionWrapper position = m_image->position();
    position.horizontalPosition = Image::HorizontalPosition(index);
    foreach(Image* image, m_imagesList)
        image->setPosition(position);
}

void ImageWidget::updateLogicalPositions() {
    Image::ReferencePoints points = m_image->points();
    points.logicalPos[0].setX(ui.sbPoisitionX1->value());
    points.logicalPos[0].setY(ui.sbPoisitionY1->value());
    points.logicalPos[1].setX(ui.sbPoisitionX2->value());
    points.logicalPos[1].setY(ui.sbPoisitionY2->value());
    points.logicalPos[2].setX(ui.sbPoisitionX3->value());
    points.logicalPos[2].setY(ui.sbPoisitionY3->value());
    points.type = Image::GraphType(ui.cbGraphType->currentIndex());
    m_image->setLogicalPoints(points);
}

/*!
    called when image's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void ImageWidget::positionYChanged(int index){
	//Enable/disable the spinbox for the y- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionY->count()-1 ){
		ui.sbPositionY->setEnabled(true);
	}else{
		ui.sbPositionY->setEnabled(false);
	}

	if (m_initializing)
		return;

    Image::PositionWrapper position = m_image->position();
    position.verticalPosition = Image::VerticalPosition(index);
    foreach(Image* image, m_imagesList)
        image->setPosition(position);
}

void ImageWidget::customPositionXChanged(double value){
	if (m_initializing)
		return;

    Image::PositionWrapper position = m_image->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
    foreach(Image* image, m_imagesList)
        image->setPosition(position);
}

void ImageWidget::customPositionYChanged(double value){
	if (m_initializing)
		return;

    Image::PositionWrapper position = m_image->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
    foreach(Image* image, m_imagesList)
        image->setPosition(position);
}

void ImageWidget::rotationChanged(int value){
	if (m_initializing)
		return;

    foreach(Image* image, m_imagesList)
        image->setRotationAngle(value);
}

void ImageWidget::visibilityChanged(bool state){
	if (m_initializing)
		return;

    foreach(Image* image, m_imagesList)
        image->setVisible(state);
}

//*********************************************************
//****** SLOTs for changes triggered in Image *********
//*********************************************************

void ImageWidget::imagePositionChanged(const Image::PositionWrapper& position){
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void ImageWidget::imageRotationAngleChanged(float angle){
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void ImageWidget::imageVisibleChanged(bool on){
	m_initializing = true;
	ui.chbVisible->setChecked(on);
    m_initializing = false;
}

void ImageWidget::handleAspectRemoved() {
    m_itemsList = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
    if (!m_itemsList.count())
        customItemWidget->setEnabled(false);
    customItemWidget->updateItemList(m_itemsList);
}

void ImageWidget::handleAspectAdded() {
    m_itemsList = m_image->children<CustomItem>(AbstractAspect::IncludeHidden);
    if ( m_itemsList.count() == 1 ) {
        customItemWidget->setEnabled(true);
        customItemWidget->setCustomItems(m_itemsList);
    } else {
        customItemWidget->updateItemList(m_itemsList);
        //set properties of new item
        CustomItem* newItem = m_itemsList.last();
        CustomItem* m_item = m_itemsList.first();
        newItem->setUndoAware(false);
        newItem->setItemsBrush(m_item->itemsBrush());
        newItem->setItemsOpacity(m_item->itemsOpacity());
        newItem->setItemsPen(m_item->itemsPen());
        newItem->setItemsRotationAngle(m_item->itemsRotationAngle());
        newItem->setItemsSize(m_item->itemsSize());
        newItem->setItemsStyle(m_item->itemsStyle());
        newItem->setUndoAware(true);
    }
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ImageWidget::load() {
    if(m_image == NULL)
		return;

	m_initializing = true;

    ui.chbVisible->setChecked( m_image->isVisible() );
	// Geometry
    ui.cbPositionX->setCurrentIndex( (int) m_image->position().horizontalPosition );
    ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_image->position().point.x(),Worksheet::Centimeter) );
    ui.cbPositionY->setCurrentIndex( (int) m_image->position().verticalPosition );
    ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_image->position().point.y(),Worksheet::Centimeter) );
    ui.sbRotation->setValue( m_image->rotationAngle() );
    ui.cbGraphType->setCurrentIndex((int) m_image->points().type);
    ui.sbPoisitionX1->setValue(m_image->points().logicalPos[0].x());
    ui.sbPoisitionY1->setValue(m_image->points().logicalPos[0].y());
    ui.sbPoisitionX2->setValue(m_image->points().logicalPos[1].x());
    ui.sbPoisitionY2->setValue(m_image->points().logicalPos[1].y());
    ui.sbPoisitionX3->setValue(m_image->points().logicalPos[2].x());
    ui.sbPoisitionY3->setValue(m_image->points().logicalPos[2].y());
    m_initializing = false;
}
