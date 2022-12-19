/*
	File                 : ImageDock.cpp
	Project              : LabPlot
	Description          : widget for image properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2020 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageDock.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"

#include <QCompleter>
// see https://gitlab.kitware.com/cmake/cmake/-/issues/21609
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QFileSystemModel>
#else
#include <QDirModel>
#endif
#include <QPageSize>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class ImageDock
  \brief  Provides a widget for editing the properties of the worksheets image element.

  \ingroup kdefrontend
*/

ImageDock::ImageDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	ui.leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));
#else
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));
#endif

	// 	ui.cbSize->addItem(i18n("Original"));
	// 	ui.cbSize->addItem(i18n("Custom"));

	// Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));

	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));

	QString suffix;
	if (m_units == BaseDock::Units::Metric)
		suffix = QStringLiteral(" cm");
	else
		suffix = QStringLiteral(" in");

	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);

	// border
	ui.cbBorderStyle->addItem(i18n("No line"));
	ui.cbBorderStyle->addItem(i18n("Solid line"));
	ui.cbBorderStyle->addItem(i18n("Dash line"));
	ui.cbBorderStyle->addItem(i18n("Dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot dot line"));

	ImageDock::updateLocale();

	// SLOTs
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &ImageDock::nameChanged);
	connect(ui.chbEmbedded, &QCheckBox::clicked, this, &ImageDock::embeddedChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ImageDock::commentChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &ImageDock::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &ImageDock::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &ImageDock::fileNameChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageDock::opacityChanged);

	// Size
	connect(ui.sbWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::widthChanged);
	connect(ui.sbHeight, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::heightChanged);
	connect(ui.chbKeepRatio, &QCheckBox::clicked, this, &ImageDock::keepRatioChanged);

	// Position
	connect(ui.cbPositionX, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageDock::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageDock::positionYChanged);
	connect(ui.sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::customPositionYChanged);
	connect(ui.cbHorizontalAlignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageDock::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageDock::verticalAlignmentChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageDock::rotationChanged);

	connect(ui.chbVisible, &QCheckBox::clicked, this, &ImageDock::visibilityChanged);

	// Border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &ImageDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageDock::borderOpacityChanged);
}

void ImageDock::setImages(QList<Image*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_imageList = list;
	m_image = list.first();
	setAspects(list);

	updateLocale();

	// if there are more then one image in the list, disable the name and comment field in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_image->name());
		ui.teComment->setText(m_image->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first image
	this->load();

	// init connections
	// General
	connect(m_image, &Image::aspectDescriptionChanged, this, &ImageDock::aspectDescriptionChanged);
	connect(m_image, &Image::fileNameChanged, this, &ImageDock::imageFileNameChanged);
	connect(m_image, &Image::embeddedChanged, this, &ImageDock::imageEmbeddedChanged);
	connect(m_image, &Image::opacityChanged, this, &ImageDock::imageOpacityChanged);
	connect(m_image, &Image::visibleChanged, this, &ImageDock::imageVisibleChanged);

	// Size
	connect(m_image, &Image::widthChanged, this, &ImageDock::imageWidthChanged);
	connect(m_image, &Image::heightChanged, this, &ImageDock::imageHeightChanged);
	connect(m_image, &Image::keepRatioChanged, this, &ImageDock::imageKeepRatioChanged);

	// Position
	connect(m_image, &Image::positionChanged, this, &ImageDock::imagePositionChanged);
	connect(m_image, &Image::horizontalAlignmentChanged, this, &ImageDock::imageHorizontalAlignmentChanged);
	connect(m_image, &Image::verticalAlignmentChanged, this, &ImageDock::imageVerticalAlignmentChanged);
	connect(m_image, &Image::rotationAngleChanged, this, &ImageDock::imageRotationAngleChanged);

	// Border
	connect(m_image, &Image::borderPenChanged, this, &ImageDock::imageBorderPenChanged);
	connect(m_image, &Image::borderOpacityChanged, this, &ImageDock::imageBorderOpacityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ImageDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
}

void ImageDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	if (m_units == BaseDock::Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QStringLiteral(" cm");
		ui.sbWidth->setValue(ui.sbWidth->value() * 2.54);
		ui.sbHeight->setValue(ui.sbHeight->value() * 2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value() * 2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value() * 2.54);
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QStringLiteral(" in");
		ui.sbWidth->setValue(ui.sbWidth->value() / 2.54);
		ui.sbHeight->setValue(ui.sbHeight->value() / 2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value() / 2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value() / 2.54);
	}

	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);
}

//*************************************************************
//******** SLOTs for changes triggered in ImageDock ***********
//*************************************************************
/*!
	opens a file dialog and lets the user select the image file.
*/
void ImageDock::selectFile() {
	const QString& path = GuiTools::openImageFile(QStringLiteral("ImageDock"));
	if (path.isEmpty())
		return;

	ui.leFileName->setText(path);

	// above the path was set in the text field which triggered setting
	// of it in Image and loading of the image. Call embeddedChanged()
	// to update the text field and to show the actual file name only
	// and not the whole path if the image is being embedded.
	CONDITIONAL_LOCK_RETURN;
	embeddedChanged(ui.chbEmbedded->checkState());
}

