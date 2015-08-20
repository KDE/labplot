
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

#include "CustomItemWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include "math.h"

#include <QDoubleSpinBox>
#include <QPainter>
#include <QDir>


CustomItemWidget::CustomItemWidget(QWidget *parent): QWidget(parent) {
    ui.setupUi(this);

    //Positioning
	ui.cbPositionX->addItem(i18n("left"));
    ui.cbPositionX->addItem(i18n("center"));
	ui.cbPositionX->addItem(i18n("right"));
	ui.cbPositionX->addItem(i18n("custom"));

    ui.cbPositionY->addItem(i18n("top"));
    ui.cbPositionY->addItem(i18n("center"));
	ui.cbPositionY->addItem(i18n("bottom"));
    ui.cbPositionY->addItem(i18n("custom"));
	//SLOTS
	// geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );
    connect( ui.cbStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(styleChanged(int)) );
    connect( ui.sbSize, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged(double)) );
    connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );
    connect( ui.sbOpacity, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)) );

    //Filling
    connect( ui.cbFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(fillingStyleChanged(int)) );
    connect( ui.kcbFillingColor, SIGNAL(changed(QColor)), this, SLOT(fillingColorChanged(QColor)) );

    //border
    connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
    connect( ui.kcbBorderColor, SIGNAL(changed(QColor)), this, SLOT(borderColorChanged(QColor)) );
    connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );

    connect( ui.chbVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

    //error bar
    connect( ui.cbErrorBarFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(errorBarFillingStyleChanged(int)) );
    connect( ui.kcbErrorBarFillingColor, SIGNAL(changed(QColor)), this, SLOT(errorBarFillingColorChanged(QColor)) );
    connect( ui.sbErrorBarSize, SIGNAL(valueChanged(double)), this, SLOT(errorBarSizeChanged(double)) );

    init();

//     TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::CustomItem);
//     ui.gridLayout->addWidget(templateHandler);
//     templateHandler->show();
//     connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
//     connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));
//     connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void CustomItemWidget::init() {
    GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

    QPainter pa;
    int iconSize = 20;
    QPixmap pm(iconSize, iconSize);
    QPen pen(Qt::SolidPattern, 0);
    ui.cbStyle->setIconSize(QSize(iconSize, iconSize));
    QTransform trafo;
    trafo.scale(15, 15);
    for (int i=0; i<18; ++i) {
        CustomItem::ItemsStyle style = (CustomItem::ItemsStyle)i;
        pm.fill(Qt::transparent);
        pa.begin(&pm);
        pa.setPen( pen );
        pa.setRenderHint(QPainter::Antialiasing);
        pa.translate(iconSize/2,iconSize/2);
        pa.drawPath(trafo.map(CustomItem::itemsPathFromStyle(style)));
        pa.end();
        ui.cbStyle->addItem(QIcon(pm), CustomItem::itemsNameFromStyle(style));
    }
    GuiTools::updateBrushStyles(ui.cbFillingStyle, Qt::black);
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, Qt::black);
    m_initializing = false;
}

void CustomItemWidget::setCustomItems(QList<CustomItem*> list) {
    if (!list.isEmpty()) {
        this->setEnabled(true);
        m_itemList = list;
        m_item = list.first();
        this->load();
        initConnections();
    } else {
        this->setEnabled(false);
    }
}

