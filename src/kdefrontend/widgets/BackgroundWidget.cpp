/*
	File                 : BackgroundWidget.cpp
	Project              : LabPlot
	Description          : background settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BackgroundWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <QCompleter>
// see https://gitlab.kitware.com/cmake/cmake/-/issues/21609
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QFileSystemModel>
#else
#include <QDirModel>
#endif
#include <QFile>

/*!
	\class BackgroundWidget
	\brief Widget for editing the properties of a Background object, mostly used in an appropriate dock widget.

	\ingroup kdefrontend
 */
BackgroundWidget::BackgroundWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);

	ui.cbColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	ui.leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));
#else
	ui.leFileName->setCompleter(new QCompleter(new QDirModel, this));
#endif

	connect(ui.chkEnabled, &QCheckBox::toggled, this, &BackgroundWidget::enabledChanged);
	connect(ui.cbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackgroundWidget::positionChanged);
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

void BackgroundWidget::setBackgrounds(const QList<Background*>& backgrounds) {
	m_backgrounds = backgrounds;
	m_background = m_backgrounds.first();
	m_prefix = m_background->prefix();

	CONDITIONAL_LOCK_RETURN;
	load();

	connect(m_background, &Background::enabledChanged, this, &BackgroundWidget::backgroundEnabledChanged);
	connect(m_background, &Background::positionChanged, this, &BackgroundWidget::backgroundPositionChanged);
	connect(m_background, &Background::typeChanged, this, &BackgroundWidget::backgroundTypeChanged);
	connect(m_background, &Background::colorStyleChanged, this, &BackgroundWidget::backgroundColorStyleChanged);
	connect(m_background, &Background::imageStyleChanged, this, &BackgroundWidget::backgroundImageStyleChanged);
	connect(m_background, &Background::brushStyleChanged, this, &BackgroundWidget::backgroundBrushStyleChanged);
	connect(m_background, &Background::firstColorChanged, this, &BackgroundWidget::backgroundFirstColorChanged);
	connect(m_background, &Background::secondColorChanged, this, &BackgroundWidget::backgroundSecondColorChanged);
	connect(m_background, &Background::fileNameChanged, this, &BackgroundWidget::backgroundFileNameChanged);
	connect(m_background, &Background::opacityChanged, this, &BackgroundWidget::backgroundOpacityChanged);
}

void BackgroundWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	adjustLayout();
}

/*!
 * this functions adjusts the width of the first column in the layout of BackgroundWidget
 * to the width of the first column in the layout of the parent widget
 * which BackgroundWidget is being embedded into.
 */
void BackgroundWidget::adjustLayout() {
	auto* parentGridLayout = dynamic_cast<QGridLayout*>(parentWidget()->layout());
	if (!parentGridLayout)
		return;

	auto* parentWidget = parentGridLayout->itemAtPosition(0, 0)->widget();
	if (!parentWidget)
		return;

	auto* gridLayout = static_cast<QGridLayout*>(layout());
	auto* widget = gridLayout->itemAtPosition(2, 0)->widget(); // use the third line, the first two are optional and not always visible

	if (parentWidget->width() >= widget->width())
		widget->setMinimumWidth(parentWidget->width());
	else
		parentWidget->setMinimumWidth(widget->width());
}

void BackgroundWidget::setEnabled(bool enabled) {
	ui.cbPosition->setEnabled(enabled);
	ui.chkEnabled->setEnabled(enabled);
	ui.cbType->setEnabled(enabled);
	ui.leFileName->setEnabled(enabled);
	ui.bOpen->setEnabled(enabled);
	ui.cbColorStyle->setEnabled(enabled);
	ui.cbImageStyle->setEnabled(enabled);
	ui.cbBrushStyle->setEnabled(enabled);
	ui.kcbFirstColor->setEnabled(enabled);
	ui.kcbSecondColor->setEnabled(enabled);
	ui.sbOpacity->setEnabled(enabled);
}

void BackgroundWidget::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbPosition->clear();
	ui.cbPosition->addItem(i18n("None"));
	ui.cbPosition->addItem(i18n("Above"));
	ui.cbPosition->addItem(i18n("Below"));
	ui.cbPosition->addItem(i18n("Zero Baseline"));
	ui.cbPosition->addItem(i18n("Left"));
	ui.cbPosition->addItem(i18n("Right"));

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
//******** SLOTs for changes triggered in BackgroundWidget ****
//*************************************************************
void BackgroundWidget::enabledChanged(bool state) {
	ui.cbType->setEnabled(state);
	ui.cbColorStyle->setEnabled(state);
	ui.cbBrushStyle->setEnabled(state);
	ui.cbImageStyle->setEnabled(state);
	ui.kcbFirstColor->setEnabled(state);
	ui.kcbSecondColor->setEnabled(state);
	ui.leFileName->setEnabled(state);
	ui.bOpen->setEnabled(state);
	ui.sbOpacity->setEnabled(state);

	CONDITIONAL_LOCK_RETURN;

	for (auto* background : m_backgrounds)
		background->setEnabled(state);
}

void BackgroundWidget::positionChanged(int index) {
	if (!m_background || !m_background->positionAvailable())
		return;

	const auto position{Background::Position(index)};
	bool b = (position != Background::Position::No);
	setEnabled(b); // call this to enable/disable the properties widget depending on the position value
	ui.cbPosition->setEnabled(true); // and enable position only

	CONDITIONAL_LOCK_RETURN;
	for (auto* background : m_backgrounds)
		background->setPosition(position);
}

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

	CONDITIONAL_LOCK_RETURN;
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

	CONDITIONAL_LOCK_RETURN;

	int size = m_backgrounds.size();
	if (size > 1) {
		m_background->beginMacro(i18n("%1 elements: background color style changed", size));
		for (auto* background : m_backgrounds)
			background->setColorStyle(style);
		m_background->endMacro();
	} else
		m_background->setColorStyle(style);
}