void ImageDock::embeddedChanged(int state) {
	bool embedded = static_cast<bool>(state);
	ui.leFileName->setEnabled(!embedded);

	CONDITIONAL_LOCK_RETURN;

	if (embedded) {
		QFileInfo fi(m_image->fileName());
		ui.leFileName->setText(fi.fileName());
	} else
		ui.leFileName->setText(m_image->fileName());

	for (auto* image : m_imageList)
		image->setEmbedded(embedded);
}

void ImageDock::fileNameChanged() {
	const QString& fileName = ui.leFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setFileName(fileName);
}

void ImageDock::opacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	float opacity = (float)value / 100;
	for (auto* image : m_imageList)
		image->setOpacity(opacity);
}

// Size
void ImageDock::sizeChanged(int /*index*/) {
	CONDITIONAL_LOCK_RETURN;
}

void ImageDock::widthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	int width = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList)
		image->setWidth(width);
}

void ImageDock::heightChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	int height = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList)
		image->setHeight(height);
}

void ImageDock::keepRatioChanged(int state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setKeepRatio(state);
}

// Position
/*!
	called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void ImageDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	WorksheetElement::PositionWrapper position = m_image->position();
	position.horizontalPosition = WorksheetElement::HorizontalPosition(index);
	for (auto* image : m_imageList)
		image->setPosition(position);
}

/*!
	called when label's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void ImageDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_image->position();
	position.verticalPosition = WorksheetElement::VerticalPosition(index);
	for (auto* image : m_imageList)
		image->setPosition(position);
}

void ImageDock::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double x = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList) {
		auto position = image->position();
		position.point.setX(x);
		image->setPosition(position);
	}
}

void ImageDock::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double y = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList) {
		auto position = image->position();
		position.point.setY(y);
		image->setPosition(position);
	}
}

void ImageDock::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void ImageDock::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}

void ImageDock::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setRotationAngle(value);
}

void ImageDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* image : m_imageList)
		image->setVisible(state);
}

// border
void ImageDock::borderStyleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setStyle(penStyle);
		image->setBorderPen(pen);
	}
}

void ImageDock::borderColorChanged(const QColor& color) {
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);

	CONDITIONAL_LOCK_RETURN;

	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setColor(color);
		image->setBorderPen(pen);
	}
}

void ImageDock::borderWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		image->setBorderPen(pen);
	}
}

void ImageDock::borderOpacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	qreal opacity = (float)value / 100.;
	for (auto* image : m_imageList)
		image->setBorderOpacity(opacity);
}

//*************************************************************
//********** SLOTs for changes triggered in Image *************
//*************************************************************
void ImageDock::imageFileNameChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	ui.leFileName->setText(name);
}

void ImageDock::imageEmbeddedChanged(bool keep) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbEmbedded->setChecked(keep);
}

void ImageDock::imageOpacityChanged(float opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOpacity->setValue(qRound(opacity * 100.0));
}

// Size
void ImageDock::imageWidthChanged(int width) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(width, m_worksheetUnit));
}

void ImageDock::imageHeightChanged(int height) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(height, m_worksheetUnit));
}

void ImageDock::imageKeepRatioChanged(bool keep) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbKeepRatio->setChecked(keep);
}

// Position
void ImageDock::imagePositionChanged(const WorksheetElement::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit));
	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
}

void ImageDock::imageHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void ImageDock::imageVerticalAlignmentChanged(WorksheetElement::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}

void ImageDock::imageRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void ImageDock::imageVisibleChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbVisible->setChecked(on);
}

// Border
void ImageDock::imageBorderPenChanged(const QPen& pen) {
	CONDITIONAL_LOCK_RETURN;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point)); // No if!
}

void ImageDock::imageBorderOpacityChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	float v = (float)value * 100.;
	ui.sbBorderOpacity->setValue(v);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void ImageDock::load() {
	if (!m_image)
		return;

	// No Lock!

	ui.leFileName->setText(m_image->fileName());
	ui.chbEmbedded->setChecked(m_image->embedded());
	embeddedChanged(ui.chbEmbedded->checkState());
	ui.chbVisible->setChecked(m_image->isVisible());

	// Size
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_image->width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_image->height(), m_worksheetUnit));
	ui.chbKeepRatio->setChecked(m_image->keepRatio());

	// Position
	ui.cbPositionX->setCurrentIndex((int)m_image->position().horizontalPosition);
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(m_image->position().point.x(), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_image->position().verticalPosition);
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(m_image->position().point.y(), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_image->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_image->verticalAlignment());
	ui.sbRotation->setValue(m_image->rotationAngle());

	// Border
	ui.kcbBorderColor->setColor(m_image->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex((int)m_image->borderPen().style());
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(m_image->borderPen().widthF(), Worksheet::Unit::Point));
	ui.sbBorderOpacity->setValue(round(m_image->borderOpacity() * 100));
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
}