void CustomItemWidget::initConnections() {
    connect(m_item, SIGNAL(positionChanged(CustomItem::PositionWrapper)),
             this, SLOT(customItemPositionChanged(CustomItem::PositionWrapper)));
    connect(m_item, SIGNAL(itemsStyleChanged(CustomItem::ItemsStyle)), this, SLOT(customItemStyleChanged(CustomItem::ItemsStyle)));
    connect(m_item, SIGNAL(itemsSizeChanged(qreal)), this, SLOT(customItemSizeChanged(qreal)));
    connect(m_item, SIGNAL(itemsRotationAngleChanged(qreal)), this, SLOT(customItemRotationAngleChanged(qreal)));
    connect(m_item, SIGNAL(itemsOpacityChanged(qreal)), this, SLOT(customItemOpacityChanged(qreal)));
    connect(m_item, SIGNAL(itemsBrushChanged(QBrush)), this, SLOT(customItemBrushChanged(QBrush)));
    connect(m_item, SIGNAL(itemsPenChanged(QPen)), this, SLOT(customItemPenChanged(QPen)));
    connect(m_item, SIGNAL(visibleChanged(bool)), this, SLOT(customItemVisibleChanged(bool)));
    connect(m_item, SIGNAL(errorBarBrushChanged(QBrush)), this, SLOT(customItemErrorBarBrushChanged(QBrush)));
    connect(m_item, SIGNAL(errorBarSizeChanged(qreal)), this, SLOT(customItemErrorBarSizeChanged(qreal)));
}

void CustomItemWidget::hidePositionWidgets() {
    ui.cbPositionX->hide();
    ui.cbPositionY->hide();
    ui.lPositionX->hide();
    ui.lPositionY->hide();
    ui.sbPositionX->hide();
    ui.sbPositionY->hide();
}

