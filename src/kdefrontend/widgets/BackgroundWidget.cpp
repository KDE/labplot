/*
	File                 : BackgroundWidget.cpp
	Project              : LabPlot
	Description          : background settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BackgroundWidget.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "kdefrontend/GuiTools.h"

#include <QCompleter>
#include <QDirModel>
#include <QFile>

/*!
	\class BackgroundWidget
	\brief Widget for editing the properties of a Background object, mostly used in an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the corresponding KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */
BackgroundWidget::BackgroundWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	ui.bOpen->setIcon(QIcon::fromTheme("document-open"));
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackgroundWidget::typeChanged);
	connect(ui.cbColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackgroundWidget::colorStyleChanged);
	connect(ui.cbImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackgroundWidget::imageStyleChanged);
	connect(ui.cbBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackgroundWidget::brushStyleChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &BackgroundWidget::selectFile);
	connect(ui.leFileName, &QLineEdit::returnPressed, this, &BackgroundWidget::fileNameChanged);
	connect(ui.leFileName, &QLineEdit::textChanged, this, &BackgroundWidget::fileNameChanged);
	connect(ui.kcbFirstColor, &KColorButton::changed, this, &BackgroundWidget::firstColorChanged);
	connect(ui.kcbSecondColor, &KColorButton::changed, this, &BackgroundWidget::secondColorChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BackgroundWidget::opacityChanged);

	retranslateUi();
}

void BackgroundWidget::setBackgrounds(QList<Background*> backgrounds) {
	m_backgrounds = backgrounds;
	m_background = m_backgrounds.first();

	load();

	connect(m_background, &Background::typeChanged, this, &BackgroundWidget::backgroundTypeChanged);
	connect(m_background, &Background::colorStyleChanged, this, &BackgroundWidget::backgroundColorStyleChanged);
	connect(m_background, &Background::imageStyleChanged, this, &BackgroundWidget::backgroundImageStyleChanged);
	connect(m_background, &Background::brushStyleChanged, this, &BackgroundWidget::backgroundBrushStyleChanged);
	connect(m_background, &Background::firstColorChanged, this, &BackgroundWidget::backgroundFirstColorChanged);
	connect(m_background, &Background::secondColorChanged, this, &BackgroundWidget::backgroundSecondColorChanged);
	connect(m_background, &Background::fileNameChanged, this, &BackgroundWidget::backgroundFileNameChanged);
	connect(m_background, &Background::opacityChanged, this, &BackgroundWidget::backgroundOpacityChanged);
}

void BackgroundWidget::retranslateUi() {
	Lock lock(m_initializing);

	ui.cbType->clear();
	ui.cbType->addItem(i18n("Color"));
	ui.cbType->addItem(i18n("Image"));
	ui.cbType->addItem(i18n("Pattern"));

	ui.cbColorStyle->clear();
	ui.cbColorStyle->addItem(i18n("Single Color"));
	ui.cbColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbImageStyle->clear();
	ui.cbImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbImageStyle->addItem(i18n("Scaled"));
	ui.cbImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbImageStyle->addItem(i18n("Centered"));
	ui.cbImageStyle->addItem(i18n("Tiled"));
	ui.cbImageStyle->addItem(i18n("Center Tiled"));
	GuiTools::updateBrushStyles(ui.cbBrushStyle, Qt::SolidPattern);
}

//*************************************************************
//******** SLOTs for changes triggered in BackgroundWidget ********
//*************************************************************
void BackgroundWidget::typeChanged(int index) {
	if (index == -1)
		return;

	auto type = (Background::Type)index;

	if (type == Background::Type::Color) {
		ui.lColorStyle->show();
		ui.cbColorStyle->show();
		ui.lImageStyle->hide();
		ui.cbImageStyle->hide();
		ui.lBrushStyle->hide();
		ui.cbBrushStyle->hide();

		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bOpen->hide();

		ui.lFirstColor->show();
		ui.kcbFirstColor->show();

		auto style = (Background::ColorStyle)ui.cbColorStyle->currentIndex();
		if (style == Background::ColorStyle::SingleColor) {
			ui.lFirstColor->setText(i18n("Color:"));
			ui.lSecondColor->hide();
			ui.kcbSecondColor->hide();
		} else {
			ui.lFirstColor->setText(i18n("First color:"));
			ui.lSecondColor->show();
			ui.kcbSecondColor->show();
		}
	} else if (type == Background::Type::Image) {
		ui.lFirstColor->hide();
		ui.kcbFirstColor->hide();
		ui.lSecondColor->hide();
		ui.kcbSecondColor->hide();

		ui.lColorStyle->hide();
		ui.cbColorStyle->hide();
		ui.lImageStyle->show();
		ui.cbImageStyle->show();
		ui.lBrushStyle->hide();
		ui.cbBrushStyle->hide();
		ui.lFileName->show();
		ui.leFileName->show();
		ui.bOpen->show();
	} else if (type == Background::Type::Pattern) {
		ui.lFirstColor->setText(i18n("Color:"));
		ui.lFirstColor->show();
		ui.kcbFirstColor->show();
		ui.lSecondColor->hide();
		ui.kcbSecondColor->hide();

		ui.lColorStyle->hide();
		ui.cbColorStyle->hide();
		ui.lImageStyle->hide();
		ui.cbImageStyle->hide();
		ui.lBrushStyle->show();
		ui.cbBrushStyle->show();
		ui.lFileName->hide();
		ui.leFileName->hide();
		ui.bOpen->hide();
	}

	if (m_initializing)
		return;

	for (auto* background : m_backgrounds)
		background->setType(type);
}

