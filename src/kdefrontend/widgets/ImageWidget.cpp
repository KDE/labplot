/***************************************************************************
    File                 : ImageWidget.cpp
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include "ImageWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "commonfrontend/widgets/qxtspanslider.h"
#include "kdefrontend/GuiTools.h"
#include "backend/worksheet/Worksheet.h"

#include "math.h"

#include <QPainter>
#include <KUrlCompletion>
#include <QFileDialog>
#include <QDir>


ImageWidget::ImageWidget(QWidget *parent): QWidget(parent) {
	ui.setupUi(this);

	ui.kleFileName->setClearButtonShown(true);
	ui.bOpen->setIcon( KIcon("document-open") );

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleFileName->setCompletionObject(comp);

	QGridLayout* editTabLayout = static_cast<QGridLayout*>(ui.tEdit->layout());
	editTabLayout->setContentsMargins(2,2,2,2);
	editTabLayout->setHorizontalSpacing(2);
	editTabLayout->setVerticalSpacing(4);

	ssIntensity = new QxtSpanSlider(Qt::Horizontal,ui.tEdit);
	ssIntensity->setRange(0, 100);
	editTabLayout->addWidget(ssIntensity, 2, 3);

	ssForeground = new QxtSpanSlider(Qt::Horizontal,ui.tEdit);
	ssForeground->setRange(0, 100);
	editTabLayout->addWidget(ssForeground, 3, 3);

	ssHue = new QxtSpanSlider(Qt::Horizontal,ui.tEdit);
	ssHue->setRange(0, 360);
	editTabLayout->addWidget(ssHue, 4, 3);

	ssSaturation = new QxtSpanSlider(Qt::Horizontal,ui.tEdit);
	ssSaturation->setRange(0,100);
	editTabLayout->addWidget(ssSaturation, 5, 3);

	ssValue = new QxtSpanSlider(Qt::Horizontal,ui.tEdit);
	ssValue->setRange(0,100);
	editTabLayout->addWidget(ssValue, 6, 3);

	ui.cbGraphType->addItem(i18n("Cartesian (x, y)"));
	ui.cbGraphType->addItem(i18n("Polar (x, y°)"));
	ui.cbGraphType->addItem(i18n("Polar (x, y(rad))"));
	ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), y)"));
	ui.cbGraphType->addItem(i18n("Logarithmic (x, ln(y))"));
	ui.cbGraphType->addItem(i18n("Ternary (x, y, z)"));

	ui.lTernaryScale->setHidden(true);
	ui.sbTernaryScale->setHidden(true);
	ui.lPoisitionZ1->setHidden(true);
	ui.lPoisitionZ2->setHidden(true);
	ui.lPoisitionZ3->setHidden(true);
	ui.sbPoisitionZ1->setHidden(true);
	ui.sbPoisitionZ2->setHidden(true);
	ui.sbPoisitionZ3->setHidden(true);

	ui.cbPlotImageType->addItem(i18n("No Image"));
	ui.cbPlotImageType->addItem(i18n("Original Image"));
	ui.cbPlotImageType->addItem(i18n("Processed Image"));

	//SLOTS
	//general
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.kleFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.kleFileName, SIGNAL(clearButtonClicked()), this, SLOT(fileNameChanged()) );

	// edit image
	connect( ui.cbPlotImageType, SIGNAL(currentIndexChanged(int)), this, SLOT(plotImageTypeChanged(int)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(double)), this, SLOT(rotationChanged(double)) );
	connect( ssIntensity, SIGNAL(spanChanged(int,int)), this, SLOT(intensitySpanChanged(int,int)) );
	connect( ssForeground, SIGNAL(spanChanged(int,int)), this, SLOT(foregroundSpanChanged(int,int)) );
	connect( ssHue, SIGNAL(spanChanged(int,int)), this, SLOT(hueSpanChanged(int,int)) );
	connect( ssSaturation, SIGNAL(spanChanged(int,int)), this, SLOT(saturationSpanChanged(int,int)) );
	connect( ssValue, SIGNAL(spanChanged(int,int)), this, SLOT(valueSpanChanged(int,int)) );
	connect( ui.sbMinSegmentLength, SIGNAL(valueChanged(int)), this, SLOT(minSegmentLengthChanged(int)) );
	connect( ui.sbPointSeparation, SIGNAL(valueChanged(int)), this, SLOT(pointSeparationChanged(int)) );

	//axis point
	connect( ui.cbGraphType, SIGNAL(currentIndexChanged(int)), this, SLOT(graphTypeChanged()) );
	connect( ui.sbTernaryScale, SIGNAL(valueChanged(double)), this, SLOT(ternaryScaleChanged(double)) );
	connect( ui.sbPoisitionX1, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionY1, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionX2, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionY2, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionX3, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionY3, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionZ1, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionZ2, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );
	connect( ui.sbPoisitionZ3, SIGNAL(valueChanged(double)), this, SLOT(logicalPositionChanged()) );

    //SYMBOL
    connect( ui.cbSymbolStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(pointsStyleChanged(int)) );
    connect( ui.sbSymbolSize, SIGNAL(valueChanged(double)), this, SLOT(pointsSizeChanged(double)) );
    connect( ui.sbSymbolRotation, SIGNAL(valueChanged(int)), this, SLOT(pointsRotationChanged(int)) );
    connect( ui.sbSymbolOpacity, SIGNAL(valueChanged(int)), this, SLOT(pointsOpacityChanged(int)) );

    //Filling
    connect( ui.cbSymbolFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(pointsFillingStyleChanged(int)) );
    connect( ui.kcbSymbolFillingColor, SIGNAL(changed(QColor)), this, SLOT(pointsFillingColorChanged(QColor)) );

    //border
    connect( ui.cbSymbolBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(pointsBorderStyleChanged(int)) );
    connect( ui.kcbSymbolBorderColor, SIGNAL(changed(QColor)), this, SLOT(pointsBorderColorChanged(QColor)) );
    connect( ui.sbSymbolBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(pointsBorderWidthChanged(double)) );

    connect( ui.chbSymbolVisible, SIGNAL(clicked(bool)), this, SLOT(pointsVisibilityChanged(bool)) );

    init();
}

void ImageWidget::init() {
    m_initializing = true;
    GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);

    QPainter pa;
    int iconSize = 20;
    QPixmap pm(iconSize, iconSize);
    QPen pen(Qt::SolidPattern, 0);
    ui.cbSymbolStyle->setIconSize(QSize(iconSize, iconSize));
    QTransform trafo;
    trafo.scale(15, 15);
    for (int i=1; i<19; ++i) {
        Symbol::Style style = (Symbol::Style)i;
        pm.fill(Qt::transparent);
        pa.begin(&pm);
        pa.setPen( pen );
        pa.setRenderHint(QPainter::Antialiasing);
        pa.translate(iconSize/2,iconSize/2);
        pa.drawPath(trafo.map(Symbol::pathFromStyle(style)));
        pa.end();
        ui.cbSymbolStyle->addItem(QIcon(pm), Symbol::nameFromStyle(style));
    }
    GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);
    m_initializing = false;
}

void ImageWidget::setImages(QList<DatapickerImage*> list) {
	m_imagesList = list;
	m_image = list.first();

	if (list.size()==1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_image->parentAspect()->name());
		ui.leComment->setText(m_image->parentAspect()->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText("");
		ui.leComment->setText("");
	}

	this->load();
	initConnections();
	handleWidgetActions();
    updateSymbolWidgets();
}

void ImageWidget::initConnections() {
	connect( m_image->parentAspect(), SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(imageDescriptionChanged(const AbstractAspect*)));
	connect( m_image, SIGNAL(fileNameChanged(QString)), this, SLOT(imageFileNameChanged(QString)) );
	connect( m_image, SIGNAL(rotationAngleChanged(float)), this, SLOT(imageRotationAngleChanged(float)) );
	connect( m_image, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
             this,SLOT(updateSymbolWidgets()) );
    connect( m_image, SIGNAL(aspectAdded(const AbstractAspect*)), this,SLOT(updateSymbolWidgets()) );
	connect( m_image, SIGNAL(axisPointsChanged(DatapickerImage::ReferencePoints)), this, SLOT(imageAxisPointsChanged(DatapickerImage::ReferencePoints)) );
	connect( m_image, SIGNAL(settingsChanged(DatapickerImage::EditorSettings)), this, SLOT(imageEditorSettingsChanged(DatapickerImage::EditorSettings)) );
	connect( m_image, SIGNAL(minSegmentLengthChanged(int)), this, SLOT(imageMinSegmentLengthChanged(int)) );
    connect( m_image, SIGNAL(pointStyleChanged(Symbol::Style)), this, SLOT(symbolStyleChanged(Symbol::Style)));
    connect( m_image, SIGNAL(pointSizeChanged(qreal)), this, SLOT(symbolSizeChanged(qreal)));
    connect( m_image, SIGNAL(pointRotationAngleChanged(qreal)), this, SLOT(symbolRotationAngleChanged(qreal)));
    connect( m_image, SIGNAL(pointOpacityChanged(qreal)), this, SLOT(symbolOpacityChanged(qreal)));
    connect( m_image, SIGNAL(pointBrushChanged(QBrush)), this, SLOT(symbolBrushChanged(QBrush)) );
    connect( m_image, SIGNAL(pointPenChanged(QPen)), this, SLOT(symbolPenChanged(QPen)) );
    connect( m_image, SIGNAL(pointVisibilityChanged(bool)), this, SLOT(symbolVisibleChanged(bool)) );

}

void ImageWidget::handleWidgetActions() {
	QString fileName =  ui.kleFileName->text().trimmed();
	if (!fileName.isEmpty()) {
		ui.tEdit->setEnabled(true);
		ui.cbGraphType->setEnabled(true);
		ui.sbPoisitionX1->setEnabled(true);
		ui.sbPoisitionX2->setEnabled(true);
		ui.sbPoisitionX3->setEnabled(true);
		ui.sbPoisitionY1->setEnabled(true);
		ui.sbPoisitionY2->setEnabled(true);
		ui.sbPoisitionY3->setEnabled(true);
	} else {
		ui.tEdit->setEnabled(false);
		ui.cbGraphType->setEnabled(false);
		ui.sbPoisitionX1->setEnabled(false);
		ui.sbPoisitionX2->setEnabled(false);
		ui.sbPoisitionX3->setEnabled(false);
		ui.sbPoisitionY1->setEnabled(false);
		ui.sbPoisitionY2->setEnabled(false);
		ui.sbPoisitionY3->setEnabled(false);
	}
}

//**********************************************************
//****** SLOTs for changes triggered in ImageWidget ********
//**********************************************************
//"General"-tab
void ImageWidget::nameChanged() {
	if (m_initializing)
		return;

	m_image->parentAspect()->setName(ui.leName->text());
}

void ImageWidget::commentChanged() {
	if (m_initializing)
		return;

	m_image->parentAspect()->setComment(ui.leComment->text());
}

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

	ui.kleFileName->setText( path );
	handleWidgetActions();

	foreach(DatapickerImage* image, m_imagesList)
		image->setFileName(path);
}

void ImageWidget::fileNameChanged() {
	if (m_initializing)
		return;

	handleWidgetActions();

	QString fileName = ui.kleFileName->text();
	foreach(DatapickerImage* image, m_imagesList) {
		image->setFileName(fileName);
	}
}

void ImageWidget::graphTypeChanged() {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.type = DatapickerImage::GraphType(ui.cbGraphType->currentIndex());

	if (points.type != DatapickerImage::Ternary) {
		ui.lTernaryScale->setHidden(true);
		ui.sbTernaryScale->setHidden(true);
		ui.lPoisitionZ1->setHidden(true);
		ui.lPoisitionZ2->setHidden(true);
		ui.lPoisitionZ3->setHidden(true);
		ui.sbPoisitionZ1->setHidden(true);
		ui.sbPoisitionZ2->setHidden(true);
		ui.sbPoisitionZ3->setHidden(true);
	} else {
		ui.lTernaryScale->setHidden(false);
		ui.sbTernaryScale->setHidden(false);
		ui.lPoisitionZ1->setHidden(false);
		ui.lPoisitionZ2->setHidden(false);
		ui.lPoisitionZ3->setHidden(false);
		ui.sbPoisitionZ1->setHidden(false);
		ui.sbPoisitionZ2->setHidden(false);
		ui.sbPoisitionZ3->setHidden(false);
	}

	foreach(DatapickerImage* image, m_imagesList)
		image->setAxisPoints(points);
}

void ImageWidget::ternaryScaleChanged(double value) {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.ternaryScale = value;

	foreach(DatapickerImage* image, m_imagesList)
		image->setAxisPoints(points);
}

void ImageWidget::logicalPositionChanged() {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.logicalPos[0].setX(ui.sbPoisitionX1->value());
	points.logicalPos[0].setY(ui.sbPoisitionY1->value());
	points.logicalPos[1].setX(ui.sbPoisitionX2->value());
	points.logicalPos[1].setY(ui.sbPoisitionY2->value());
	points.logicalPos[2].setX(ui.sbPoisitionX3->value());
	points.logicalPos[2].setY(ui.sbPoisitionY3->value());
	points.logicalPos[0].setZ(ui.sbPoisitionZ1->value());
	points.logicalPos[1].setZ(ui.sbPoisitionZ2->value());
	points.logicalPos[2].setZ(ui.sbPoisitionZ3->value());

	foreach(DatapickerImage* image, m_imagesList)
		image->setAxisPoints(points);
}

void ImageWidget::pointsStyleChanged(int index) {
    Symbol::Style style = Symbol::Style(index + 1);
    //enable/disable the  filling options in the GUI depending on the currently selected points.
    if (style != Symbol::Line && style != Symbol::Cross) {
        ui.cbSymbolFillingStyle->setEnabled(true);
        bool noBrush = (Qt::BrushStyle(ui.cbSymbolFillingStyle->currentIndex())==Qt::NoBrush);
        ui.kcbSymbolFillingColor->setEnabled(!noBrush);
    } else {
        ui.kcbSymbolFillingColor->setEnabled(false);
        ui.cbSymbolFillingStyle->setEnabled(false);
    }

    bool noLine = (Qt::PenStyle(ui.cbSymbolBorderStyle->currentIndex())== Qt::NoPen);
    ui.kcbSymbolBorderColor->setEnabled(!noLine);
    ui.sbSymbolBorderWidth->setEnabled(!noLine);

    if (m_initializing)
        return;

    foreach(DatapickerImage* image, m_imagesList)
        image->setPointStyle(style);
}

void ImageWidget::pointsSizeChanged(double value) {
    if (m_initializing)
        return;

    foreach(DatapickerImage* image, m_imagesList)
        image->setPointSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void ImageWidget::pointsRotationChanged(int value) {
    if (m_initializing)
        return;

    foreach(DatapickerImage* image, m_imagesList)
        image->setPointRotationAngle(value);
}

void ImageWidget::pointsOpacityChanged(int value) {
    if (m_initializing)
        return;

    qreal opacity = (float)value/100.;
    foreach(DatapickerImage* image, m_imagesList)
        image->setPointOpacity(opacity);
}

void ImageWidget::pointsFillingStyleChanged(int index) {
    Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
    ui.kcbSymbolFillingColor->setEnabled(!(brushStyle==Qt::NoBrush));

    if (m_initializing)
        return;

    QBrush brush;
    foreach(DatapickerImage* image, m_imagesList) {
        brush = image->pointBrush();
        brush.setStyle(brushStyle);
        image->setPointBrush(brush);
    }
}

void ImageWidget::pointsFillingColorChanged(const QColor& color) {
    if (m_initializing)
        return;

    QBrush brush;
    foreach(DatapickerImage* image, m_imagesList) {
        brush = image->pointBrush();
        brush.setColor(color);
        image->setPointBrush(brush);
    }

    m_initializing = true;
    GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color );
    m_initializing = false;
}

void ImageWidget::pointsBorderStyleChanged(int index) {
    Qt::PenStyle penStyle=Qt::PenStyle(index);

    if ( penStyle == Qt::NoPen ) {
        ui.kcbSymbolBorderColor->setEnabled(false);
        ui.sbSymbolBorderWidth->setEnabled(false);
    } else {
        ui.kcbSymbolBorderColor->setEnabled(true);
        ui.sbSymbolBorderWidth->setEnabled(true);
    }

    if (m_initializing)
        return;

    QPen pen;
    foreach(DatapickerImage* image, m_imagesList) {
        pen = image->pointPen();
        pen.setStyle(penStyle);
        image->setPointPen(pen);
    }
}

void ImageWidget::pointsBorderColorChanged(const QColor& color) {
    if (m_initializing)
        return;

    QPen pen;
    foreach(DatapickerImage* image, m_imagesList) {
        pen = image->pointPen();
        pen.setColor(color);
        image->setPointPen(pen);
    }

    m_initializing = true;
    GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
    m_initializing = false;
}

void ImageWidget::pointsBorderWidthChanged(double value) {
    if (m_initializing)
        return;

    QPen pen;
    foreach(DatapickerImage* image, m_imagesList) {
        pen = image->pointPen();
        pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
        image->setPointPen(pen);
    }
}

void ImageWidget::pointsVisibilityChanged(bool state) {
    if (m_initializing)
        return;

    foreach(DatapickerImage* image, m_imagesList)
        image->setPointVisibility(state);
}

void ImageWidget::intensitySpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.type = DatapickerImage::Intensity;
	settings.intensityThresholdHigh = upperLimit;
	settings.intensityThresholdLow = lowerLimit;
	foreach(DatapickerImage* image, m_imagesList)
		image->setSettings(settings);
}

void ImageWidget::foregroundSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.type = DatapickerImage::Foreground;
	settings.foregroundThresholdHigh = upperLimit;
	settings.foregroundThresholdLow = lowerLimit;
	foreach(DatapickerImage* image, m_imagesList)
		image->setSettings(settings);
}

void ImageWidget::hueSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.type = DatapickerImage::Hue;
	settings.hueThresholdHigh = upperLimit;
	settings.hueThresholdLow = lowerLimit;
	foreach(DatapickerImage* image, m_imagesList)
		image->setSettings(settings);
}

void ImageWidget::saturationSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.type = DatapickerImage::Saturation;
	settings.saturationThresholdHigh = upperLimit;
	settings.saturationThresholdLow = lowerLimit;
	foreach(DatapickerImage* image, m_imagesList)
		image->setSettings(settings);
}

void ImageWidget::valueSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.type = DatapickerImage::Value;
	settings.valueThresholdHigh = upperLimit;
	settings.valueThresholdLow = lowerLimit;
	foreach(DatapickerImage* image, m_imagesList)
		image->setSettings(settings);
}

void ImageWidget::plotImageTypeChanged(int index) {
	if (m_initializing)
		return;

	foreach(DatapickerImage* image, m_imagesList)
        image->setPlotImageType(DatapickerImage::PlotImageType(index));
}

void ImageWidget::rotationChanged(double value) {
    if (m_initializing)
        return;

    foreach(DatapickerImage* image, m_imagesList)
        image->setRotationAngle(value);
}

void ImageWidget::minSegmentLengthChanged(int value) {
	if (m_initializing)
		return;

	foreach(DatapickerImage* image, m_imagesList)
		image->setminSegmentLength(value);
}

void ImageWidget::pointSeparationChanged(int value) {
	if (m_initializing)
		return;

	foreach(DatapickerImage* image, m_imagesList)
		image->setPointSeparation(value);
}

//*********************************************************
//******** SLOTs for changes triggered in DatapickerImage ***********
//*********************************************************
/*!
    called when the name or comment of image's parent (datapicker) was changed.
 */
