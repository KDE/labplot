/*
	File                 : ScriptButtonDock.cpp
	Project              : LabPlot
	Description          : widget to edit the properties of a script button
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptButtonDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Settings.h"
#include "backend/script/Script.h"
#include "backend/worksheet/ScriptButton.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalization>
#include <KLocalizedString>

#include <gsl/gsl_const_cgs.h>

/*!
  \class ScriptButtonDock
  \brief  Provides a widget for editing the properties of the worksheets button element.

  \ingroup frontend
*/

ScriptButtonDock::ScriptButtonDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	cbScript = new TreeViewComboBox(this);
	auto* gridLayout = static_cast<QGridLayout*>(this->layout());
	gridLayout->addWidget(cbScript, 3, 2);

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
	connect(ui.leText, &TimedLineEdit::textEdited, this, &ScriptButtonDock::textChanged);
	connect(cbScript, &TreeViewComboBox::currentModelIndexChanged, this, &ScriptButtonDock::scriptChanged);
	connect(ui.kfontRequester, &KFontRequester::fontSelected, this, &ScriptButtonDock::fontChanged);
	connect(ui.kcbFontColor, &KColorButton::changed, this, &ScriptButtonDock::fontColorChanged);
	connect(ui.kcbBackgroundColor, &KColorButton::changed, this, &ScriptButtonDock::backgroundColorChanged);

	// Size
	connect(ui.sbWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ScriptButtonDock::widthChanged);
	connect(ui.sbHeight, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ScriptButtonDock::heightChanged);

	// Position
	connect(ui.cbPositionX, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScriptButtonDock::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScriptButtonDock::positionYChanged);
	connect(ui.sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ScriptButtonDock::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ScriptButtonDock::customPositionYChanged);
	connect(ui.cbHorizontalAlignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScriptButtonDock::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScriptButtonDock::verticalAlignmentChanged);
}

void ScriptButtonDock::setModel() {
	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Script});
	cbScript->setTopLevelClasses({AspectType::Script});
	cbScript->setModel(model);
}

void ScriptButtonDock::setScriptButtons(QList<ScriptButton*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_scriptButtons = list;
	m_scriptButton = list.first();
	setAspects(list);
	setModel();


	// show the properties of the first button
	this->load();

	// init connections
	connect(m_scriptButton, &ScriptButton::scriptChanged, this, &ScriptButtonDock::buttonScriptChanged);
	connect(m_scriptButton, &ScriptButton::textChanged, this, &ScriptButtonDock::buttonTextChanged);
	connect(m_scriptButton, &ScriptButton::fontChanged, this, &ScriptButtonDock::buttonFontChanged);
	connect(m_scriptButton, &ScriptButton::textColorChanged, this, &ScriptButtonDock::buttonTextColorChanged);
	connect(m_scriptButton, &ScriptButton::backgroundColorChanged, this, &ScriptButtonDock::buttonBackgroundColorChanged);

	// Size
	connect(m_scriptButton, &ScriptButton::widthChanged, this, &ScriptButtonDock::buttonWidthChanged);
	connect(m_scriptButton, &ScriptButton::heightChanged, this, &ScriptButtonDock::buttonHeightChanged);

	// Position
	connect(m_scriptButton, &ScriptButton::positionChanged, this, &ScriptButtonDock::buttonPositionChanged);
	connect(m_scriptButton, &ScriptButton::horizontalAlignmentChanged, this, &ScriptButtonDock::buttonHorizontalAlignmentChanged);
	connect(m_scriptButton, &ScriptButton::verticalAlignmentChanged, this, &ScriptButtonDock::buttonVerticalAlignmentChanged);
}

void ScriptButtonDock::updateUnits() {
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
void ScriptButtonDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
}