//**********************************************************
//****** SLOTs for changes triggered in CustomWidget ********
//**********************************************************
/*!
    called when item's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void CustomItemWidget::positionXChanged(int index) {
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
    if ( index == ui.cbPositionX->count()-1 ) {
		ui.sbPositionX->setEnabled(true);
    } else {
		ui.sbPositionX->setEnabled(false);
	}

	if (m_initializing)
		return;

    CustomItem::PositionWrapper position = m_item->position();
    position.horizontalPosition = CustomItem::HorizontalPosition(index);
    m_item->beginMacro(i18n("%1 CustomItems: changed", m_itemList.count()));
    foreach(CustomItem* item, m_itemList)
        item->setPosition(position);
    m_item->endMacro();
}

/*!
    called when item's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void CustomItemWidget::positionYChanged(int index) {
	//Enable/disable the spinbox for the y- oordinates if the "custom position"-item is selected/deselected
    if ( index == ui.cbPositionY->count()-1 ) {
		ui.sbPositionY->setEnabled(true);
    } else {
		ui.sbPositionY->setEnabled(false);
	}

	if (m_initializing)
		return;

    CustomItem::PositionWrapper position = m_item->position();
    position.verticalPosition = CustomItem::VerticalPosition(index);
        m_item->beginMacro(i18n("%1 CustomItems: changed", m_itemList.count()));

    foreach(CustomItem* item, m_itemList)
        item->setPosition(position);
    m_item->endMacro();

}

void CustomItemWidget::customPositionXChanged(double value) {
	if (m_initializing)
		return;

    CustomItem::PositionWrapper position = m_item->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
        m_item->beginMacro(i18n("%1 CustomItems: changed", m_itemList.count()));

    foreach(CustomItem* item, m_itemList)
        item->setPosition(position);
    m_item->endMacro();
}

void CustomItemWidget::customPositionYChanged(double value) {
	if (m_initializing)
		return;

    CustomItem::PositionWrapper position = m_item->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
        m_item->beginMacro(i18n("%1 CustomItems: changed", m_itemList.count()));

    foreach(CustomItem* item, m_itemList)
        item->setPosition(position);
    m_item->endMacro();
}

void CustomItemWidget::styleChanged(int index) {
  CustomItem::ItemsStyle style = CustomItem::ItemsStyle(index);
    //enable/disable the  filling options in the GUI depending on the currently selected item.
    if (style!=CustomItem::Line && style!=CustomItem::Cross) {
      ui.cbFillingStyle->setEnabled(true);
      bool noBrush = (Qt::BrushStyle(ui.cbFillingStyle->currentIndex())==Qt::NoBrush);
      ui.kcbFillingColor->setEnabled(!noBrush);
    } else {
      ui.kcbFillingColor->setEnabled(false);
      ui.cbFillingStyle->setEnabled(false);
    }

    bool noLine = (Qt::PenStyle(ui.cbBorderStyle->currentIndex())== Qt::NoPen);
    ui.kcbBorderColor->setEnabled(!noLine);
    ui.sbBorderWidth->setEnabled(!noLine);

  if (m_initializing)
    return;

  m_item->beginMacro(i18n("%1 CustomItems: style changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList)
    item->setItemsStyle(style);
  m_item->endMacro();
}

void CustomItemWidget::sizeChanged(double value) {
  if (m_initializing)
    return;

  m_item->beginMacro(i18n("%1 CustomItems: size changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList)
    item->setItemsSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  m_item->endMacro();
}

void CustomItemWidget::errorBarSizeChanged(double value) {
  if (m_initializing)
    return;

  m_item->beginMacro(i18n("%1 CustomItems: error bar size changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList)
    item->setErrorBarSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  m_item->endMacro();
}

void CustomItemWidget::rotationChanged(int value) {
    if (m_initializing)
        return;

    m_item->beginMacro(i18n("%1 CustomItems: rotation changed", m_itemList.count()));
    foreach(CustomItem* item, m_itemList)
        item->setItemsRotationAngle(value);
    m_item->endMacro();
}

void CustomItemWidget::opacityChanged(int value) {
    if (m_initializing)
        return;

    qreal opacity = (float)value/100.;
    m_item->beginMacro(i18n("%1 CustomItems: opacity changed", m_itemList.count()));
    foreach(CustomItem* item, m_itemList)
        item->setItemsOpacity(opacity);
    m_item->endMacro();
}

void CustomItemWidget::fillingStyleChanged(int index) {
  Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
  ui.kcbFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

  if (m_initializing)
    return;

  QBrush brush;
  m_item->beginMacro(i18n("%1 CustomItems: filling style changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    brush = item->itemsBrush();
    brush.setStyle(brushStyle);
    item->setItemsBrush(brush);
  }
  m_item->endMacro();

}

void CustomItemWidget::errorBarFillingStyleChanged(int index) {
  Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
  ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

  if (m_initializing)
    return;

  QBrush brush;
  m_item->beginMacro(i18n("%1 CustomItems: Error Bar filling style changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    brush = item->itemsBrush();
    brush.setStyle(brushStyle);
    item->setErrorBarBrush(brush);
  }
  m_item->endMacro();

}

void CustomItemWidget::fillingColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QBrush brush;
  m_item->beginMacro(i18n("%1 CustomItems: filling color changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    brush=item->itemsBrush();
    brush.setColor(color);
    item->setItemsBrush(brush);
  }
  m_item->endMacro();

  m_initializing = true;
  GuiTools::updateBrushStyles(ui.cbFillingStyle, color );
  m_initializing = false;
}

void CustomItemWidget::errorBarFillingColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QBrush brush;
  m_item->beginMacro(i18n("%1 CustomItems: Error bar filling color changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    brush=item->errorBarBrush();
    brush.setColor(color);
    item->setErrorBarBrush(brush);
  }
  m_item->endMacro();

  m_initializing = true;
  GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, color );
  m_initializing = false;
}

void CustomItemWidget::borderStyleChanged(int index) {
  Qt::PenStyle penStyle=Qt::PenStyle(index);

  if ( penStyle == Qt::NoPen ) {
    ui.kcbBorderColor->setEnabled(false);
    ui.sbBorderWidth->setEnabled(false);
  } else {
    ui.kcbBorderColor->setEnabled(true);
    ui.sbBorderWidth->setEnabled(true);
  }

  if (m_initializing)
    return;

  QPen pen;
  m_item->beginMacro(i18n("%1 CustomItems: border style changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    pen=item->itemsPen();
    pen.setStyle(penStyle);
    item->setItemsPen(pen);
  }
  m_item->endMacro();
}

void CustomItemWidget::borderColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QPen pen;
  m_item->beginMacro(i18n("%1 CustomItems: border color changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    pen=item->itemsPen();
    pen.setColor(color);
    item->setItemsPen(pen);
  }
  m_item->endMacro();

  m_initializing = true;
  GuiTools::updatePenStyles(ui.cbBorderStyle, color);
  m_initializing = false;
}

void CustomItemWidget::borderWidthChanged(double value) {
  if (m_initializing)
    return;

  QPen pen;
  m_item->beginMacro(i18n("%1 CustomItems: border width changed", m_itemList.count()));
  foreach(CustomItem* item, m_itemList){
    pen = item->itemsPen();
    pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
    item->setItemsPen(pen);
  }
  m_item->endMacro();
}

void CustomItemWidget::visibilityChanged(bool state) {
    if (m_initializing)
        return;

    m_item->beginMacro(i18n("%1 CustomItems: visibility changed", m_itemList.count()));
    foreach(CustomItem* item, m_itemList)
        item->setVisible(state);
    m_item->endMacro();
}
//*********************************************************
//****** SLOTs for changes triggered in CustomItem *********
//*********************************************************

void CustomItemWidget::customItemPositionChanged(const CustomItem::PositionWrapper& position) {
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void CustomItemWidget::customItemStyleChanged(CustomItem::ItemsStyle style) {
    m_initializing = true;
    ui.cbStyle->setCurrentIndex((int)style);
    m_initializing = false;
}

void CustomItemWidget::customItemSizeChanged(qreal size) {
    m_initializing = true;
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
    m_initializing = false;
}

void CustomItemWidget::customItemErrorBarSizeChanged(qreal size) {
    m_initializing = true;
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
    m_initializing = false;
}

void CustomItemWidget::customItemRotationAngleChanged(qreal angle) {
    m_initializing = true;
    ui.sbRotation->setValue(round(angle));
    m_initializing = false;
}

void CustomItemWidget::customItemOpacityChanged(qreal opacity) {
    m_initializing = true;
    ui.sbOpacity->setValue( round(opacity*100.0) );
    m_initializing = false;
}

void CustomItemWidget::customItemBrushChanged(QBrush brush) {
    m_initializing = true;
    ui.cbFillingStyle->setCurrentIndex((int) brush.style());
    ui.kcbFillingColor->setColor(brush.color());
    GuiTools::updateBrushStyles(ui.cbFillingStyle, brush.color());
    m_initializing = false;
}

void CustomItemWidget::customItemErrorBarBrushChanged(QBrush brush) {
    m_initializing = true;
    ui.cbErrorBarFillingStyle->setCurrentIndex((int) brush.style());
    ui.kcbErrorBarFillingColor->setColor(brush.color());
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, brush.color());
    m_initializing = false;
}

void CustomItemWidget::customItemPenChanged(const QPen& pen) {
    m_initializing = true;
    ui.cbBorderStyle->setCurrentIndex( (int) pen.style());
    ui.kcbBorderColor->setColor( pen.color());
    GuiTools::updatePenStyles(ui.cbBorderStyle, pen.color());
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
    m_initializing = false;
}

void CustomItemWidget::customItemVisibleChanged(bool on) {
    m_initializing = true;
    ui.chbVisible->setChecked(on);
    m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void CustomItemWidget::load() {
    if(m_item == NULL)
		return;

	m_initializing = true;

    ui.cbPositionX->setCurrentIndex( (int) m_item->position().horizontalPosition );
    ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_item->position().point.x(),Worksheet::Centimeter) );
    ui.cbPositionY->setCurrentIndex( (int) m_item->position().verticalPosition );
    ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_item->position().point.y(),Worksheet::Centimeter) );
    ui.cbStyle->setCurrentIndex( (int)m_item->itemsStyle() );
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(m_item->itemsSize(), Worksheet::Point) );
    ui.sbRotation->setValue( m_item->itemsRotationAngle() );
    ui.sbOpacity->setValue( round(m_item->itemsOpacity()*100.0) );

    ui.cbFillingStyle->setCurrentIndex( (int) m_item->itemsBrush().style() );
    ui.kcbFillingColor->setColor(  m_item->itemsBrush().color() );

    ui.cbBorderStyle->setCurrentIndex( (int) m_item->itemsPen().style() );
    ui.kcbBorderColor->setColor( m_item->itemsPen().color() );
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_item->itemsPen().widthF(), Worksheet::Point) );

    ui.chbVisible->setChecked( m_item->isVisible() );

    ui.cbErrorBarFillingStyle->setCurrentIndex( (int) m_item->errorBarBrush().style() );
    ui.kcbErrorBarFillingColor->setColor(  m_item->errorBarBrush().color() );
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(m_item->errorBarSize(), Worksheet::Point) );

    m_initializing = false;
}


void CustomItemWidget::loadConfigFromTemplate(KConfig& config) {
    //extract the name of the template from the file name
    QString name;
    int index = config.name().lastIndexOf(QDir::separator());
    if (index!=-1)
        name = config.name().right(config.name().size() - index - 1);
    else
        name = config.name();

    int size = m_itemList.size();
    if (size>1)
        m_item->beginMacro(i18n("%1 custom-items: template \"%2\" loaded", size, name));
    else
        m_item->beginMacro(i18n("%1: template \"%2\" loaded", m_item->name(), name));

    this->loadConfig(config);

    m_item->endMacro();
}

void CustomItemWidget::loadConfig(KConfig& config) {
    KConfigGroup group = config.group( "CustomItem" );

    ui.cbStyle->setCurrentIndex( group.readEntry("ItemStyle", (int)m_item->itemsStyle()) );
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ItemSize", m_item->itemsSize()), Worksheet::Point) );
    ui.sbRotation->setValue( group.readEntry("ItemRotation", m_item->itemsRotationAngle()) );
    ui.sbOpacity->setValue( round(group.readEntry("ItemOpacity", m_item->itemsOpacity())*100.0) );
    ui.cbFillingStyle->setCurrentIndex( group.readEntry("ItemFillingStyle", (int) m_item->itemsBrush().style()) );
    ui.kcbFillingColor->setColor(  group.readEntry("ItemFillingColor", m_item->itemsBrush().color()) );
    ui.cbBorderStyle->setCurrentIndex( group.readEntry("ItemBorderStyle", (int) m_item->itemsPen().style()) );
    ui.kcbBorderColor->setColor( group.readEntry("ItemBorderColor", m_item->itemsPen().color()) );
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ItemBorderWidth",m_item->itemsPen().widthF()), Worksheet::Point) );
    ui.cbErrorBarFillingStyle->setCurrentIndex( group.readEntry("ErrorBarFillingStyle", (int) m_item->errorBarBrush().style()) );
    ui.kcbErrorBarFillingColor->setColor(  group.readEntry("ErrorBarFillingColor", m_item->errorBarBrush().color()) );
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarSize", m_item->errorBarSize()), Worksheet::Point) );

    m_initializing=true;
    GuiTools::updateBrushStyles(ui.cbFillingStyle, ui.kcbFillingColor->color());
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, ui.kcbErrorBarFillingColor->color());
    GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
    m_initializing=false;
}

void CustomItemWidget::saveConfig(KConfig& config){
    KConfigGroup group = config.group( "CustomItem" );

    group.writeEntry("ItemStyle", ui.cbStyle->currentText());
    group.writeEntry("ItemSize", Worksheet::convertToSceneUnits(ui.sbSize->value(),Worksheet::Point));
    group.writeEntry("ItemRotation", ui.sbRotation->value());
    group.writeEntry("ItemOpacity", ui.sbOpacity->value()/100 );
    group.writeEntry("ItemFillingStyle", ui.cbFillingStyle->currentIndex());
    group.writeEntry("ItemFillingColor", ui.kcbFillingColor->color());
    group.writeEntry("ItemBorderStyle", ui.cbBorderStyle->currentIndex());
    group.writeEntry("ItemBorderColor", ui.kcbBorderColor->color());
    group.writeEntry("ItemBorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(),Worksheet::Point));
    group.writeEntry("ErrorBarFillingStyle", ui.cbErrorBarFillingStyle->currentIndex());
    group.writeEntry("ErrorBarFillingColor", ui.kcbErrorBarFillingColor->color());
    group.writeEntry("ErrorBarSize", Worksheet::convertToSceneUnits(ui.sbErrorBarSize->value(),Worksheet::Point));

    config.sync();
}
