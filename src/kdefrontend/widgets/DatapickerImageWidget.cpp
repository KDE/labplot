/***************************************************************************
    File                 : DatapickerImageWidget.cpp
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015-2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2019 by Alexander Semke (alexander.semke@web.de)

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

#include "DatapickerImageWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "commonfrontend/widgets/qxtspanslider.h"
#include "kdefrontend/GuiTools.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/datapicker/ImageEditor.h"

#include <QCompleter>
#include <QDir>
#include <QFileDialog>
#include <QDirModel>
#include <QGraphicsScene>
#include <QImageReader>
#include <QPainter>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <cmath>

HistogramView::HistogramView(QWidget* parent, int range) : QGraphicsView(parent),
	m_scene(new QGraphicsScene()),
	m_range(range) {

	setTransform(QTransform());
	QRectF pageRect( 0, 0, 1000, 100 );
	m_scene->setSceneRect(pageRect);
	setScene(m_scene);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_lowerSlider = new QGraphicsRectItem(pageRect, nullptr);
	m_lowerSlider->setPen(QPen(Qt::black, 0.5));
	m_lowerSlider->setBrush(Qt::blue);
	m_lowerSlider->setOpacity(0.2);
	m_scene->addItem(m_lowerSlider);

	m_upperSlider = new QGraphicsRectItem(pageRect, nullptr);
	m_upperSlider->setPen(QPen(Qt::black, 0.5));
	m_upperSlider->setBrush(Qt::blue);
	m_upperSlider->setOpacity(0.2);
	m_scene->addItem(m_upperSlider);
}

void HistogramView::setScalePixmap(const QString& file) {
	// scene rect is 1000*100 where upper 1000*80 is for histogram graph
	// and lower 1000*20 is for histogram scale
	QGraphicsPixmapItem* pixmap = new QGraphicsPixmapItem(QPixmap(file).scaled( 1000, 20, Qt::IgnoreAspectRatio), nullptr);
	pixmap->setZValue(-1);
	pixmap->setPos(0, 90);
	m_scene->addItem(pixmap);
}

void HistogramView::setSpan(int l, int h) {
	l = l*1000/m_range;
	h = h*1000/m_range;
	m_lowerSlider->setPos(QPointF(l - 1000, 0));
	m_upperSlider->setPos(QPointF(h, 0));
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void HistogramView::resizeEvent(QResizeEvent *event) {
	fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
	QGraphicsView::resizeEvent(event);
}

void HistogramView::drawBackground(QPainter* painter, const QRectF& rect) {
	if (!bins)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	int max = 1;
	for (int i = 0; i <= m_range; i++)
		if (bins [i] > max)
			max = bins [i];

	// convert y-scale count to log scale so small counts are still visible
	// scene rect is 1000*100 where upper 1000*80 is for histogram graph
	// and lower 1000*20 is for histogram scale
	QPainterPath path(QPointF(0, (log(bins[0])*100/log(max))));
	for (int i = 1; i <= m_range; i++) {
		int x = i*1000/m_range;
		int y = 80;
		if ( bins[i] > 1 )
			y = 80 - (log(bins[i])*80/log(max));

		path.lineTo(QPointF(x, y));
	}

	painter->drawPath(path);
	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();
}

DatapickerImageWidget::DatapickerImageWidget(QWidget* parent) : BaseDock(parent), m_image(nullptr) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	ui.leFileName->setClearButtonEnabled(true);
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));

	auto* editTabLayout = static_cast<QGridLayout*>(ui.tEdit->layout());
	editTabLayout->setContentsMargins(2,2,2,2);
	editTabLayout->setHorizontalSpacing(2);
	editTabLayout->setVerticalSpacing(4);

	ssHue = new QxtSpanSlider(Qt::Horizontal, ui.tEdit);
	ssHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	ssHue->setRange(0, 360);
	editTabLayout->addWidget(ssHue, 3, 2);

	ssSaturation = new QxtSpanSlider(Qt::Horizontal, ui.tEdit);
	ssSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	ssSaturation->setRange(0,100);
	editTabLayout->addWidget(ssSaturation, 5, 2);

	ssValue = new QxtSpanSlider(Qt::Horizontal, ui.tEdit);
	ssValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	ssValue->setRange(0,100);
	editTabLayout->addWidget(ssValue, 7, 2);

	ssIntensity = new QxtSpanSlider(Qt::Horizontal, ui.tEdit);
	ssIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	ssIntensity->setRange(0, 100);
	editTabLayout->addWidget(ssIntensity, 9, 2);

	ssForeground = new QxtSpanSlider(Qt::Horizontal, ui.tEdit);
	ssForeground->setToolTip(i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
	ssForeground->setRange(0, 100);
	editTabLayout->addWidget(ssForeground, 11, 2);

	ui.cbGraphType->addItem(i18n("Cartesian (x, y)"));
	ui.cbGraphType->addItem(i18n("Polar (x, y°)"));
	ui.cbGraphType->addItem(i18n("Polar (x, y(rad))"));
	ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), y)"));
	ui.cbGraphType->addItem(i18n("Logarithmic (x, ln(y))"));
	ui.cbGraphType->addItem(i18n("Ternary (x, y, z)"));

	ui.lTernaryScale->setHidden(true);
	ui.sbTernaryScale->setHidden(true);
	ui.lPositionZ1->setHidden(true);
	ui.lPositionZ2->setHidden(true);
	ui.lPositionZ3->setHidden(true);
	ui.sbPositionZ1->setHidden(true);
	ui.sbPositionZ2->setHidden(true);
	ui.sbPositionZ3->setHidden(true);

	ui.cbPlotImageType->addItem(i18n("No Image"));
	ui.cbPlotImageType->addItem(i18n("Original Image"));
	ui.cbPlotImageType->addItem(i18n("Processed Image"));

	QString valueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/colorchooser/colorchooser_value.xpm");
	QString hueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/colorchooser/colorchooser_hue.xpm");
	QString saturationFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/colorchooser/colorchooser_saturation.xpm");

	gvHue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::Hue));
	gvHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvHue, 2, 2);
	gvHue->setScalePixmap(hueFile);

	gvSaturation = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::Saturation));
	gvSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvSaturation, 4, 2);
	gvSaturation->setScalePixmap(saturationFile);

	gvValue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::Value));
	gvValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvValue, 6,2);
	gvValue->setScalePixmap(valueFile);

	gvIntensity = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::Intensity));
	gvIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvIntensity, 8, 2);
	gvIntensity->setScalePixmap(valueFile);

	gvForeground = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::Foreground));
	gvForeground->setToolTip(i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvForeground, 10, 2);
	gvForeground->setScalePixmap(valueFile);

	//SLOTS
	//general
	connect(ui.leName, &QLineEdit::textChanged, this, &DatapickerImageWidget::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &DatapickerImageWidget::commentChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &DatapickerImageWidget::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &DatapickerImageWidget::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &DatapickerImageWidget::fileNameChanged);

	// edit image
	connect(ui.cbPlotImageType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerImageWidget::plotImageTypeChanged);
	connect(ui.sbRotation, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::rotationChanged);
	connect(ssIntensity, &QxtSpanSlider::spanChanged, this, &DatapickerImageWidget::intensitySpanChanged);
	connect(ssIntensity, &QxtSpanSlider::spanChanged, gvIntensity, &HistogramView::setSpan);
	connect(ssForeground, &QxtSpanSlider::spanChanged, this, &DatapickerImageWidget::foregroundSpanChanged);
	connect(ssForeground, &QxtSpanSlider::spanChanged, gvForeground, &HistogramView::setSpan );
	connect(ssHue, &QxtSpanSlider::spanChanged, this, &DatapickerImageWidget::hueSpanChanged);
	connect(ssHue, &QxtSpanSlider::spanChanged, gvHue, &HistogramView::setSpan );
	connect(ssSaturation, &QxtSpanSlider::spanChanged, this, &DatapickerImageWidget::saturationSpanChanged);
	connect(ssSaturation, &QxtSpanSlider::spanChanged, gvSaturation, &HistogramView::setSpan );
	connect(ssValue, &QxtSpanSlider::spanChanged, this, &DatapickerImageWidget::valueSpanChanged);
	connect(ssValue, &QxtSpanSlider::spanChanged, gvValue, &HistogramView::setSpan );
	connect(ui.sbMinSegmentLength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &DatapickerImageWidget::minSegmentLengthChanged);
	connect(ui.sbPointSeparation, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &DatapickerImageWidget::pointSeparationChanged);

	//axis point
	connect(ui.cbGraphType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerImageWidget::graphTypeChanged);
	connect(ui.sbTernaryScale, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::ternaryScaleChanged);
	connect(ui.sbPositionX1, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY1, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionX2, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY2, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionX3, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY3, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ1, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ2, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ3, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::logicalPositionChanged);

	//SYMBOL
	connect(ui.cbSymbolStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerImageWidget::pointsStyleChanged);
	connect(ui.sbSymbolSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::pointsSizeChanged);
	connect(ui.sbSymbolRotation,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &DatapickerImageWidget::pointsRotationChanged);
	connect(ui.sbSymbolOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &DatapickerImageWidget::pointsOpacityChanged);

	//Filling
	connect(ui.cbSymbolFillingStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerImageWidget::pointsFillingStyleChanged);
	connect(ui.kcbSymbolFillingColor, &KColorButton::changed, this, &DatapickerImageWidget::pointsFillingColorChanged);

	//border
	connect(ui.cbSymbolBorderStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &DatapickerImageWidget::pointsBorderStyleChanged);
	connect(ui.kcbSymbolBorderColor, &KColorButton::changed, this, &DatapickerImageWidget::pointsBorderColorChanged);
	connect(ui.sbSymbolBorderWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &DatapickerImageWidget::pointsBorderWidthChanged);

	connect(ui.chbSymbolVisible, &QCheckBox::clicked, this, &DatapickerImageWidget::pointsVisibilityChanged);

	init();
}

void DatapickerImageWidget::init() {
	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);

	QPainter pa;
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	QPen pen(Qt::SolidPattern, 0);
	ui.cbSymbolStyle->setIconSize(QSize(iconSize, iconSize));
	QTransform trafo;
	trafo.scale(15, 15);
	//TODO: constant?
	for (int i = 1; i < 19; ++i) {
		auto style = (Symbol::Style)i;
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

void DatapickerImageWidget::setImages(QList<DatapickerImage*> list) {
	m_initializing = true;
	m_imagesList = list;
	m_image = list.first();
	m_aspect = list.first();

	if (list.size() == 1) {
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
		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	this->load();
	initConnections();
	handleWidgetActions();
	updateSymbolWidgets();
	m_initializing = false;
}

void DatapickerImageWidget::initConnections() {
	connect(m_image->parentAspect(), &AbstractAspect::aspectDescriptionChanged, this, &DatapickerImageWidget::imageDescriptionChanged);
	connect(m_image, &DatapickerImage::fileNameChanged, this, &DatapickerImageWidget::imageFileNameChanged);
	connect(m_image, &DatapickerImage::rotationAngleChanged, this, &DatapickerImageWidget::imageRotationAngleChanged);
	connect(m_image, &AbstractAspect::aspectRemoved, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &AbstractAspect::aspectAdded, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &DatapickerImage::axisPointsChanged, this, &DatapickerImageWidget::imageAxisPointsChanged);
	connect(m_image, &DatapickerImage::settingsChanged, this, &DatapickerImageWidget::imageEditorSettingsChanged);
	connect(m_image, &DatapickerImage::minSegmentLengthChanged, this, &DatapickerImageWidget::imageMinSegmentLengthChanged);
	connect(m_image, &DatapickerImage::pointStyleChanged, this, &DatapickerImageWidget::symbolStyleChanged);
	connect(m_image, &DatapickerImage::pointSizeChanged, this, &DatapickerImageWidget::symbolSizeChanged);
	connect(m_image, &DatapickerImage::pointRotationAngleChanged, this, &DatapickerImageWidget::symbolRotationAngleChanged);
	connect(m_image, &DatapickerImage::pointOpacityChanged, this, &DatapickerImageWidget::symbolOpacityChanged);
	connect(m_image, &DatapickerImage::pointBrushChanged, this, &DatapickerImageWidget::symbolBrushChanged);
	connect(m_image, &DatapickerImage::pointPenChanged, this, &DatapickerImageWidget::symbolPenChanged);
	connect(m_image, &DatapickerImage::pointVisibilityChanged, this, &DatapickerImageWidget::symbolVisibleChanged);
}

void DatapickerImageWidget::handleWidgetActions() {
	QString fileName =  ui.leFileName->text().trimmed();
	bool b = !fileName.isEmpty();
	ui.tEdit->setEnabled(b);
	ui.cbGraphType->setEnabled(b);
	ui.sbRotation->setEnabled(b);
	ui.sbPositionX1->setEnabled(b);
	ui.sbPositionX2->setEnabled(b);
	ui.sbPositionX3->setEnabled(b);
	ui.sbPositionY1->setEnabled(b);
	ui.sbPositionY2->setEnabled(b);
	ui.sbPositionY3->setEnabled(b);
	ui.sbMinSegmentLength->setEnabled(b);
	ui.sbPointSeparation->setEnabled(b);

	if (b) {
		//upload histogram to view
		gvIntensity->bins = m_image->intensityBins;
		gvForeground->bins = m_image->foregroundBins;
		gvHue->bins = m_image->hueBins;
		gvSaturation->bins = m_image->saturationBins;
		gvValue->bins = m_image->valueBins;
	}
}

//**********************************************************
//****** SLOTs for changes triggered in DatapickerImageWidget ********
//**********************************************************
//"General"-tab
void DatapickerImageWidget::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "DatapickerImageWidget");
	QString dir = conf.readEntry("LastImageDir", "");
	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		formats.isEmpty() ? formats += f : formats += " " + f;
	}
	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leFileName->setText( path );
	handleWidgetActions();

	for (auto* image : m_imagesList)
		image->setFileName(path);
}

void DatapickerImageWidget::fileNameChanged() {
	if (m_initializing)
		return;

	handleWidgetActions();

	QString fileName = ui.leFileName->text();
	if (!fileName.isEmpty() && !QFile::exists(fileName))
		ui.leFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leFileName->setStyleSheet(QString());

	for (auto* image : m_imagesList)
		image->setFileName(fileName);
}

void DatapickerImageWidget::graphTypeChanged() {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.type = DatapickerImage::GraphType(ui.cbGraphType->currentIndex());

	if (points.type != DatapickerImage::Ternary) {
		ui.lTernaryScale->setHidden(true);
		ui.sbTernaryScale->setHidden(true);
		ui.lPositionZ1->setHidden(true);
		ui.lPositionZ2->setHidden(true);
		ui.lPositionZ3->setHidden(true);
		ui.sbPositionZ1->setHidden(true);
		ui.sbPositionZ2->setHidden(true);
		ui.sbPositionZ3->setHidden(true);
	} else {
		ui.lTernaryScale->setHidden(false);
		ui.sbTernaryScale->setHidden(false);
		ui.lPositionZ1->setHidden(false);
		ui.lPositionZ2->setHidden(false);
		ui.lPositionZ3->setHidden(false);
		ui.sbPositionZ1->setHidden(false);
		ui.sbPositionZ2->setHidden(false);
		ui.sbPositionZ3->setHidden(false);
	}

	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::ternaryScaleChanged(double value) {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.ternaryScale = value;

	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::logicalPositionChanged() {
	if (m_initializing)
		return;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.logicalPos[0].setX(ui.sbPositionX1->value());
	points.logicalPos[0].setY(ui.sbPositionY1->value());
	points.logicalPos[1].setX(ui.sbPositionX2->value());
	points.logicalPos[1].setY(ui.sbPositionY2->value());
	points.logicalPos[2].setX(ui.sbPositionX3->value());
	points.logicalPos[2].setY(ui.sbPositionY3->value());
	points.logicalPos[0].setZ(ui.sbPositionZ1->value());
	points.logicalPos[1].setZ(ui.sbPositionZ2->value());
	points.logicalPos[2].setZ(ui.sbPositionZ3->value());

	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::pointsStyleChanged(int index) {
	auto style = Symbol::Style(index + 1);
	//enable/disable the  filling options in the GUI depending on the currently selected points.
	if (style != Symbol::Line && style != Symbol::Cross) {
		ui.cbSymbolFillingStyle->setEnabled(true);
		bool noBrush = (Qt::BrushStyle(ui.cbSymbolFillingStyle->currentIndex()) == Qt::NoBrush);
		ui.kcbSymbolFillingColor->setEnabled(!noBrush);
	} else {
		ui.kcbSymbolFillingColor->setEnabled(false);
		ui.cbSymbolFillingStyle->setEnabled(false);
	}

	bool noLine = (Qt::PenStyle(ui.cbSymbolBorderStyle->currentIndex()) == Qt::NoPen);
	ui.kcbSymbolBorderColor->setEnabled(!noLine);
	ui.sbSymbolBorderWidth->setEnabled(!noLine);

	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPointStyle(style);
}

void DatapickerImageWidget::pointsSizeChanged(double value) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPointSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void DatapickerImageWidget::pointsRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPointRotationAngle(value);
}

void DatapickerImageWidget::pointsOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* image : m_imagesList)
		image->setPointOpacity(opacity);
}

void DatapickerImageWidget::pointsFillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbSymbolFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* image : m_imagesList) {
		brush = image->pointBrush();
		brush.setStyle(brushStyle);
		image->setPointBrush(brush);
	}
}

void DatapickerImageWidget::pointsFillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* image : m_imagesList) {
		brush = image->pointBrush();
		brush.setColor(color);
		image->setPointBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color );
	m_initializing = false;
}

void DatapickerImageWidget::pointsBorderStyleChanged(int index) {
	auto penStyle = Qt::PenStyle(index);

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
	for (auto* image : m_imagesList) {
		pen = image->pointPen();
		pen.setStyle(penStyle);
		image->setPointPen(pen);
	}
}

void DatapickerImageWidget::pointsBorderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* image : m_imagesList) {
		pen = image->pointPen();
		pen.setColor(color);
		image->setPointPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
	m_initializing = false;
}

void DatapickerImageWidget::pointsBorderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* image : m_imagesList) {
		pen = image->pointPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
		image->setPointPen(pen);
	}
}

void DatapickerImageWidget::pointsVisibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPointVisibility(state);
}

void DatapickerImageWidget::intensitySpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.intensityThresholdHigh = upperLimit;
	settings.intensityThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::foregroundSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.foregroundThresholdHigh = upperLimit;
	settings.foregroundThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::hueSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.hueThresholdHigh = upperLimit;
	settings.hueThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::saturationSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.saturationThresholdHigh = upperLimit;
	settings.saturationThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::valueSpanChanged(int lowerLimit, int upperLimit) {
	if (m_initializing)
		return;

	DatapickerImage::EditorSettings settings = m_image->settings();
	settings.valueThresholdHigh = upperLimit;
	settings.valueThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::plotImageTypeChanged(int index) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPlotImageType(DatapickerImage::PlotImageType(index));
}

void DatapickerImageWidget::rotationChanged(double value) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setRotationAngle(value);
}

void DatapickerImageWidget::minSegmentLengthChanged(int value) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setminSegmentLength(value);
}

void DatapickerImageWidget::pointSeparationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* image : m_imagesList)
		image->setPointSeparation(value);
}

//*******************************************************************
//******** SLOTs for changes triggered in DatapickerImage ***********
//*******************************************************************
/*!
 * called when the name or comment of image's parent (datapicker) was changed.
 */