void ImageWidget::imageDescriptionChanged(const AbstractAspect* aspect) {
	if (m_image->parentAspect() != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void ImageWidget::imageFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.kleFileName->setText(name);
	m_initializing = false;
}

void ImageWidget::imageRotationAngleChanged(float angle) {
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void ImageWidget::imageAxisPointsChanged(const DatapickerImage::ReferencePoints& axisPoints) {
	m_initializing = true;
	ui.cbGraphType->setCurrentIndex((int) axisPoints.type);
	ui.sbTernaryScale->setValue(axisPoints.ternaryScale);
	ui.sbPoisitionX1->setValue(axisPoints.logicalPos[0].x());
	ui.sbPoisitionY1->setValue(axisPoints.logicalPos[0].y());
	ui.sbPoisitionX2->setValue(axisPoints.logicalPos[1].x());
	ui.sbPoisitionY2->setValue(axisPoints.logicalPos[1].y());
	ui.sbPoisitionX3->setValue(axisPoints.logicalPos[2].x());
	ui.sbPoisitionY3->setValue(axisPoints.logicalPos[2].y());
	ui.sbPoisitionZ1->setValue(axisPoints.logicalPos[0].z());
	ui.sbPoisitionZ2->setValue(axisPoints.logicalPos[1].z());
	ui.sbPoisitionZ3->setValue(axisPoints.logicalPos[2].z());
	m_initializing = false;
}

void ImageWidget::imageEditorSettingsChanged(const DatapickerImage::EditorSettings& settings) {
	m_initializing = true;
	ssIntensity->setSpan(settings.intensityThresholdLow, settings.intensityThresholdHigh);
	ssForeground->setSpan(m_image->settings().foregroundThresholdLow, settings.foregroundThresholdHigh);
	ssHue->setSpan(settings.hueThresholdLow, settings.hueThresholdHigh);
	ssSaturation->setSpan(settings.saturationThresholdLow, settings.saturationThresholdHigh);
	ssValue->setSpan(settings.valueThresholdLow, settings.valueThresholdHigh);
	m_initializing = false;
}

void ImageWidget::imageMinSegmentLengthChanged(const int value) {
	m_initializing = true;
	ui.sbMinSegmentLength->setValue(value);
	m_initializing = false;
}

void ImageWidget::updateSymbolWidgets() {
    int pointCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
    if (pointCount)
        ui.tSymbol->setEnabled(true);
    else
        ui.tSymbol->setEnabled(false);
}

void ImageWidget::symbolStyleChanged(Symbol::Style style) {
    m_initializing = true;
    ui.cbSymbolStyle->setCurrentIndex((int)style - 1);
    m_initializing = false;
}

void ImageWidget::symbolSizeChanged(qreal size) {
    m_initializing = true;
    ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
    m_initializing = false;
}

void ImageWidget::symbolRotationAngleChanged(qreal angle) {
    m_initializing = true;
    ui.sbSymbolRotation->setValue(round(angle));
    m_initializing = false;
}

void ImageWidget::symbolOpacityChanged(qreal opacity) {
    m_initializing = true;
    ui.sbSymbolOpacity->setValue( round(opacity*100.0) );
    m_initializing = false;
}

void ImageWidget::symbolBrushChanged(QBrush brush) {
    m_initializing = true;
    ui.cbSymbolFillingStyle->setCurrentIndex((int) brush.style());
    ui.kcbSymbolFillingColor->setColor(brush.color());
    GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, brush.color());
    m_initializing = false;
}

void ImageWidget::symbolPenChanged(const QPen& pen) {
    m_initializing = true;
    ui.cbSymbolBorderStyle->setCurrentIndex( (int) pen.style());
    ui.kcbSymbolBorderColor->setColor( pen.color());
    GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, pen.color());
    ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
    m_initializing = false;
}

