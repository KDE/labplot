/***************************************************************************
    File                 : DatapickerPointWidget.cpp
    Project              : LabPlot
    Description          : widget for Datapicker-Point properties
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

#include "DatapickerPointWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include "math.h"

#include <QDoubleSpinBox>
#include <QPainter>
#include <QDir>


DatapickerPointWidget::DatapickerPointWidget(QWidget *parent): QWidget(parent) {
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

//     TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::DatapickerPoint);
//     ui.gridLayout->addWidget(templateHandler);
//     templateHandler->show();
//     connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
//     connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfig(KConfig&)));
//     connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void DatapickerPointWidget::init() {
    m_initializing = true;
    GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

    QPainter pa;
    int iconSize = 20;
    QPixmap pm(iconSize, iconSize);
    QPen pen(Qt::SolidPattern, 0);
    ui.cbStyle->setIconSize(QSize(iconSize, iconSize));
    QTransform trafo;
    trafo.scale(15, 15);
    for (int i=0; i<18; ++i) {
        DatapickerPoint::PointsStyle style = (DatapickerPoint::PointsStyle)i;
        pm.fill(Qt::transparent);
        pa.begin(&pm);
        pa.setPen( pen );
        pa.setRenderHint(QPainter::Antialiasing);
        pa.translate(iconSize/2,iconSize/2);
        pa.drawPath(trafo.map(DatapickerPoint::pointPathFromStyle(style)));
        pa.end();
        ui.cbStyle->addItem(QIcon(pm), DatapickerPoint::pointNameFromStyle(style));
    }
    GuiTools::updateBrushStyles(ui.cbFillingStyle, Qt::black);
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, Qt::black);
    m_initializing = false;
}

void DatapickerPointWidget::setDatapickerPoints(QList<DatapickerPoint*> list) {
    if (!list.isEmpty()) {
        this->setEnabled(true);
        m_pointsList = list;
        m_point = list.first();
        this->load();
        initConnections();
    } else {
        this->setEnabled(false);
    }
}

void DatapickerPointWidget::initConnections() {
    connect(m_point, SIGNAL(positionChanged(DatapickerPoint::PositionWrapper)),
             this, SLOT(pointPositionChanged(DatapickerPoint::PositionWrapper)));
    connect(m_point, SIGNAL(styleChanged(DatapickerPoint::PointsStyle)), this, SLOT(pointStyleChanged(DatapickerPoint::PointsStyle)));
    connect(m_point, SIGNAL(sizeChanged(qreal)), this, SLOT(pointSizeChanged(qreal)));
    connect(m_point, SIGNAL(rotationAngleChanged(qreal)), this, SLOT(pointRotationAngleChanged(qreal)));
    connect(m_point, SIGNAL(opacityChanged(qreal)), this, SLOT(pointOpacityChanged(qreal)));
    connect(m_point, SIGNAL(brushChanged(QBrush)), this, SLOT(pointBrushChanged(QBrush)));
    connect(m_point, SIGNAL(penChanged(QPen)), this, SLOT(pointPenChanged(QPen)));
    connect(m_point, SIGNAL(visibleChanged(bool)), this, SLOT(pointVisibleChanged(bool)));
    connect(m_point, SIGNAL(errorBarBrushChanged(QBrush)), this, SLOT(pointErrorBarBrushChanged(QBrush)));
    connect(m_point, SIGNAL(errorBarSizeChanged(qreal)), this, SLOT(pointErrorBarSizeChanged(qreal)));
}

void DatapickerPointWidget::hidePositionWidgets() {
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
    called when point's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void DatapickerPointWidget::positionXChanged(int index) {
    //Enable/disable the spinbox for the x- coordinates if the "custom position" is selected/deselected
    if ( index == ui.cbPositionX->count()-1 ) {
		ui.sbPositionX->setEnabled(true);
    } else {
		ui.sbPositionX->setEnabled(false);
	}

	if (m_initializing)
		return;

    DatapickerPoint::PositionWrapper position = m_point->position();
    position.horizontalPosition = DatapickerPoint::HorizontalPosition(index);
    m_point->beginMacro(i18n("%1 DatapickerPoints: changed", m_pointsList.count()));
    foreach(DatapickerPoint* point, m_pointsList)
        point->setPosition(position);
    m_point->endMacro();
}

/*!
    called when point's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void DatapickerPointWidget::positionYChanged(int index) {
    //Enable/disable the spinbox for the y- oordinates if the "custom position" is selected/deselected
    if ( index == ui.cbPositionY->count()-1 ) {
		ui.sbPositionY->setEnabled(true);
    } else {
		ui.sbPositionY->setEnabled(false);
	}

	if (m_initializing)
		return;

    DatapickerPoint::PositionWrapper position = m_point->position();
    position.verticalPosition = DatapickerPoint::VerticalPosition(index);
        m_point->beginMacro(i18n("%1 DatapickerPoints: changed", m_pointsList.count()));

    foreach(DatapickerPoint* point, m_pointsList)
        point->setPosition(position);
    m_point->endMacro();

}

void DatapickerPointWidget::customPositionXChanged(double value) {
	if (m_initializing)
		return;

    DatapickerPoint::PositionWrapper position = m_point->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
        m_point->beginMacro(i18n("%1 DatapickerPoints: changed", m_pointsList.count()));

    foreach(DatapickerPoint* point, m_pointsList)
        point->setPosition(position);
    m_point->endMacro();
}

void DatapickerPointWidget::customPositionYChanged(double value) {
	if (m_initializing)
		return;

    DatapickerPoint::PositionWrapper position = m_point->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
        m_point->beginMacro(i18n("%1 DatapickerPoints: changed", m_pointsList.count()));

    foreach(DatapickerPoint* point, m_pointsList)
        point->setPosition(position);
    m_point->endMacro();
}

void DatapickerPointWidget::styleChanged(int index) {
  DatapickerPoint::PointsStyle style = DatapickerPoint::PointsStyle(index);
    //enable/disable the  filling options in the GUI depending on the currently selected points.
    if (style!=DatapickerPoint::Line && style!=DatapickerPoint::Cross) {
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

  m_point->beginMacro(i18n("%1 DatapickerPoints: style changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList)
    point->setStyle(style);
  m_point->endMacro();
}

void DatapickerPointWidget::sizeChanged(double value) {
  if (m_initializing)
    return;

  m_point->beginMacro(i18n("%1 DatapickerPoints: size changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList)
    point->setSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  m_point->endMacro();
}

void DatapickerPointWidget::errorBarSizeChanged(double value) {
  if (m_initializing)
    return;

  m_point->beginMacro(i18n("%1 DatapickerPoints: error bar size changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList)
    point->setErrorBarSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  m_point->endMacro();
}

void DatapickerPointWidget::rotationChanged(int value) {
    if (m_initializing)
        return;

    m_point->beginMacro(i18n("%1 DatapickerPoints: rotation changed", m_pointsList.count()));
    foreach(DatapickerPoint* point, m_pointsList)
        point->setRotationAngle(value);
    m_point->endMacro();
}

void DatapickerPointWidget::opacityChanged(int value) {
    if (m_initializing)
        return;

    qreal opacity = (float)value/100.;
    m_point->beginMacro(i18n("%1 DatapickerPoints: opacity changed", m_pointsList.count()));
    foreach(DatapickerPoint* point, m_pointsList)
        point->setOpacity(opacity);
    m_point->endMacro();
}

void DatapickerPointWidget::fillingStyleChanged(int index) {
  Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
  ui.kcbFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

  if (m_initializing)
    return;

  QBrush brush;
  m_point->beginMacro(i18n("%1 DatapickerPoints: filling style changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    brush = point->brush();
    brush.setStyle(brushStyle);
    point->setBrush(brush);
  }
  m_point->endMacro();

}

void DatapickerPointWidget::errorBarFillingStyleChanged(int index) {
  Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
  ui.kcbErrorBarFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

  if (m_initializing)
    return;

  QBrush brush;
  m_point->beginMacro(i18n("%1 DatapickerPoints: Error Bar filling style changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    brush = point->brush();
    brush.setStyle(brushStyle);
    point->setErrorBarBrush(brush);
  }
  m_point->endMacro();

}

void DatapickerPointWidget::fillingColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QBrush brush;
  m_point->beginMacro(i18n("%1 DatapickerPoints: filling color changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    brush = point->brush();
    brush.setColor(color);
    point->setBrush(brush);
  }
  m_point->endMacro();

  m_initializing = true;
  GuiTools::updateBrushStyles(ui.cbFillingStyle, color );
  m_initializing = false;
}

void DatapickerPointWidget::errorBarFillingColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QBrush brush;
  m_point->beginMacro(i18n("%1 DatapickerPoints: Error bar filling color changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    brush = point->errorBarBrush();
    brush.setColor(color);
    point->setErrorBarBrush(brush);
  }
  m_point->endMacro();

  m_initializing = true;
  GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, color );
  m_initializing = false;
}

void DatapickerPointWidget::borderStyleChanged(int index) {
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
  m_point->beginMacro(i18n("%1 DatapickerPoints: border style changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    pen = point->pen();
    pen.setStyle(penStyle);
    point->setPen(pen);
  }
  m_point->endMacro();
}

void DatapickerPointWidget::borderColorChanged(const QColor& color) {
  if (m_initializing)
    return;

  QPen pen;
  m_point->beginMacro(i18n("%1 DatapickerPoints: border color changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    pen = point->pen();
    pen.setColor(color);
    point->setPen(pen);
  }
  m_point->endMacro();

  m_initializing = true;
  GuiTools::updatePenStyles(ui.cbBorderStyle, color);
  m_initializing = false;
}

void DatapickerPointWidget::borderWidthChanged(double value) {
  if (m_initializing)
    return;

  QPen pen;
  m_point->beginMacro(i18n("%1 DatapickerPoints: border width changed", m_pointsList.count()));
  foreach(DatapickerPoint* point, m_pointsList){
    pen = point->pen();
    pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
    point->setPen(pen);
  }
  m_point->endMacro();
}

void DatapickerPointWidget::visibilityChanged(bool state) {
    if (m_initializing)
        return;

    m_point->beginMacro(i18n("%1 DatapickerPoints: visibility changed", m_pointsList.count()));
    foreach(DatapickerPoint* point, m_pointsList)
        point->setVisible(state);
    m_point->endMacro();
}
//*********************************************************
//****** SLOTs for changes triggered in DatapickerPoint *********
//*********************************************************

void DatapickerPointWidget::pointPositionChanged(const DatapickerPoint::PositionWrapper& position) {
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void DatapickerPointWidget::pointStyleChanged(DatapickerPoint::PointsStyle style) {
    m_initializing = true;
    ui.cbStyle->setCurrentIndex((int)style);
    m_initializing = false;
}

void DatapickerPointWidget::pointSizeChanged(qreal size) {
    m_initializing = true;
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
    m_initializing = false;
}

void DatapickerPointWidget::pointErrorBarSizeChanged(qreal size) {
    m_initializing = true;
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
    m_initializing = false;
}

void DatapickerPointWidget::pointRotationAngleChanged(qreal angle) {
    m_initializing = true;
    ui.sbRotation->setValue(round(angle));
    m_initializing = false;
}

void DatapickerPointWidget::pointOpacityChanged(qreal opacity) {
    m_initializing = true;
    ui.sbOpacity->setValue( round(opacity*100.0) );
    m_initializing = false;
}

void DatapickerPointWidget::pointBrushChanged(QBrush brush) {
    m_initializing = true;
    ui.cbFillingStyle->setCurrentIndex((int) brush.style());
    ui.kcbFillingColor->setColor(brush.color());
    GuiTools::updateBrushStyles(ui.cbFillingStyle, brush.color());
    m_initializing = false;
}

void DatapickerPointWidget::pointErrorBarBrushChanged(QBrush brush) {
    m_initializing = true;
    ui.cbErrorBarFillingStyle->setCurrentIndex((int) brush.style());
    ui.kcbErrorBarFillingColor->setColor(brush.color());
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, brush.color());
    m_initializing = false;
}

void DatapickerPointWidget::pointPenChanged(const QPen& pen) {
    m_initializing = true;
    ui.cbBorderStyle->setCurrentIndex( (int) pen.style());
    ui.kcbBorderColor->setColor( pen.color());
    GuiTools::updatePenStyles(ui.cbBorderStyle, pen.color());
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
    m_initializing = false;
}

void DatapickerPointWidget::pointVisibleChanged(bool on) {
    m_initializing = true;
    ui.chbVisible->setChecked(on);
    m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerPointWidget::load() {
    if(m_point == NULL)
		return;

	m_initializing = true;

    ui.cbPositionX->setCurrentIndex( (int) m_point->position().horizontalPosition );
    ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_point->position().point.x(),Worksheet::Centimeter) );
    ui.cbPositionY->setCurrentIndex( (int) m_point->position().verticalPosition );
    ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_point->position().point.y(),Worksheet::Centimeter) );
    ui.cbStyle->setCurrentIndex( (int)m_point->style() );
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(m_point->size(), Worksheet::Point) );
    ui.sbRotation->setValue( m_point->rotationAngle() );
    ui.sbOpacity->setValue( round(m_point->opacity()*100.0) );

    ui.cbFillingStyle->setCurrentIndex( (int) m_point->brush().style() );
    ui.kcbFillingColor->setColor(  m_point->brush().color() );

    ui.cbBorderStyle->setCurrentIndex( (int) m_point->pen().style() );
    ui.kcbBorderColor->setColor( m_point->pen().color() );
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_point->pen().widthF(), Worksheet::Point) );

    ui.chbVisible->setChecked( m_point->isVisible() );

    ui.cbErrorBarFillingStyle->setCurrentIndex( (int) m_point->errorBarBrush().style() );
    ui.kcbErrorBarFillingColor->setColor(  m_point->errorBarBrush().color() );
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(m_point->errorBarSize(), Worksheet::Point) );

    m_initializing = false;
}


void DatapickerPointWidget::loadConfigFromTemplate(KConfig& config) {
    //extract the name of the template from the file name
    QString name;
    int index = config.name().lastIndexOf(QDir::separator());
    if (index!=-1)
        name = config.name().right(config.name().size() - index - 1);
    else
        name = config.name();

    int size = m_pointsList.size();
    if (size>1)
        m_point->beginMacro(i18n("%1 Datapicker-Point: template \"%2\" loaded", size, name));
    else
        m_point->beginMacro(i18n("%1: template \"%2\" loaded", m_point->name(), name));

    this->loadConfig(config);

    m_point->endMacro();
}

void DatapickerPointWidget::loadConfig(KConfig& config) {
    KConfigGroup group = config.group( "DatapickerPoint" );
    ui.cbStyle->setCurrentIndex( group.readEntry("Style", (int)m_point->style()) );
    ui.sbSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("Size", m_point->size()), Worksheet::Point) );
    ui.sbRotation->setValue( group.readEntry("Rotation", m_point->rotationAngle()) );
    ui.sbOpacity->setValue( round(group.readEntry("Opacity", m_point->opacity())*100.0) );
    ui.cbFillingStyle->setCurrentIndex( group.readEntry("FillingStyle", (int) m_point->brush().style()) );
    ui.kcbFillingColor->setColor(  group.readEntry("FillingColor", m_point->brush().color()) );
    ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_point->pen().style()) );
    ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_point->pen().color()) );
    ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth",m_point->pen().widthF()), Worksheet::Point) );
    ui.cbErrorBarFillingStyle->setCurrentIndex( group.readEntry("ErrorBarFillingStyle", (int) m_point->errorBarBrush().style()) );
    ui.kcbErrorBarFillingColor->setColor(  group.readEntry("ErrorBarFillingColor", m_point->errorBarBrush().color()) );
    ui.sbErrorBarSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("ErrorBarSize", m_point->errorBarSize()), Worksheet::Point) );

    m_initializing=true;
    GuiTools::updateBrushStyles(ui.cbFillingStyle, ui.kcbFillingColor->color());
    GuiTools::updateBrushStyles(ui.cbErrorBarFillingStyle, ui.kcbErrorBarFillingColor->color());
    GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
    m_initializing=false;
}

void DatapickerPointWidget::saveConfig(KConfig& config){
    KConfigGroup group = config.group( "DatapickerPoint" );
    group.writeEntry("Style", ui.cbStyle->currentText());
    group.writeEntry("Size", Worksheet::convertToSceneUnits(ui.sbSize->value(),Worksheet::Point));
    group.writeEntry("Rotation", ui.sbRotation->value());
    group.writeEntry("Opacity", ui.sbOpacity->value()/100 );
    group.writeEntry("FillingStyle", ui.cbFillingStyle->currentIndex());
    group.writeEntry("FillingColor", ui.kcbFillingColor->color());
    group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
    group.writeEntry("BorderColor", ui.kcbBorderColor->color());
    group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(),Worksheet::Point));
    group.writeEntry("ErrorBarFillingStyle", ui.cbErrorBarFillingStyle->currentIndex());
    group.writeEntry("ErrorBarFillingColor", ui.kcbErrorBarFillingColor->color());
    group.writeEntry("ErrorBarSize", Worksheet::convertToSceneUnits(ui.sbErrorBarSize->value(),Worksheet::Point));
    config.sync();
}
