#include "ImageWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/CustomItem.h"
#include "kdefrontend/widgets/CustomItemWidget.h"

#include <QWidgetAction>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <KUrlCompletion>
#include <QFileDialog>
#include <QDir>


ImageWidget::ImageWidget(QWidget *parent): QWidget(parent) {
    ui.setupUi(this);

    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tSymbol);
    customItemWidget = new CustomItemWidget(ui.tSymbol);
    hboxLayout->addWidget(customItemWidget);
    customItemWidget->hidePositionWidgets();
    customItemWidget->setEnabled(false);

    ui.kleBackgroundFileName->setClearButtonShown(true);
    ui.bOpen->setIcon( KIcon("document-open") );

    KUrlCompletion *comp = new KUrlCompletion();
    ui.kleBackgroundFileName->setCompletionObject(comp);

    ui.cbGraphType->addItem(i18n("Cartesian (x, y)"));
    ui.cbGraphType->addItem(i18n("Polar (x, y°)"));
    ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), y)"));

    m_itemsList.clear();
	//SLOTS
	// geometry
    connect( ui.sbRotation, SIGNAL(valueChanged(double)), this, SLOT(rotationChanged(double)) );
    connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
    connect( ui.kleBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
    connect( ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );

}

void ImageWidget::setImages(QList<Image*> list){
    m_imagesList = list;
    m_image = list.first();

	this->load();
	initConnections();
}

void ImageWidget::initConnections() {
    connect( m_image, SIGNAL(rotationAngleChanged(float)), this, SLOT(imageRotationAngleChanged(float)) );
    connect( m_image, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)), this,SLOT(handleAspectRemoved()));
    connect( m_image, SIGNAL(aspectAdded(const AbstractAspect*)), this,SLOT(handleAspectAdded()));
    connect( m_image, SIGNAL(updateLogicalPositions()), this, SLOT(updateLogicalPositions()));
}

//**********************************************************
//****** SLOTs for changes triggered in ImageWidget ********
//**********************************************************
void ImageWidget::selectFile() {
    KConfigGroup conf(KSharedConfig::openConfig(), "ImageWidget");
    QString dir = conf.readEntry("LastImageDir", "");
    QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir);
    if (path.isEmpty())
        return; //cancel was clicked in the file-dialog

    int pos = path.lastIndexOf(QDir::separator());
    if (pos!=-1) {
        QString newDir = path.left(pos);
        if (newDir!=dir)
            conf.writeEntry("LastImageDir", newDir);
    }

    ui.kleBackgroundFileName->setText( path );

    foreach(Image* image, m_imagesList)
        image->setImageFileName(path);
}

void ImageWidget::fileNameChanged(){
    if (m_initializing)
        return;

    QString fileName = ui.kleBackgroundFileName->text();
    foreach(Image* image, m_imagesList){
        image->setImageFileName(fileName);
  }
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
    m_image->setPoints(points);
}

void ImageWidget::rotationChanged(double value){
	if (m_initializing)
		return;

    foreach(Image* image, m_imagesList)
        image->setRotationAngle(value);
}

//*********************************************************
//****** SLOTs for changes triggered in Image *********
//*********************************************************
void ImageWidget::imageFileNameChanged(const QString& name) {
    m_initializing = true;
    ui.kleBackgroundFileName->setText(name);
    m_initializing = false;
}

void ImageWidget::imageRotationAngleChanged(float angle){
	m_initializing = true;
	ui.sbRotation->setValue(angle);
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
    CustomItem* m_item = m_itemsList.first();
    if ( m_itemsList.count() == 1 ) {
        customItemWidget->setEnabled(true);
        m_item->setUndoAware(false);
        m_item->setItemsStyle(CustomItem::Cross);
        m_item->setUndoAware(true);
        customItemWidget->setCustomItems(m_itemsList);
    } else {
        customItemWidget->updateItemList(m_itemsList);
        //set properties of new item
        CustomItem* newItem = m_itemsList.last();
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
	// Geometry
    ui.sbRotation->setValue( m_image->rotationAngle() );
    ui.kleBackgroundFileName->setText( m_image->imageFileName() );
    ui.cbGraphType->setCurrentIndex((int) m_image->points().type);
    ui.sbPoisitionX1->setValue(m_image->points().logicalPos[0].x());
    ui.sbPoisitionY1->setValue(m_image->points().logicalPos[0].y());
    ui.sbPoisitionX2->setValue(m_image->points().logicalPos[1].x());
    ui.sbPoisitionY2->setValue(m_image->points().logicalPos[1].y());
    ui.sbPoisitionX3->setValue(m_image->points().logicalPos[2].x());
    ui.sbPoisitionY3->setValue(m_image->points().logicalPos[2].y());
    m_initializing = false;
}