void BackgroundWidget::colorStyleChanged(int index) {
	if (index == -1)
		return;

	auto style = (Background::ColorStyle)index;

	if (style == Background::ColorStyle::SingleColor) {
		ui.lFirstColor->setText(i18n("Color:"));
		ui.lSecondColor->hide();
		ui.kcbSecondColor->hide();
	} else {
		ui.lFirstColor->setText(i18n("First color:"));
		ui.lSecondColor->show();
		ui.kcbSecondColor->show();
	}

	if (m_initializing)
		return;

	int size = m_backgrounds.size();
	if (size > 1) {
		m_background->beginMacro(i18n("%1 worksheets: background color style changed", size));
		for (auto* background : m_backgrounds)
			background->setColorStyle(style);
		m_background->endMacro();
	} else
		m_background->setColorStyle(style);
}

void BackgroundWidget::imageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Background::ImageStyle)index;
	for (auto* background : m_backgrounds)
		background->setImageStyle(style);
}

void BackgroundWidget::brushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* background : m_backgrounds)
		background->setBrushStyle(style);
}

void BackgroundWidget::firstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* background : m_backgrounds)
		background->setFirstColor(c);
}

void BackgroundWidget::secondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* background : m_backgrounds)
		background->setSecondColor(c);
}

void BackgroundWidget::opacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value / 100;
	for (auto* background : m_backgrounds)
		background->setOpacity(opacity);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void BackgroundWidget::selectFile() {
	const QString& path = GuiTools::openImageFile(QLatin1String("WorksheetDock"));
	if (path.isEmpty())
		return;

	ui.leFileName->setText(path);
}

void BackgroundWidget::fileNameChanged() {
	if (m_initializing)
		return;

	const QString& fileName = ui.leFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	for (auto* background : m_backgrounds)
		background->setFileName(fileName);
}

//*************************************************************
//********* SLOTs for changes triggered in Background *********
//*************************************************************
void BackgroundWidget::backgroundTypeChanged(Background::Type type) {
	m_initializing = true;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void BackgroundWidget::backgroundColorStyleChanged(Background::ColorStyle style) {
	m_initializing = true;
	ui.cbColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void BackgroundWidget::backgroundImageStyleChanged(Background::ImageStyle style) {
	m_initializing = true;
	ui.cbImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void BackgroundWidget::backgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void BackgroundWidget::backgroundFirstColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbFirstColor->setColor(color);
	m_initializing = false;
}

void BackgroundWidget::backgroundSecondColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbSecondColor->setColor(color);
	m_initializing = false;
}

void BackgroundWidget::backgroundFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.leFileName->setText(name);
	m_initializing = false;
}

void BackgroundWidget::backgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbOpacity->setValue(qRound(opacity * 100.0));
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BackgroundWidget::load() {
	const Lock lock(m_initializing);

	bool enabled = m_background->showEnabled();
	ui.lEnabled->setVisible(enabled);
	ui.chkEnabled->setVisible(enabled);

	// highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_background->fileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	ui.cbType->setCurrentIndex((int)m_background->type());
	ui.cbColorStyle->setCurrentIndex((int)m_background->colorStyle());
	ui.cbImageStyle->setCurrentIndex((int)m_background->imageStyle());
	ui.cbBrushStyle->setCurrentIndex((int)m_background->brushStyle());
	ui.leFileName->setText(m_background->fileName());
	ui.kcbFirstColor->setColor(m_background->firstColor());
	ui.kcbSecondColor->setColor(m_background->secondColor());
	ui.sbOpacity->setValue(qRound(m_background->opacity() * 100));
}

void BackgroundWidget::loadConfig(const KConfigGroup& group) {
	const Lock lock(m_initializing);

	ui.cbType->setCurrentIndex(group.readEntry("BackgroundType", (int)m_background->type()));
	ui.cbColorStyle->setCurrentIndex(group.readEntry("BackgroundColorStyle", (int)m_background->colorStyle()));
	ui.cbImageStyle->setCurrentIndex(group.readEntry("BackgroundImageStyle", (int)m_background->imageStyle()));
	ui.cbBrushStyle->setCurrentIndex(group.readEntry("BackgroundBrushStyle", (int)m_background->brushStyle()));
	ui.leFileName->setText(group.readEntry("BackgroundFileName", m_background->fileName()));
	ui.kcbFirstColor->setColor(group.readEntry("BackgroundFirstColor", m_background->firstColor()));
	ui.kcbSecondColor->setColor(group.readEntry("BackgroundSecondColor", m_background->secondColor()));
	ui.sbOpacity->setValue(qRound(group.readEntry("BackgroundOpacity", m_background->opacity()) * 100));
}

void BackgroundWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry("BackgroundType", ui.cbType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbOpacity->value() / 100.0);;
}