void BackgroundWidget::imageStyleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto style = (Background::ImageStyle)index;
	for (auto* background : m_backgrounds)
		background->setImageStyle(style);
}

void BackgroundWidget::brushStyleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto style = (Qt::BrushStyle)index;
	for (auto* background : m_backgrounds)
		background->setBrushStyle(style);
}

void BackgroundWidget::firstColorChanged(const QColor& c) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* background : m_backgrounds)
		background->setFirstColor(c);
}

void BackgroundWidget::secondColorChanged(const QColor& c) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* background : m_backgrounds)
		background->setSecondColor(c);
}

void BackgroundWidget::opacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

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
	const QString& fileName = ui.leFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);

	CONDITIONAL_LOCK_RETURN;

	for (auto* background : m_backgrounds)
		background->setFileName(fileName);
}

//*************************************************************
//********* SLOTs for changes triggered in Background *********
//*************************************************************
void BackgroundWidget::backgroundEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkEnabled->setChecked(status);
}

void BackgroundWidget::backgroundPositionChanged(Background::Position position) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPosition->setCurrentIndex((int)position);
}

void BackgroundWidget::backgroundTypeChanged(Background::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
}

void BackgroundWidget::backgroundColorStyleChanged(Background::ColorStyle style) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbColorStyle->setCurrentIndex(static_cast<int>(style));
}

void BackgroundWidget::backgroundImageStyleChanged(Background::ImageStyle style) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbImageStyle->setCurrentIndex(static_cast<int>(style));
}

void BackgroundWidget::backgroundBrushStyleChanged(Qt::BrushStyle style) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbBrushStyle->setCurrentIndex(style);
}

void BackgroundWidget::backgroundFirstColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbFirstColor->setColor(color);
}

void BackgroundWidget::backgroundSecondColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbSecondColor->setColor(color);
}

void BackgroundWidget::backgroundFileNameChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	ui.leFileName->setText(name);
}

void BackgroundWidget::backgroundOpacityChanged(float opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOpacity->setValue(std::round(opacity * 100));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BackgroundWidget::load() {
	ui.cbType->setCurrentIndex((int)m_background->type());
	ui.cbColorStyle->setCurrentIndex((int)m_background->colorStyle());
	ui.cbImageStyle->setCurrentIndex((int)m_background->imageStyle());
	ui.cbBrushStyle->setCurrentIndex((int)m_background->brushStyle());
	ui.leFileName->setText(m_background->fileName());
	ui.kcbFirstColor->setColor(m_background->firstColor());
	ui.kcbSecondColor->setColor(m_background->secondColor());
	ui.sbOpacity->setValue(std::round(m_background->opacity() * 100));

	// optional parameters
	bool visible = m_background->enabledAvailable();
	ui.lEnabled->setVisible(visible);
	ui.chkEnabled->setVisible(visible);

	visible = m_background->positionAvailable();
	ui.lPosition->setVisible(visible);
	ui.cbPosition->setVisible(visible);

	if (m_background->enabledAvailable())
		ui.chkEnabled->setChecked(m_background->enabled());

	if (m_background->enabledAvailable())
		ui.cbPosition->setCurrentIndex((int)m_background->position());

	// highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_background->fileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leFileName, invalid);
}

void BackgroundWidget::loadConfig(const KConfigGroup& group) {
	ui.cbType->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Type"), (int)m_background->type()));
	ui.cbColorStyle->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("ColorStyle"), (int)m_background->colorStyle()));
	ui.cbImageStyle->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("ImageStyle"), (int)m_background->imageStyle()));
	ui.cbBrushStyle->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("BrushStyle"), (int)m_background->brushStyle()));
	ui.leFileName->setText(group.readEntry(m_prefix + QStringLiteral("FileName"), m_background->fileName()));
	ui.kcbFirstColor->setColor(group.readEntry(m_prefix + QStringLiteral("FirstColor"), m_background->firstColor()));
	ui.kcbSecondColor->setColor(group.readEntry(m_prefix + QStringLiteral("SecondColor"), m_background->secondColor()));
	ui.sbOpacity->setValue(std::round(group.readEntry(m_prefix + QStringLiteral("Opacity"), m_background->opacity()) * 100));

	// optional parameters
	if (m_background->enabledAvailable())
		ui.chkEnabled->setChecked(group.readEntry(m_prefix + QStringLiteral("Enabled"), m_background->enabled()));

	if (m_background->positionAvailable())
		ui.cbPosition->setCurrentIndex(group.readEntry(m_prefix + QStringLiteral("Position"), (int)m_background->position()));
}

void BackgroundWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry(m_prefix + QStringLiteral("Type"), ui.cbType->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("ColorStyle"), ui.cbColorStyle->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("ImageStyle"), ui.cbImageStyle->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("BrushStyle"), ui.cbBrushStyle->currentIndex());
	group.writeEntry(m_prefix + QStringLiteral("FileName"), ui.leFileName->text());
	group.writeEntry(m_prefix + QStringLiteral("FirstColor"), ui.kcbFirstColor->color());
	group.writeEntry(m_prefix + QStringLiteral("SecondColor"), ui.kcbSecondColor->color());
	group.writeEntry(m_prefix + QStringLiteral("Opacity"), ui.sbOpacity->value() / 100.0);

	// optional parameters
	if (m_background->enabledAvailable())
		group.writeEntry(m_prefix + QStringLiteral("Enabled"), ui.chkEnabled->isChecked());

	if (m_background->positionAvailable())
		group.writeEntry(m_prefix + QStringLiteral("Position"), ui.cbPosition->currentIndex());
}
