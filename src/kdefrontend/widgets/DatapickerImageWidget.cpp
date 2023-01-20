/*
	File                 : DatapickerImageWidget.cpp
	Project              : LabPlot
	Description          : widget for datapicker properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2016 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerImageWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/ImageEditor.h"
#include "commonfrontend/widgets/qxtspanslider.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QCompleter>
#include <QDir>
// see https://gitlab.kitware.com/cmake/cmake/-/issues/21609
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QFileSystemModel>
#else
#include <QDirModel>
#endif
#include <QGraphicsScene>
#include <QPainter>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <cmath>

HistogramView::HistogramView(QWidget* parent, int range)
	: QGraphicsView(parent)
	, m_scene(new QGraphicsScene())
	, m_range(range) {
	setTransform(QTransform());
	QRectF pageRect(0, 0, 1000, 100);
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
	auto* pixmap = new QGraphicsPixmapItem(QPixmap(file).scaled(1000, 20, Qt::IgnoreAspectRatio), nullptr);
	pixmap->setZValue(-1);
	pixmap->setPos(0, 90);
	m_scene->addItem(pixmap);
}

void HistogramView::setSpan(int l, int h) {
	l = l * 1000 / m_range;
	h = h * 1000 / m_range;
	m_lowerSlider->setPos(QPointF(l - 1000, 0));
	m_upperSlider->setPos(QPointF(h, 0));
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void HistogramView::resizeEvent(QResizeEvent* event) {
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
		if (bins[i] > max)
			max = bins[i];

	// convert y-scale count to log scale so small counts are still visible
	// scene rect is 1000*100 where upper 1000*80 is for histogram graph
	// and lower 1000*20 is for histogram scale
	QPainterPath path(QPointF(0, (log(bins[0]) * 100 / log(max))));
	for (int i = 1; i <= m_range; i++) {
		int x = i * 1000 / m_range;
		int y = 80;
		if (bins[i] > 1)
			y = 80 - (log(bins[i]) * 80 / log(max));

		path.lineTo(QPointF(x, y));
	}

	painter->drawPath(path);
	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();
}

DatapickerImageWidget::DatapickerImageWidget(QWidget* parent)
	: BaseDock(parent)
	, m_image(nullptr) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//"General"-tab
	ui.leFileName->setClearButtonEnabled(true);
	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	ui.leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));
#else
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));
#endif

	//"Symbol"-tab
	symbolWidget = new SymbolWidget(ui.tSymbol);
	auto* gridLayout = dynamic_cast<QGridLayout*>(ui.tSymbol->layout());
	if (gridLayout)
		gridLayout->addWidget(symbolWidget, 0, 0, 1, 1);

	//"Edit Image"-tab
	auto* editTabLayout = static_cast<QGridLayout*>(ui.tEdit->layout());
	editTabLayout->setContentsMargins(2, 2, 2, 2);
	editTabLayout->setHorizontalSpacing(2);
	editTabLayout->setVerticalSpacing(4);

	ssHue = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	ssHue->setRange(0, 360);
	editTabLayout->addWidget(ssHue, 3, 2);

	ssSaturation = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	ssSaturation->setRange(0, 100);
	editTabLayout->addWidget(ssSaturation, 5, 2);

	ssValue = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	ssValue->setRange(0, 100);
	editTabLayout->addWidget(ssValue, 7, 2);

	ssIntensity = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	ssIntensity->setRange(0, 100);
	editTabLayout->addWidget(ssIntensity, 9, 2);

	ssForeground = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssForeground->setToolTip(
		i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
	ssForeground->setRange(0, 100);
	editTabLayout->addWidget(ssForeground, 11, 2);

	ui.cbGraphType->addItem(i18n("Cartesian (x, y)"), (int)DatapickerImage::GraphType::Linear);
	ui.cbGraphType->addItem(i18n("Polar (x, y°)"), (int)DatapickerImage::GraphType::PolarInDegree);
	ui.cbGraphType->addItem(i18n("Polar (x, y(rad))"), (int)DatapickerImage::GraphType::PolarInRadians);
	ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), ln(y))"), (int)DatapickerImage::GraphType::LnXY);
	ui.cbGraphType->addItem(i18n("Logarithmic (ln(x), y)"), (int)DatapickerImage::GraphType::LnX);
	ui.cbGraphType->addItem(i18n("Logarithmic (x, ln(y))"), (int)DatapickerImage::GraphType::LnY);
	ui.cbGraphType->addItem(i18n("Logarithmic (log(x), log(y))"), (int)DatapickerImage::GraphType::Log10XY);
	ui.cbGraphType->addItem(i18n("Logarithmic (log(x), y)"), (int)DatapickerImage::GraphType::Log10X);
	ui.cbGraphType->addItem(i18n("Logarithmic (x, log(y))"), (int)DatapickerImage::GraphType::Log10Y);
	ui.cbGraphType->addItem(i18n("Ternary (x, y, z)"), (int)DatapickerImage::GraphType::Ternary);

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

	QString valueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_value.xpm"));
	QString hueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_hue.xpm"));
	QString saturationFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_saturation.xpm"));

	gvHue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Hue));
	gvHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvHue, 2, 2);
	gvHue->setScalePixmap(hueFile);

	gvSaturation = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Saturation));
	gvSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvSaturation, 4, 2);
	gvSaturation->setScalePixmap(saturationFile);

	gvValue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Value));
	gvValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvValue, 6, 2);
	gvValue->setScalePixmap(valueFile);

	gvIntensity = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Intensity));
	gvIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvIntensity, 8, 2);
	gvIntensity->setScalePixmap(valueFile);

	gvForeground = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Foreground));
	gvForeground->setToolTip(
		i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
	editTabLayout->addWidget(gvForeground, 10, 2);
	gvForeground->setScalePixmap(valueFile);

	DatapickerImageWidget::updateLocale();

	// SLOTS
	// general
	connect(ui.leName, &QLineEdit::textChanged, this, &DatapickerImageWidget::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &DatapickerImageWidget::commentChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &DatapickerImageWidget::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &DatapickerImageWidget::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &DatapickerImageWidget::fileNameChanged);

	// edit image
	connect(ui.cbPlotImageType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DatapickerImageWidget::plotImageTypeChanged);
	connect(ui.sbRotation, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::rotationChanged);
	connect(ssIntensity, &SpanSlider::spanChanged, this, &DatapickerImageWidget::intensitySpanChanged);
	connect(ssIntensity, &SpanSlider::spanChanged, gvIntensity, &HistogramView::setSpan);
	connect(ssForeground, &SpanSlider::spanChanged, this, &DatapickerImageWidget::foregroundSpanChanged);
	connect(ssForeground, &SpanSlider::spanChanged, gvForeground, &HistogramView::setSpan);
	connect(ssHue, &SpanSlider::spanChanged, this, &DatapickerImageWidget::hueSpanChanged);
	connect(ssHue, &SpanSlider::spanChanged, gvHue, &HistogramView::setSpan);
	connect(ssSaturation, &SpanSlider::spanChanged, this, &DatapickerImageWidget::saturationSpanChanged);
	connect(ssSaturation, &SpanSlider::spanChanged, gvSaturation, &HistogramView::setSpan);
	connect(ssValue, &SpanSlider::spanChanged, this, &DatapickerImageWidget::valueSpanChanged);
	connect(ssValue, &SpanSlider::spanChanged, gvValue, &HistogramView::setSpan);
	connect(ui.sbMinSegmentLength, QOverload<int>::of(&QSpinBox::valueChanged), this, &DatapickerImageWidget::minSegmentLengthChanged);
	connect(ui.sbPointSeparation, QOverload<int>::of(&QSpinBox::valueChanged), this, &DatapickerImageWidget::pointSeparationChanged);

	// axis point
	connect(ui.cbGraphType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DatapickerImageWidget::graphTypeChanged);
	connect(ui.sbTernaryScale, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::ternaryScaleChanged);
	connect(ui.sbPositionX1, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY1, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionX2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionX3, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionY3, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ1, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ2, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.sbPositionZ3, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &DatapickerImageWidget::logicalPositionChanged);

	connect(ui.chbSymbolVisible, &QCheckBox::clicked, this, &DatapickerImageWidget::pointsVisibilityChanged);
}

void DatapickerImageWidget::setImages(QList<DatapickerImage*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_imagesList = list;
	m_image = list.first();
	setAspects(list);

	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_image->parentAspect()->name());
		ui.teComment->setText(m_image->parentAspect()->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	this->load();

	QList<Symbol*> symbols;
	for (auto* image : m_imagesList)
		symbols << image->symbol();

	symbolWidget->setSymbols(symbols);

	connect(m_image->parentAspect(), &AbstractAspect::aspectDescriptionChanged, this, &DatapickerImageWidget::aspectDescriptionChanged);
	connect(m_image, &DatapickerImage::fileNameChanged, this, &DatapickerImageWidget::imageFileNameChanged);
	connect(m_image, &DatapickerImage::rotationAngleChanged, this, &DatapickerImageWidget::imageRotationAngleChanged);
	connect(m_image, &AbstractAspect::aspectRemoved, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &AbstractAspect::aspectAdded, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &DatapickerImage::axisPointsChanged, this, &DatapickerImageWidget::imageAxisPointsChanged);
	connect(m_image, &DatapickerImage::settingsChanged, this, &DatapickerImageWidget::imageEditorSettingsChanged);
	connect(m_image, &DatapickerImage::minSegmentLengthChanged, this, &DatapickerImageWidget::imageMinSegmentLengthChanged);
	connect(m_image, &DatapickerImage::pointVisibilityChanged, this, &DatapickerImageWidget::symbolVisibleChanged);
	connect(m_image, QOverload<int>::of(&DatapickerImage::referencePointSelected), this, &DatapickerImageWidget::imageReferencePointSelected);

	handleWidgetActions();
	updateSymbolWidgets();
}

void DatapickerImageWidget::handleWidgetActions() {
	QString fileName = ui.leFileName->text().trimmed();
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
		// upload histogram to view
		gvIntensity->bins = m_image->intensityBins;
		gvForeground->bins = m_image->foregroundBins;
		gvHue->bins = m_image->hueBins;
		gvSaturation->bins = m_image->saturationBins;
		gvValue->bins = m_image->valueBins;
	}
}

void DatapickerImageWidget::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbRotation->setLocale(numberLocale);
	ui.sbPositionX1->setLocale(numberLocale);
	ui.sbPositionX2->setLocale(numberLocale);
	ui.sbPositionX3->setLocale(numberLocale);
	ui.sbPositionY1->setLocale(numberLocale);
	ui.sbPositionY2->setLocale(numberLocale);
	ui.sbPositionY3->setLocale(numberLocale);
}

//**********************************************************
//****** SLOTs for changes triggered in DatapickerImageWidget ********
//**********************************************************
//"General"-tab
void DatapickerImageWidget::selectFile() {
	const QString& path = GuiTools::openImageFile(QLatin1String("DatapickerImageWidget"));
	if (path.isEmpty())
		return;

	ui.leFileName->setText(path);
}

void DatapickerImageWidget::fileNameChanged() {
	CONDITIONAL_LOCK_RETURN;

	handleWidgetActions();

	const QString& fileName = ui.leFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	for (auto* image : m_imagesList)
		image->setFileName(fileName);
}

void DatapickerImageWidget::graphTypeChanged(int index) {
	auto points = m_image->axisPoints();
	points.type = static_cast<DatapickerImage::GraphType>(ui.cbGraphType->itemData(index).toInt());

	const bool ternary = (points.type == DatapickerImage::GraphType::Ternary);
	ui.lTernaryScale->setVisible(ternary);
	ui.sbTernaryScale->setVisible(ternary);
	ui.lPositionZ1->setVisible(ternary);
	ui.lPositionZ2->setVisible(ternary);
	ui.lPositionZ3->setVisible(ternary);
	ui.sbPositionZ1->setVisible(ternary);
	ui.sbPositionZ2->setVisible(ternary);
	ui.sbPositionZ3->setVisible(ternary);

	CONDITIONAL_RETURN_NO_LOCK;

	if (points.type == DatapickerImage::GraphType::LnXY || points.type == DatapickerImage::GraphType::LnX || points.type == DatapickerImage::GraphType::Log10XY
		|| points.type == DatapickerImage::GraphType::Log10X) {
		if (points.logicalPos[0].x() == 0.0f)
			points.logicalPos[0].setX(0.01f);
		if (points.logicalPos[1].x() == 0.0f)
			points.logicalPos[1].setX(0.01f);
		if (points.logicalPos[2].x() == 0.0f)
			points.logicalPos[2].setX(0.01f);
	}
	if (points.type == DatapickerImage::GraphType::LnXY || points.type == DatapickerImage::GraphType::LnY || points.type == DatapickerImage::GraphType::Log10XY
		|| points.type == DatapickerImage::GraphType::Log10Y) {
		if (points.logicalPos[0].y() == 0.0f)
			points.logicalPos[0].setY(0.01f);
		if (points.logicalPos[1].y() == 0.0f)
			points.logicalPos[1].setY(0.01f);
		if (points.logicalPos[2].y() == 0.0f)
			points.logicalPos[2].setY(0.01f);
	}

	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::ternaryScaleChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	DatapickerImage::ReferencePoints points = m_image->axisPoints();
	points.ternaryScale = value;

	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::logicalPositionChanged() {
	CONDITIONAL_RETURN_NO_LOCK;

	auto points = m_image->axisPoints();
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

void DatapickerImageWidget::pointsVisibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList)
		image->setPointVisibility(state);
}

void DatapickerImageWidget::intensitySpanChanged(int lowerLimit, int upperLimit) {
	CONDITIONAL_LOCK_RETURN;

	auto settings = m_image->settings();
	settings.intensityThresholdHigh = upperLimit;
	settings.intensityThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::foregroundSpanChanged(int lowerLimit, int upperLimit) {
	CONDITIONAL_LOCK_RETURN;

	auto settings = m_image->settings();
	settings.foregroundThresholdHigh = upperLimit;
	settings.foregroundThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::hueSpanChanged(int lowerLimit, int upperLimit) {
	CONDITIONAL_LOCK_RETURN;

	auto settings = m_image->settings();
	settings.hueThresholdHigh = upperLimit;
	settings.hueThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::saturationSpanChanged(int lowerLimit, int upperLimit) {
	CONDITIONAL_LOCK_RETURN;

	auto settings = m_image->settings();
	settings.saturationThresholdHigh = upperLimit;
	settings.saturationThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::valueSpanChanged(int lowerLimit, int upperLimit) {
	CONDITIONAL_LOCK_RETURN;

	auto settings = m_image->settings();
	settings.valueThresholdHigh = upperLimit;
	settings.valueThresholdLow = lowerLimit;
	for (auto* image : m_imagesList)
		image->setSettings(settings);
}

void DatapickerImageWidget::plotImageTypeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList)
		image->setPlotImageType(DatapickerImage::PlotImageType(index));
}

void DatapickerImageWidget::rotationChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* image : m_imagesList)
		image->setRotationAngle(value);
}

void DatapickerImageWidget::minSegmentLengthChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList)
		image->setminSegmentLength(value);
}

void DatapickerImageWidget::pointSeparationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList)
		image->setPointSeparation(value);
}

//*******************************************************************
//******** SLOTs for changes triggered in DatapickerImage ***********
//*******************************************************************
void DatapickerImageWidget::imageFileNameChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	ui.leFileName->setText(name);
}

void DatapickerImageWidget::imageRotationAngleChanged(float angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void DatapickerImageWidget::imageAxisPointsChanged(const DatapickerImage::ReferencePoints& axisPoints) {
	CONDITIONAL_LOCK_RETURN;
	int index = ui.cbGraphType->findData((int)axisPoints.type);
	ui.cbGraphType->setCurrentIndex(index);
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
}

void DatapickerImageWidget::imageEditorSettingsChanged(const DatapickerImage::EditorSettings& settings) {
	CONDITIONAL_LOCK_RETURN;
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
}

void DatapickerImageWidget::imageMinSegmentLengthChanged(const int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbMinSegmentLength->setValue(value);
}

void DatapickerImageWidget::updateSymbolWidgets() {
	int pointCount = m_image->childCount<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	if (pointCount)
		ui.tSymbol->setEnabled(true);
	else
		ui.tSymbol->setEnabled(false);
}

void DatapickerImageWidget::symbolVisibleChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbSymbolVisible->setChecked(on);
}

void DatapickerImageWidget::imageReferencePointSelected(int index) {
	ui.rbRefPoint1->setChecked(index == 0);
	ui.rbRefPoint2->setChecked(index == 1);
	ui.rbRefPoint3->setChecked(index == 2);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerImageWidget::load() {
	if (!m_image)
		return;

	// No lock, because it is done already in the caller function
	ui.leFileName->setText(m_image->fileName());

	// highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_image->fileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	imageReferencePointSelected(m_image->currentSelectedReferencePoint());

	ui.cbGraphType->setCurrentIndex((int)m_image->axisPoints().type);
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
	ui.cbPlotImageType->setCurrentIndex((int)m_image->plotImageType());
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
	ui.chbSymbolVisible->setChecked(m_image->pointVisibility());
}