void ImageWidget::symbolVisibleChanged(bool on) {
    m_initializing = true;
    ui.chbSymbolVisible->setChecked(on);
    m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ImageWidget::load() {
	if(m_image == NULL)
		return;

	m_initializing = true;
	ui.kleFileName->setText( m_image->fileName() );
	ui.cbGraphType->setCurrentIndex((int) m_image->axisPoints().type);
	ui.sbTernaryScale->setValue(m_image->axisPoints().ternaryScale);
	ui.sbPoisitionX1->setValue(m_image->axisPoints().logicalPos[0].x());
	ui.sbPoisitionY1->setValue(m_image->axisPoints().logicalPos[0].y());
	ui.sbPoisitionX2->setValue(m_image->axisPoints().logicalPos[1].x());
	ui.sbPoisitionY2->setValue(m_image->axisPoints().logicalPos[1].y());
	ui.sbPoisitionX3->setValue(m_image->axisPoints().logicalPos[2].x());
	ui.sbPoisitionY3->setValue(m_image->axisPoints().logicalPos[2].y());
	ui.sbPoisitionZ1->setValue(m_image->axisPoints().logicalPos[0].z());
	ui.sbPoisitionZ2->setValue(m_image->axisPoints().logicalPos[1].z());
	ui.sbPoisitionZ3->setValue(m_image->axisPoints().logicalPos[2].z());
	ui.cbPlotImageType->setCurrentIndex((int) m_image->plotImageType());
	ssIntensity->setSpan(m_image->settings().intensityThresholdLow, m_image->settings().intensityThresholdHigh);
	ssForeground->setSpan(m_image->settings().foregroundThresholdLow, m_image->settings().foregroundThresholdHigh);
	ssHue->setSpan(m_image->settings().hueThresholdLow, m_image->settings().hueThresholdHigh);
	ssSaturation->setSpan(m_image->settings().saturationThresholdLow, m_image->settings().saturationThresholdHigh);
	ssValue->setSpan(m_image->settings().valueThresholdLow, m_image->settings().valueThresholdHigh);
	ui.sbPointSeparation->setValue(m_image->pointSeparation());
	ui.sbMinSegmentLength->setValue(m_image->minSegmentLength());
    ui.cbSymbolStyle->setCurrentIndex( (int)m_image->pointStyle() - 1 );
    ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(m_image->pointSize(), Worksheet::Point) );
    ui.sbSymbolRotation->setValue( m_image->pointRotationAngle() );
    ui.sbSymbolOpacity->setValue( round(m_image->pointOpacity()*100.0) );
    ui.cbSymbolFillingStyle->setCurrentIndex( (int) m_image->pointBrush().style() );
    ui.kcbSymbolFillingColor->setColor(  m_image->pointBrush().color() );
    ui.cbSymbolBorderStyle->setCurrentIndex( (int) m_image->pointPen().style() );
    ui.kcbSymbolBorderColor->setColor( m_image->pointPen().color() );
    ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_image->pointPen().widthF(), Worksheet::Point) );
    ui.chbSymbolVisible->setChecked( m_image->pointVisibility() );
	m_initializing = false;
}
