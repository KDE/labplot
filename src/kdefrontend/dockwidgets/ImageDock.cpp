/***************************************************************************
    File                 : ImageDock.cpp
    Project              : LabPlot
    Description          : widget for image properties
    --------------------------------------------------------------------
    Copyright            : (C) 2019-2020 by Alexander Semke (alexander.semke@web.de)

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

#include "ImageDock.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/TemplateHandler.h"

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QPageSize>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class ImageDock
  \brief  Provides a widget for editing the properties of the worksheets image element.

  \ingroup kdefrontend
*/

ImageDock::ImageDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));

// 	ui.cbSize->addItem(i18n("Original"));
// 	ui.cbSize->addItem(i18n("Custom"));

	//Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));
	ui.cbPositionX->addItem(i18n("Custom"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));
	ui.cbPositionY->addItem(i18n("Custom"));

	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));

	QString suffix;
	if (m_units == BaseDock::Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);

	//border
	ui.cbBorderStyle->addItem(i18n("No line"));
	ui.cbBorderStyle->addItem(i18n("Solid line"));
	ui.cbBorderStyle->addItem(i18n("Dash line"));
	ui.cbBorderStyle->addItem(i18n("Dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot dot line"));

	ImageDock::updateLocale();

	//SLOTs
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &ImageDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ImageDock::commentChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &ImageDock::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &ImageDock::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &ImageDock::fileNameChanged);
	connect(ui.sbOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageDock::opacityChanged);

	//Size
	connect(ui.sbWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ImageDock::widthChanged);
	connect(ui.sbHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ImageDock::heightChanged);
	connect(ui.chbKeepRatio, &QCheckBox::clicked, this, &ImageDock::keepRatioChanged);

	//Position
	connect(ui.cbPositionX, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageDock::positionXChanged);
	connect(ui.cbPositionY, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageDock::positionYChanged);
	connect(ui.sbPositionX, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ImageDock::customPositionXChanged);
	connect(ui.sbPositionY, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ImageDock::customPositionYChanged);
	connect(ui.cbHorizontalAlignment, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageDock::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageDock::verticalAlignmentChanged);
	connect(ui.sbRotation, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageDock::rotationChanged);

	connect(ui.chbVisible, &QCheckBox::clicked, this, &ImageDock::visibilityChanged);

	//Border
	connect(ui.cbBorderStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ImageDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &ImageDock::borderColorChanged);
	connect(ui.sbBorderWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ImageDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageDock::borderOpacityChanged);
}

void ImageDock::setImages(QList<Image*> list) {
	Lock lock(m_initializing);
	m_imageList = list;
	m_image = list.first();
	m_aspect = list.first();

	SET_NUMBER_LOCALE
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);

	//if there are more then one image in the list, disable the name and comment field in the tab "general"
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
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first image
	this->load();

	//init connections
	//General
	connect(m_image, &Image::aspectDescriptionChanged, this, &ImageDock::aspectDescriptionChanged);
	connect(m_image, &Image::fileNameChanged, this, &ImageDock::imageFileNameChanged);
	connect(m_image, &Image::opacityChanged, this, &ImageDock::imageOpacityChanged);
	connect(m_image, &Image::visibleChanged, this, &ImageDock::imageVisibleChanged);

	//Size
	connect(m_image, &Image::widthChanged, this, &ImageDock::imageWidthChanged);
	connect(m_image, &Image::heightChanged, this, &ImageDock::imageHeightChanged);
	connect(m_image, &Image::keepRatioChanged, this, &ImageDock::imageKeepRatioChanged);

	//Position
	connect(m_image, &Image::positionChanged, this, &ImageDock::imagePositionChanged);
	connect(m_image, &Image::horizontalAlignmentChanged, this, &ImageDock::imageHorizontalAlignmentChanged);
	connect(m_image, &Image::verticalAlignmentChanged, this, &ImageDock::imageVerticalAlignmentChanged);
	connect(m_image, &Image::rotationAngleChanged, this, &ImageDock::imageRotationAngleChanged);

	//Border
	connect(m_image, &Image::borderPenChanged, this, &ImageDock::imageBorderPenChanged);
	connect(m_image, &Image::borderOpacityChanged, this, &ImageDock::imageBorderOpacityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void ImageDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
}

void ImageDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	Lock lock(m_initializing);
	QString suffix;
	if (m_units == BaseDock::Units::Metric) {
		//convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbWidth->setValue(ui.sbWidth->value()*2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()*2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value()*2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value()*2.54);
	} else {
		//convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbWidth->setValue(ui.sbWidth->value()/2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()/2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value()/2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value()/2.54);
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
	KConfigGroup conf(KSharedConfig::openConfig(), "ImageDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leFileName->setText(path);
}

void ImageDock::fileNameChanged() {
	if (m_initializing)
		return;

	const QString& fileName = ui.leFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	for (auto* image : m_imageList)
		image->setFileName(fileName);
}

void ImageDock::opacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	for (auto* image : m_imageList)
		image->setOpacity(opacity);
}

//Size
void ImageDock::sizeChanged(int index) {
	Q_UNUSED(index);
}

void ImageDock::widthChanged(double value) {
	if (m_initializing)
		return;

	int width = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList)
		image->setWidth(width);
}

void ImageDock::heightChanged(double value) {
	if (m_initializing)
		return;

	int height = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* image : m_imageList)
		image->setHeight(height);
}

void ImageDock::keepRatioChanged(int state) {
	if (m_initializing)
		return;

	for (auto* image : m_imageList)
		image->setKeepRatio(state);
}