void ScriptButtonDock::retranslateUi() {
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
//**** SLOTs for changes triggered in ScriptButtonDock ********
//*************************************************************
void ScriptButtonDock::textChanged() {
	CONDITIONAL_RETURN_NO_LOCK;
	const auto& text = ui.leText->text();
	for (auto* button : m_scriptButtons)
		button->setText(text);
}

void ScriptButtonDock::scriptChanged(const QModelIndex&) {
	CONDITIONAL_RETURN_NO_LOCK;

	auto* script = dynamic_cast<Script*>(cbScript->currentAspect());
	for (auto* button : m_scriptButtons)
		button->setScript(script);
}

void ScriptButtonDock::fontChanged(const QFont& font) {
	CONDITIONAL_RETURN_NO_LOCK;
	for (auto* button : m_scriptButtons)
		button->setFont(font);
}

void ScriptButtonDock::fontColorChanged(const QColor& color) {
	CONDITIONAL_RETURN_NO_LOCK;
	for (auto* button : m_scriptButtons)
		button->setTextColor(color);
}

void ScriptButtonDock::backgroundColorChanged(const QColor& color) {
	CONDITIONAL_RETURN_NO_LOCK;
	for (auto* button : m_scriptButtons)
		button->setBackgroundColor(color);
}

// Size
void ScriptButtonDock::widthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;
	const int width = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* button : m_scriptButtons)
		button->setWidth(width);
}

void ScriptButtonDock::heightChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;
	const int height = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* button : m_scriptButtons)
		button->setHeight(height);
}

// Position
/*!
	called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void ScriptButtonDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_scriptButton->position();
	position.horizontalPosition = WorksheetElement::HorizontalPosition(index);
	for (auto* button : m_scriptButtons)
		button->setPosition(position);
}

/*!
	called when label's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void ScriptButtonDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_scriptButton->position();
	position.verticalPosition = WorksheetElement::VerticalPosition(index);
	for (auto* button : m_scriptButtons)
		button->setPosition(position);
}

void ScriptButtonDock::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double x = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* button : m_scriptButtons) {
		auto position = button->position();
		position.point.setX(x);
		button->setPosition(position);
	}
}

void ScriptButtonDock::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double y = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* button : m_scriptButtons) {
		auto position = button->position();
		position.point.setY(y);
		button->setPosition(position);
	}
}

void ScriptButtonDock::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* button : m_scriptButtons)
		button->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void ScriptButtonDock::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* button : m_scriptButtons)
		button->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}


//*************************************************************
//********** SLOTs for changes triggered in ScriptButton *************
//*************************************************************
void ScriptButtonDock::buttonTextChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;
	ui.leText->setText(text);
}

void ScriptButtonDock::buttonScriptChanged(Script*) {
	CONDITIONAL_LOCK_RETURN;
	cbScript->setAspect(m_scriptButton->script());
}

void ScriptButtonDock::buttonFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfontRequester->setFont(font);
}

void ScriptButtonDock::buttonTextColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbFontColor->setColor(color);
}

void ScriptButtonDock::buttonBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbBackgroundColor->setColor(color);
}

// Size
void ScriptButtonDock::buttonWidthChanged(int width) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(width, m_worksheetUnit));
}

void ScriptButtonDock::buttonHeightChanged(int height) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(height, m_worksheetUnit));
}

// Position
void ScriptButtonDock::buttonPositionChanged(const WorksheetElement::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.x(), m_units), m_worksheetUnit));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(position.point.y(), m_units), m_worksheetUnit));
	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
}

void ScriptButtonDock::buttonHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void ScriptButtonDock::buttonVerticalAlignmentChanged(WorksheetElement::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}


//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void ScriptButtonDock::load() {
	if (!m_scriptButton)
		return;

	cbScript->setAspect(m_scriptButton->script());
	ui.leText->setText(m_scriptButton->text());
	ui.kfontRequester->setFont(m_scriptButton->font());
	ui.kcbFontColor->setColor(m_scriptButton->textColor());
	ui.kcbBackgroundColor->setColor(m_scriptButton->backgroundColor());

	// Size
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_scriptButton->width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_scriptButton->height(), m_worksheetUnit));

	// // Position
	ui.cbPositionX->setCurrentIndex((int)m_scriptButton->position().horizontalPosition);
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_scriptButton->position().point.x(), m_units), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_scriptButton->position().verticalPosition);
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_scriptButton->position().point.y(), m_units), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_scriptButton->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_scriptButton->verticalAlignment());
}