void DatapickerImageWidget::imageDescriptionChanged(const AbstractAspect* aspect) {
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

void DatapickerImageWidget::imageFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.leFileName->setText(name);
	m_initializing = false;
}

void DatapickerImageWidget::imageRotationAngleChanged(float angle) {
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void DatapickerImageWidget::imageAxisPointsChanged(const DatapickerImage::ReferencePoints& axisPoints) {
	m_initializing = true;
	ui.cbGraphType->setCurrentIndex((int) axisPoints.type);
	ui.sbTernaryScale->setValue(axisPoints.ternaryScale);
	ui.sbPositionX1->setValue(axisPoints.logicalPos[0].x());
	ui.sbPositionY1->setValue(axisPoints.logicalPos[0].y());
	ui.sbPositionX2->setValue(axisPoints.logicalPos[1].x());
	ui.sbPositionY2->setValue(axisPoints.logicalPos[1].y());
	ui.sbPositionX3->setValue(axisPoints.logicalPos[2].x());
	ui.sbPositionY3->setValue(axisPoints.logicalPos[2].y());
	ui.sbPositionZ1->setValue(axisPoints.logicalPos[0].z());
	ui.sbPositionZ2->setValue(axisPoints.logicalPos[1].z());
	ui.sbPositionZ3->setValue(axisPoints.logicalPos[2].z());
	m_initializing = false;
}

void DatapickerImageWidget::imageEditorSettingsChanged(const DatapickerImage::EditorSettings& settings) {
	m_initializing = true;
	ssIntensity->setSpan(settings.intensityThresholdLow, settings.intensityThresholdHigh);
	ssForeground->setSpan(settings.foregroundThresholdLow, settings.foregroundThresholdHigh);
	ssHue->setSpan(settings.hueThresholdLow, settings.hueThresholdHigh);
	ssSaturation->setSpan(settings.saturationThresholdLow, settings.saturationThresholdHigh);
	ssValue->setSpan(settings.valueThresholdLow, settings.valueThresholdHigh);
	gvIntensity->setSpan(settings.intensityThresholdLow, settings.intensityThresholdHigh);
	gvForeground->setSpan(settings.foregroundThresholdLow, settings.foregroundThresholdHigh);
	gvHue->setSpan(settings.hueThresholdLow, settings.hueThresholdHigh);
	gvSaturation->setSpan(settings.saturationThresholdLow, settings.saturationThresholdHigh);
	gvValue->setSpan(settings.valueThresholdLow, settings.valueThresholdHigh);
	m_initializing = false;
}

void DatapickerImageWidget::imageMinSegmentLengthChanged(const int value) {
	m_initializing = true;
	ui.sbMinSegmentLength->setValue(value);
	m_initializing = false;
}

void DatapickerImageWidget::updateSymbolWidgets() {
	int pointCount = m_image->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
	if (pointCount)
		ui.tSymbol->setEnabled(true);
	else
		ui.tSymbol->setEnabled(false);
}

void DatapickerImageWidget::symbolStyleChanged(Symbol::Style style) {
	m_initializing = true;
	ui.cbSymbolStyle->setCurrentIndex((int)style - 1);
	m_initializing = false;
}

void DatapickerImageWidget::symbolSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Point) );
	m_initializing = false;
}