//Position
/*!
    called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void ImageDock::positionXChanged(int index) {
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionX->count()-1 )
		ui.sbPositionX->setEnabled(true);
	else
		ui.sbPositionX->setEnabled(false);

	if (m_initializing)
		return;

	WorksheetElement::PositionWrapper position = m_image->position();
	position.horizontalPosition = WorksheetElement::HorizontalPosition(index);
	for (auto* image : m_imageList)
		image->setPosition(position);
}

/*!
    called when label's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void ImageDock::positionYChanged(int index) {
	//Enable/disable the spinbox for the y-coordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionY->count()-1 )
		ui.sbPositionY->setEnabled(true);
	else
		ui.sbPositionY->setEnabled(false);

	if (m_initializing)
		return;

	WorksheetElement::PositionWrapper position = m_image->position();
	position.verticalPosition = WorksheetElement::VerticalPosition(index);
	for (auto* image : m_imageList)
		image->setPosition(position);
}

void ImageDock::customPositionXChanged(double value) {
	if (m_initializing)
		return;

	WorksheetElement::PositionWrapper position = m_image->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
	for (auto* image : m_imageList)
		image->setPosition(position);
}

void ImageDock::customPositionYChanged(double value) {
	if (m_initializing)
		return;

	WorksheetElement::PositionWrapper position = m_image->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
	for (auto* image : m_imageList)
		image->setPosition(position);
}

void ImageDock::horizontalAlignmentChanged(int index) {
	if (m_initializing)
		return;

	for (auto* image : m_imageList)
		image->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void ImageDock::verticalAlignmentChanged(int index) {
	if (m_initializing)
		return;

	for (auto* image : m_imageList)
		image->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}

void ImageDock::rotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* image : m_imageList)
		image->setRotationAngle(value);
}

void ImageDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* image : m_imageList)
		image->setVisible(state);
}

//border
void ImageDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setStyle(penStyle);
		image->setBorderPen(pen);
	}
}

void ImageDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setColor(color);
		image->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void ImageDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* image : m_imageList) {
		pen = image->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		image->setBorderPen(pen);
	}
}

void ImageDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* image : m_imageList)
		image->setBorderOpacity(opacity);
}

//*************************************************************
//********** SLOTs for changes triggered in Image *************
//*************************************************************
void ImageDock::imageFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.leFileName->setText(name);
	m_initializing = false;
}

void ImageDock::imageOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

//Size
void ImageDock::imageWidthChanged(int width) {
	m_initializing = true;
	ui.sbWidth->setValue( Worksheet::convertFromSceneUnits(width, m_worksheetUnit) );
	m_initializing = false;
}

void ImageDock::imageHeightChanged(int height) {
	m_initializing = true;
	ui.sbHeight->setValue( Worksheet::convertFromSceneUnits(height, m_worksheetUnit) );
	m_initializing = false;
}


void ImageDock::imageKeepRatioChanged(bool keep) {
	m_initializing = true;
	ui.chbKeepRatio->setChecked(keep);
	m_initializing = false;
}

//Position
void ImageDock::imagePositionChanged(const WorksheetElement::PositionWrapper& position) {
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit) );
	ui.cbPositionX->setCurrentIndex( static_cast<int>(position.horizontalPosition) );
	ui.cbPositionY->setCurrentIndex( static_cast<int>(position.verticalPosition) );
	m_initializing = false;
}

void ImageDock::imageHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment index) {
	m_initializing = true;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
	m_initializing = false;
}

void ImageDock::imageVerticalAlignmentChanged(WorksheetElement::VerticalAlignment index) {
	m_initializing = true;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
	m_initializing = false;
}

void ImageDock::imageRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void ImageDock::imageVisibleChanged(bool on) {
	m_initializing = true;
	ui.chbVisible->setChecked(on);
	m_initializing = false;
}

//Border
void ImageDock::imageBorderPenChanged(const QPen& pen) {
	m_initializing = true;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void ImageDock::imageBorderOpacityChanged(float value) {
	m_initializing = true;
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void ImageDock::load() {
	if (!m_image)
		return;

	m_initializing = true;

	ui.leFileName->setText(m_image->fileName());
	ui.chbVisible->setChecked(m_image->isVisible());

	//Size
	ui.sbWidth->setValue( Worksheet::convertFromSceneUnits(m_image->width(), m_worksheetUnit) );
	ui.sbHeight->setValue( Worksheet::convertFromSceneUnits(m_image->height(), m_worksheetUnit) );
	ui.chbKeepRatio->setChecked(m_image->keepRatio());

	//Position
	ui.cbPositionX->setCurrentIndex( (int) m_image->position().horizontalPosition );
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_image->position().point.x(), m_worksheetUnit) );
	ui.cbPositionY->setCurrentIndex( (int) m_image->position().verticalPosition );
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_image->position().point.y(), m_worksheetUnit) );

	ui.cbHorizontalAlignment->setCurrentIndex( (int) m_image->horizontalAlignment() );
	ui.cbVerticalAlignment->setCurrentIndex( (int) m_image->verticalAlignment() );
	ui.sbRotation->setValue( m_image->rotationAngle() );

	//Border
	ui.kcbBorderColor->setColor( m_image->borderPen().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_image->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_image->borderPen().widthF(), Worksheet::Unit::Point) );
	ui.sbBorderOpacity->setValue( round(m_image->borderOpacity()*100) );
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());

	m_initializing = false;
}
