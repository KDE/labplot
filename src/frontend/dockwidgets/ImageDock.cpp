/*
	File                 : ImageDock.cpp
	Project              : LabPlot
	Description          : widget for image properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageDock.h"
#include "backend/core/Settings.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/Worksheet.h"
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/ThemeHandler.h"
#include "frontend/widgets/LineWidget.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalization>
#include <KLocalizedString>

#include <QCompleter>
#include <QFileSystemModel>
#include <QPageSize>

#include <gsl/gsl_const_cgs.h>

/*!
  \class ImageDock
  \brief  Provides a widget for editing the properties of the worksheets image element.

  \ingroup frontend
*/

ImageDock::ImageDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chbVisible);

	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
	ui.leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));

	auto* layout = static_cast<QHBoxLayout*>(ui.tabBorder->layout());
	borderLineWidget = new LineWidget(ui.tabBorder);
	layout->insertWidget(0, borderLineWidget);

	QString suffix;
	if (m_units == BaseDock::Units::Metric)
		suffix = i18n("%v cm");
	else
		suffix = i18n("%v in");

	KLocalization::setupSpinBoxFormatString(ui.sbWidth, ki18nc("@label:spinbox Suffix for the width", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbHeight, ki18nc("@label:spinbox Suffix for the height", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPositionX, ki18nc("@label:spinbox Suffix for the X position", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPositionY, ki18nc("@label:spinbox Suffix for the Y position", qPrintable(suffix)));

	updateLocale();
	retranslateUi();

	// SLOTs
	// General
	connect(ui.chbEmbedded, &QCheckBox::clicked, this, &ImageDock::embeddedChanged);
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

	connect(ui.dtePositionXLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &ImageDock::positionXLogicalDateTimeChanged);
	connect(ui.dtePositionXLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &ImageDock::positionXLogicalDateTimeChanged);
	connect(ui.sbPositionYLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ImageDock::positionYLogicalChanged);

	connect(ui.chbLock, &QCheckBox::clicked, this, &ImageDock::lockChanged);
	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &ImageDock::bindingChanged);
}

void ImageDock::setImages(QList<Image*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_imageList = list;
	m_image = list.first();
	setAspects(list);

	QList<Line*> lines;
	for (auto* image : m_imageList)
		lines << image->borderLine();

	borderLineWidget->setLines(lines);

	// show the properties of the first image
	this->load();

	// init connections
	// General
	connect(m_image, &Image::fileNameChanged, this, &ImageDock::imageFileNameChanged);
	connect(m_image, &Image::embeddedChanged, this, &ImageDock::imageEmbeddedChanged);
	connect(m_image, &Image::opacityChanged, this, &ImageDock::imageOpacityChanged);
	connect(m_image, &Image::lockChanged, this, &ImageDock::imageLockChanged);

	// Size
	connect(m_image, &Image::widthChanged, this, &ImageDock::imageWidthChanged);
	connect(m_image, &Image::heightChanged, this, &ImageDock::imageHeightChanged);
	connect(m_image, &Image::keepRatioChanged, this, &ImageDock::imageKeepRatioChanged);

	// Position
	connect(m_image, &Image::positionChanged, this, &ImageDock::imagePositionChanged);
	connect(m_image, &Image::positionLogicalChanged, this, &ImageDock::imagePositionLogicalChanged);
	connect(m_image, &Image::coordinateBindingEnabledChanged, this, &ImageDock::imageCoordinateBindingEnabledChanged);
	connect(m_image, &Image::horizontalAlignmentChanged, this, &ImageDock::imageHorizontalAlignmentChanged);
	connect(m_image, &Image::verticalAlignmentChanged, this, &ImageDock::imageVerticalAlignmentChanged);
	connect(m_image, &Image::rotationAngleChanged, this, &ImageDock::imageRotationAngleChanged);
}

void ImageDock::updateUnits() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	if (m_units == BaseDock::Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = i18n("%v cm");
		ui.sbWidth->setValue(roundValue(ui.sbWidth->value() * GSL_CONST_CGS_INCH));
		ui.sbHeight->setValue(roundValue(ui.sbHeight->value() * GSL_CONST_CGS_INCH));
		ui.sbPositionX->setValue(roundValue(ui.sbPositionX->value() * GSL_CONST_CGS_INCH));
		ui.sbPositionY->setValue(roundValue(ui.sbPositionY->value() * GSL_CONST_CGS_INCH));
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = i18n("%v in");
		ui.sbWidth->setValue(roundValue(ui.sbWidth->value() / GSL_CONST_CGS_INCH));
		ui.sbHeight->setValue(roundValue(ui.sbHeight->value() / GSL_CONST_CGS_INCH));
		ui.sbPositionX->setValue(roundValue(ui.sbPositionX->value() / GSL_CONST_CGS_INCH));
		ui.sbPositionY->setValue(roundValue(ui.sbPositionY->value() / GSL_CONST_CGS_INCH));
	}

	KLocalization::setupSpinBoxFormatString(ui.sbWidth, ki18nc("@label:spinbox Suffix for the width", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbHeight, ki18nc("@label:spinbox Suffix for the height", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPositionX, ki18nc("@label:spinbox Suffix for the X position", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPositionY, ki18nc("@label:spinbox Suffix for the Y position", qPrintable(suffix)));
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void ImageDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	borderLineWidget->updateLocale();
}

void ImageDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// Positioning and alignment
	ui.cbPositionX->clear();
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));

	ui.cbPositionY->clear();
	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));

	ui.cbHorizontalAlignment->clear();
	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->clear();
	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));
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

	for (auto* image : m_imageList)
		image->setEmbedded(embedded);

	// embedded property was set, update the file name LineEdit after this
	if (embedded) {
		QFileInfo fi(m_image->fileName());
		ui.leFileName->setText(fi.fileName());
	} else
		ui.leFileName->setText(m_image->fileName());
}

