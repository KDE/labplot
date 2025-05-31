/*
	File                 : DatapickerImageWidget.cpp
	Project              : LabPlot
	Description          : widget for datapicker properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2016 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerImageWidget.h"
#include "backend/core/Project.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/ImageEditor.h"
#include "frontend/GuiTools.h"
#include "frontend/widgets/qxtspanslider.h"
#include "frontend/widgets/SymbolWidget.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QCompleter>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <QStandardPaths>

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
	painter->setRenderHint(QPainter::Antialiasing);
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
	setBaseWidgets(ui.leName, ui.teComment);

	//"General"-tab
	ui.leFileName->setClearButtonEnabled(true);
	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
	ui.leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));

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
	ssHue->setRange(0, 360);
	editTabLayout->addWidget(ssHue, 3, 2);

	ssSaturation = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssSaturation->setRange(0, 100);
	editTabLayout->addWidget(ssSaturation, 5, 2);

	ssValue = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssValue->setRange(0, 100);
	editTabLayout->addWidget(ssValue, 7, 2);

	ssIntensity = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssIntensity->setRange(0, 100);
	editTabLayout->addWidget(ssIntensity, 9, 2);

	ssForeground = new SpanSlider(Qt::Horizontal, ui.tEdit);
	ssForeground->setRange(0, 100);
	editTabLayout->addWidget(ssForeground, 11, 2);

	ui.lTernaryScale->setHidden(true);
	ui.sbTernaryScale->setHidden(true);
	ui.lPositionZ1->setHidden(true);
	ui.lPositionZ2->setHidden(true);
	ui.lPositionZ3->setHidden(true);
	ui.sbPositionZ1->setHidden(true);
	ui.sbPositionZ2->setHidden(true);
	ui.sbPositionZ3->setHidden(true);

	QString valueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_value.xpm"));
	QString hueFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_hue.xpm"));
	QString saturationFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/colorchooser/colorchooser_saturation.xpm"));

	gvHue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Hue));
	editTabLayout->addWidget(gvHue, 2, 2);
	gvHue->setScalePixmap(hueFile);

	gvSaturation = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Saturation));
	editTabLayout->addWidget(gvSaturation, 4, 2);
	gvSaturation->setScalePixmap(saturationFile);

	gvValue = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Value));
	editTabLayout->addWidget(gvValue, 6, 2);
	gvValue->setScalePixmap(valueFile);

	gvIntensity = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Intensity));
	editTabLayout->addWidget(gvIntensity, 8, 2);
	gvIntensity->setScalePixmap(valueFile);

	gvForeground = new HistogramView(ui.tEdit, ImageEditor::colorAttributeMax(DatapickerImage::ColorAttributes::Foreground));
	editTabLayout->addWidget(gvForeground, 10, 2);
	gvForeground->setScalePixmap(valueFile);

	updateLocale();
	retranslateUi();

	// SLOTS
	// general
	connect(ui.bOpen, &QPushButton::clicked, this, &DatapickerImageWidget::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &DatapickerImageWidget::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &DatapickerImageWidget::fileNameChanged);
	connect(ui.cbFileRelativePath, &QCheckBox::clicked, this, &DatapickerImageWidget::relativeChanged);
	connect(ui.cbFileEmbedd, &QCheckBox::clicked, this, &DatapickerImageWidget::embeddedChanged);

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

	connect(ui.cbDatetime, &QCheckBox::clicked, this, &DatapickerImageWidget::dateTimeUsageChanged);
	connect(ui.dtePositionX1, &QDateTimeEdit::dateTimeChanged, this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.dtePositionX2, &QDateTimeEdit::dateTimeChanged, this, &DatapickerImageWidget::logicalPositionChanged);
	connect(ui.dtePositionX3, &QDateTimeEdit::dateTimeChanged, this, &DatapickerImageWidget::logicalPositionChanged);

	connect(ui.chbSymbolVisible, &QCheckBox::clicked, this, &DatapickerImageWidget::pointsVisibilityChanged);
}

void DatapickerImageWidget::setImages(QList<DatapickerImage*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_imagesList = list;
	m_image = list.first();

	// Set parents as aspects, because their name will be changed
	QList<AbstractAspect*> datapickers;
	for (const auto* l : list)
		datapickers.push_back(l->parentAspect());
	setAspects(datapickers);

	if (list.size() == 1) {
		ui.leName->setText(m_image->parentAspect()->name());
		ui.teComment->setText(m_image->parentAspect()->comment());
	}

	this->load();

	QList<Symbol*> symbols;
	for (auto* image : m_imagesList)
		symbols << image->symbol();

	symbolWidget->setSymbols(symbols);

	connect(m_image, &DatapickerImage::fileNameChanged, this, &DatapickerImageWidget::imageFileNameChanged);
	connect(m_image, &DatapickerImage::embeddedChanged, this, &DatapickerImageWidget::imageEmbeddedChanged);
	connect(m_image, &DatapickerImage::rotationAngleChanged, this, &DatapickerImageWidget::imageRotationAngleChanged);
	connect(m_image, &AbstractAspect::childAspectRemoved, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &AbstractAspect::childAspectAdded, this, &DatapickerImageWidget::updateSymbolWidgets);
	connect(m_image, &DatapickerImage::axisPointsChanged, this, &DatapickerImageWidget::imageAxisPointsChanged);
	connect(m_image, &DatapickerImage::settingsChanged, this, &DatapickerImageWidget::imageEditorSettingsChanged);
	connect(m_image, &DatapickerImage::minSegmentLengthChanged, this, &DatapickerImageWidget::imageMinSegmentLengthChanged);
	connect(m_image, &DatapickerImage::pointVisibilityChanged, this, &DatapickerImageWidget::symbolVisibleChanged);
	connect(m_image, QOverload<int>::of(&DatapickerImage::referencePointSelected), this, &DatapickerImageWidget::imageReferencePointSelected);
	connect(m_image, &DatapickerImage::relativeFilePathChanged, this, &DatapickerImageWidget::imageRelativeChanged);
	if (m_image->project())
		connect(m_image->project(), &Project::saved, this, &DatapickerImageWidget::updateFileRelativePathCheckBoxEnable);

	handleWidgetActions();
	updateSymbolWidgets();
}

void DatapickerImageWidget::handleWidgetActions() {
	const QString fileName = m_image->fileName();
	const bool embedded = m_image->embedded();
	const bool valid = !m_image->originalPlotImage.isNull();
	const bool b = !fileName.isEmpty() || (embedded && valid);
	ui.leFileName->setEnabled(!embedded);
	updateFileRelativePathCheckBoxEnable();
	ui.tEdit->setEnabled(b);
	ui.cbFileEmbedd->setEnabled(valid);
	ui.cbGraphType->setEnabled(b);
	ui.cbDatetime->setEnabled(b);
	ui.sbRotation->setEnabled(b);
	ui.sbPositionX1->setEnabled(b);
	ui.sbPositionX2->setEnabled(b);
	ui.sbPositionX3->setEnabled(b);
	ui.sbPositionY1->setEnabled(b);
	ui.sbPositionY2->setEnabled(b);
	ui.sbPositionY3->setEnabled(b);
	ui.dtePositionX1->setEnabled(b);
	ui.dtePositionX2->setEnabled(b);
	ui.dtePositionX3->setEnabled(b);
	ui.sbMinSegmentLength->setEnabled(b);
	ui.sbPointSeparation->setEnabled(b);

	const bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName) && !embedded);
	GuiTools::highlight(ui.leFileName, invalid);

	if (b) {
		// upload histogram to view
		gvIntensity->bins = m_image->intensityBins;
		gvForeground->bins = m_image->foregroundBins;
		gvHue->bins = m_image->hueBins;
		gvSaturation->bins = m_image->saturationBins;
		gvValue->bins = m_image->valueBins;
	}
}

void DatapickerImageWidget::updateXPositionWidgets(bool datetime) {
	ui.sbPositionX1->setVisible(!datetime);
	ui.sbPositionX2->setVisible(!datetime);
	ui.sbPositionX3->setVisible(!datetime);
	ui.dtePositionX1->setVisible(datetime);
	ui.dtePositionX2->setVisible(datetime);
	ui.dtePositionX3->setVisible(datetime);
}

void DatapickerImageWidget::updateLocale() {
	const auto locale = QLocale();
	ui.sbRotation->setLocale(locale);
	ui.sbPositionX1->setLocale(locale);
	ui.sbPositionX2->setLocale(locale);
	ui.sbPositionX3->setLocale(locale);
	ui.sbPositionY1->setLocale(locale);
	ui.sbPositionY2->setLocale(locale);
	ui.sbPositionY3->setLocale(locale);
	ui.dtePositionX1->setLocale(locale);
	ui.dtePositionX2->setLocale(locale);
	ui.dtePositionX3->setLocale(locale);
}

void DatapickerImageWidget::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbGraphType->clear();
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

	ui.cbPlotImageType->clear();
	ui.cbPlotImageType->addItem(i18n("No Image"));
	ui.cbPlotImageType->addItem(i18n("Original Image"));
	ui.cbPlotImageType->addItem(i18n("Processed Image"));

	// tooltip texts
	ssValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	ssIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	ssForeground->setToolTip(
		i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
	ssHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	ssSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	gvHue->setToolTip(i18n("Select the range for the hue.\nEverything outside of this range will be set to white."));
	gvSaturation->setToolTip(i18n("Select the range for the saturation.\nEverything outside of this range will be set to white."));
	gvValue->setToolTip(i18n("Select the range for the value, the degree of lightness of the color.\nEverything outside of this range will be set to white."));
	gvIntensity->setToolTip(i18n("Select the range for the intensity.\nEverything outside of this range will be set to white."));
	gvForeground->setToolTip(i18n("Select the range for the colors that are not part of the background color.\nEverything outside of this range will be set to white."));
}

void DatapickerImageWidget::updateFileRelativePathCheckBoxEnable() {
	const auto* project = m_image->project();
	if (!project || project->fileName().isEmpty()) {
		ui.cbFileRelativePath->setEnabled(false);
		ui.cbFileRelativePath->setToolTip(i18n("Save project before using this option"));
	} else if (m_image->embedded()) {
		ui.cbFileRelativePath->setEnabled(false);
		ui.cbFileRelativePath->setToolTip(QStringLiteral(""));
	} else if (!m_image->fileName().isEmpty() && QFile::exists(m_image->fileName())) {
		ui.cbFileRelativePath->setEnabled(true);
		ui.cbFileRelativePath->setToolTip(QStringLiteral(""));
	} else {
		ui.cbFileRelativePath->setEnabled(false);
		ui.cbFileRelativePath->setToolTip(i18n("Invalid image"));
	}

	ui.cbFileRelativePath->setVisible(!m_image->embedded());
}

//**********************************************************
//****** SLOTs for changes triggered in DatapickerImageWidget ********
//**********************************************************
//"General"-tab
void DatapickerImageWidget::selectFile() {
	const QString& path = GuiTools::openImageFile(QLatin1String("DatapickerImageWidget"));
	if (path.isEmpty())
		return;

	ui.cbFileRelativePath->setChecked(false);
	ui.leFileName->setText(path);
}

void DatapickerImageWidget::embeddedChanged(bool embedded) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList)
		image->setEmbedded(embedded);

	// embedded property was set, update the file name LineEdit after this
	if (embedded) {
		QFileInfo fi(m_image->fileName());
		ui.leFileName->setText(fi.fileName());
	} else
		ui.leFileName->setText(m_image->fileName());
}

void DatapickerImageWidget::relativeChanged(bool relative) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imagesList) {
		image->setRelativeFilePath(relative);
	}

	// Load new filename
	ui.leFileName->setText(m_image->fileName());
}

void DatapickerImageWidget::fileNameChanged() {
	CONDITIONAL_LOCK_RETURN;

	const QString fileName = ui.leFileName->text();
	for (auto* image : m_imagesList)
		image->setImage(fileName, image->embedded());
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

void DatapickerImageWidget::dateTimeUsageChanged(bool datetime) {
	updateXPositionWidgets(datetime);

	CONDITIONAL_LOCK_RETURN;

	auto points = m_image->axisPoints();
	points.datetime = datetime;
	for (auto* image : m_imagesList)
		image->setAxisPoints(points);
}

void DatapickerImageWidget::logicalPositionChanged() {
	CONDITIONAL_RETURN_NO_LOCK;

	auto points = m_image->axisPoints();
	if (points.datetime) {
		points.logicalPos[0].setX(ui.dtePositionX1->dateTime().toMSecsSinceEpoch());
		points.logicalPos[1].setX(ui.dtePositionX2->dateTime().toMSecsSinceEpoch());
		points.logicalPos[2].setX(ui.dtePositionX3->dateTime().toMSecsSinceEpoch());
	} else {
		points.logicalPos[0].setX(ui.sbPositionX1->value());
		points.logicalPos[1].setX(ui.sbPositionX2->value());
		points.logicalPos[2].setX(ui.sbPositionX3->value());
	}

	points.logicalPos[0].setY(ui.sbPositionY1->value());
	points.logicalPos[1].setY(ui.sbPositionY2->value());
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
	handleWidgetActions();

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

void DatapickerImageWidget::imageEmbeddedChanged(bool embedded) {
	handleWidgetActions();

	CONDITIONAL_LOCK_RETURN;
	ui.cbFileEmbedd->setChecked(embedded);
}

void DatapickerImageWidget::imageRelativeChanged(bool relative) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbFileRelativePath->setChecked(relative);
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
	ui.cbFileEmbedd->setChecked(m_image->embedded());
	embeddedChanged(m_image->embedded());
	ui.cbFileRelativePath->setChecked(m_image->isRelativeFilePath());
	updateFileRelativePathCheckBoxEnable();
	ui.leFileName->setText(m_image->fileName());

	// highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_image->fileName();
	bool invalid = (!m_image->embedded() && !fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	imageReferencePointSelected(m_image->currentSelectedReferencePoint());

	ui.cbGraphType->setCurrentIndex(ui.cbGraphType->findData((int)m_image->axisPoints().type));
	ui.sbTernaryScale->setValue(m_image->axisPoints().ternaryScale);
	const bool datetime = m_image->axisPoints().datetime;
	ui.cbDatetime->setChecked(datetime);
	updateXPositionWidgets(datetime);

	const double x1 = m_image->axisPoints().logicalPos[0].x();
	const double x2 = m_image->axisPoints().logicalPos[1].x();
	const double x3 = m_image->axisPoints().logicalPos[2].x();

	ui.dtePositionX1->setMSecsSinceEpochUTC(x1);
	ui.dtePositionX2->setMSecsSinceEpochUTC(x2);
	ui.dtePositionX3->setMSecsSinceEpochUTC(x3);

	ui.sbPositionX1->setValue(x1);
	ui.sbPositionY1->setValue(m_image->axisPoints().logicalPos[0].y());
	ui.sbPositionX2->setValue(x2);
	ui.sbPositionY2->setValue(m_image->axisPoints().logicalPos[1].y());
	ui.sbPositionX3->setValue(x3);
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