void DatapickerImageWidget::symbolRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbSymbolRotation->setValue(round(angle));
	m_initializing = false;
}

void DatapickerImageWidget::symbolOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbSymbolOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void DatapickerImageWidget::symbolBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbSymbolFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbSymbolFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, brush.color());
	m_initializing = false;
}

void DatapickerImageWidget::symbolPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbSymbolBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbSymbolBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, pen.color());
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Point));
	m_initializing = false;
}

void DatapickerImageWidget::symbolVisibleChanged(bool on) {
	m_initializing = true;
	ui.chbSymbolVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerImageWidget::load() {
	if (!m_image)
		return;

	m_initializing = true;
	ui.leFileName->setText( m_image->fileName() );

	//highlight the text field for the background image red if an image is used and cannot be found
	if (!m_image->fileName().isEmpty() && !QFile::exists(m_image->fileName()))
		ui.leFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leFileName->setStyleSheet(QString());

	ui.cbGraphType->setCurrentIndex((int) m_image->axisPoints().type);
	ui.sbTernaryScale->setValue(m_image->axisPoints().ternaryScale);
	ui.sbPositionX1->setValue(m_image->axisPoints().logicalPos[0].x());
	ui.sbPositionY1->setValue(m_image->axisPoints().logicalPos[0].y());
	ui.sbPositionX2->setValue(m_image->axisPoints().logicalPos[1].x());
	ui.sbPositionY2->setValue(m_image->axisPoints().logicalPos[1].y());
	ui.sbPositionX3->setValue(m_image->axisPoints().logicalPos[2].x());
	ui.sbPositionY3->setValue(m_image->axisPoints().logicalPos[2].y());
	ui.sbPositionZ1->setValue(m_image->axisPoints().logicalPos[0].z());
	ui.sbPositionZ2->setValue(m_image->axisPoints().logicalPos[1].z());
	ui.sbPositionZ3->setValue(m_image->axisPoints().logicalPos[2].z());
	ui.cbPlotImageType->setCurrentIndex((int) m_image->plotImageType());
	ssIntensity->setSpan(m_image->settings().intensityThresholdLow, m_image->settings().intensityThresholdHigh);
	ssForeground->setSpan(m_image->settings().foregroundThresholdLow, m_image->settings().foregroundThresholdHigh);
	ssHue->setSpan(m_image->settings().hueThresholdLow, m_image->settings().hueThresholdHigh);
	ssSaturation->setSpan(m_image->settings().saturationThresholdLow, m_image->settings().saturationThresholdHigh);
	ssValue->setSpan(m_image->settings().valueThresholdLow, m_image->settings().valueThresholdHigh);
	gvIntensity->setSpan(m_image->settings().intensityThresholdLow, m_image->settings().intensityThresholdHigh);
	gvForeground->setSpan(m_image->settings().foregroundThresholdLow, m_image->settings().foregroundThresholdHigh);
	gvHue->setSpan(m_image->settings().hueThresholdLow, m_image->settings().hueThresholdHigh);
	gvSaturation->setSpan(m_image->settings().saturationThresholdLow, m_image->settings().saturationThresholdHigh);
	gvValue->setSpan(m_image->settings().valueThresholdLow, m_image->settings().valueThresholdHigh);
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