void ImageDock::fileNameChanged() {
	const QString& fileName = ui.leFileName->text();

	if (!m_image->embedded()) {
		bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
		GuiTools::highlight(ui.leFileName, invalid);
		ui.chbEmbedded->setEnabled(!invalid);
	} else
		GuiTools::highlight(ui.leFileName, false);

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

	auto position = m_image->position();
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

/*!
 * \brief ImageDock::bindingChanged
 * Bind Image to the cartesian plot coords or not
 * \param checked
 */
void ImageDock::bindingChanged(bool checked) {
	// widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_image->parent<CartesianPlot>());
	if (plot && plot->xRangeFormatDefault() == RangeT::Format::DateTime) {
		ui.lPositionXLogicalDateTime->setVisible(checked);
		ui.dtePositionXLogical->setVisible(checked);

		ui.lPositionXLogical->setVisible(false);
		ui.sbPositionXLogical->setVisible(false);
	} else {
		ui.lPositionXLogicalDateTime->setVisible(false);
		ui.dtePositionXLogical->setVisible(false);

		ui.lPositionXLogical->setVisible(checked);
		ui.sbPositionXLogical->setVisible(checked);
	}

	ui.lPositionYLogical->setVisible(checked);
	ui.sbPositionYLogical->setVisible(checked);

	CONDITIONAL_LOCK_RETURN;

	ui.chbBindLogicalPos->setChecked(checked);

	for (auto* label : m_imageList)
		label->setCoordinateBindingEnabled(checked);
}

// positioning using logical plot coordinates
void ImageDock::positionXLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPointF pos = m_image->positionLogical();
	pos.setX(value);
	for (auto* label : m_imageList)
		label->setPositionLogical(pos);
}

void ImageDock::positionXLogicalDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	QPointF pos = m_image->positionLogical();
	pos.setX(value);
	for (auto* label : m_imageList)
		label->setPositionLogical(pos);
}

void ImageDock::positionYLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPointF pos = m_image->positionLogical();
	pos.setY(value);
	for (auto* label : m_imageList)
		label->setPositionLogical(pos);
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

void ImageDock::lockChanged(bool locked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* image : m_imageList)
		image->setLock(locked);
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
	ui.sbOpacity->setValue(std::round(opacity * 100));
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
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.x(), m_units), m_worksheetUnit));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.y(), m_units), m_worksheetUnit));
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

void ImageDock::imageCoordinateBindingEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	bindingChanged(enabled);
}

void ImageDock::imagePositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionXLogical->setValue(pos.x());
	ui.dtePositionXLogical->setMSecsSinceEpochUTC(pos.x());
	ui.sbPositionYLogical->setValue(pos.y());
}

void ImageDock::imageRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void ImageDock::imageLockChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbLock->setChecked(on);
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
	ui.chbLock->setChecked(m_image->isLocked());
	ui.chbVisible->setChecked(m_image->isVisible());

	// Size
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_image->width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_image->height(), m_worksheetUnit));
	ui.chbKeepRatio->setChecked(m_image->keepRatio());

	// Position
	ui.cbPositionX->setCurrentIndex((int)m_image->position().horizontalPosition);
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_image->position().point.x(), m_units), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_image->position().verticalPosition);
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_image->position().point.y(), m_units), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_image->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_image->verticalAlignment());

	// widgets for positioning using logical plot coordinates
	bool allowLogicalCoordinates = (m_image->plot() != nullptr);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_image->plot());
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lPositionXLogical->show();
			ui.sbPositionXLogical->show();
			ui.lPositionXLogicalDateTime->hide();
			ui.dtePositionXLogical->hide();

			ui.sbPositionXLogical->setValue(m_image->positionLogical().x());
			ui.sbPositionYLogical->setValue(m_image->positionLogical().y());
		} else { // DateTime
			ui.lPositionXLogical->hide();
			ui.sbPositionXLogical->hide();
			ui.lPositionXLogicalDateTime->show();
			ui.dtePositionXLogical->show();

			ui.dtePositionXLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionXLogical->setMSecsSinceEpochUTC(m_image->positionLogical().x());
		}

		ui.chbBindLogicalPos->setChecked(m_image->coordinateBindingEnabled());
		bindingChanged(m_image->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.sbPositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.sbPositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
	}

	ui.sbRotation->setValue(m_image->rotationAngle());
}
