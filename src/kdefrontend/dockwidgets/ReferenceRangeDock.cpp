/*
	File                 : ReferenceRangeDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ReferenceRangeDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"

#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"

#include <KConfig>
#include <KLocalizedString>
#include <QFile>

ReferenceRangeDock::ReferenceRangeDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	ui.lePositionStart->setValidator(new QDoubleValidator(ui.lePositionStart));
	ui.lePositionEnd->setValidator(new QDoubleValidator(ui.lePositionEnd));

	// background
	ui.cbBackgroundType->addItem(i18n("Color"));
	ui.cbBackgroundType->addItem(i18n("Image"));
	ui.cbBackgroundType->addItem(i18n("Pattern"));

	ui.cbBackgroundColorStyle->addItem(i18n("Single Color"));
	ui.cbBackgroundColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("Centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("Tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Center Tiled"));

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	SET_NUMBER_LOCALE
	ui.sbBorderWidth->setLocale(numberLocale);

	// SLOTS
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &ReferenceRangeDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ReferenceRangeDock::commentChanged);

	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::orientationChanged);
	connect(ui.lePositionStart, &QLineEdit::textChanged, this, &ReferenceRangeDock::positionLogicalStartChanged);
	connect(ui.lePositionEnd, &QLineEdit::textChanged, this, &ReferenceRangeDock::positionLogicalEndChanged);
	connect(ui.dtePositionStart, &QDateTimeEdit::dateTimeChanged, this, &ReferenceRangeDock::positionLogicalDateTimeStartChanged);
	connect(ui.dtePositionEnd, &QDateTimeEdit::dateTimeChanged, this, &ReferenceRangeDock::positionLogicalDateTimeEndChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::plotRangeChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &ReferenceRangeDock::visibilityChanged);

	// Background
	connect(ui.cbBackgroundType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::backgroundTypeChanged);
	connect(ui.cbBackgroundColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::backgroundColorStyleChanged);
	connect(ui.cbBackgroundImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::backgroundImageStyleChanged);
	connect(ui.cbBackgroundBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::backgroundBrushStyleChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &ReferenceRangeDock::selectFile);
	connect(ui.leBackgroundFileName, &QLineEdit::textChanged, this, &ReferenceRangeDock::fileNameChanged);
	connect(ui.kcbBackgroundFirstColor, &KColorButton::changed, this, &ReferenceRangeDock::backgroundFirstColorChanged);
	connect(ui.kcbBackgroundSecondColor, &KColorButton::changed, this, &ReferenceRangeDock::backgroundSecondColorChanged);
	connect(ui.sbBackgroundOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ReferenceRangeDock::backgroundOpacityChanged);

	// Border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReferenceRangeDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &ReferenceRangeDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ReferenceRangeDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ReferenceRangeDock::borderOpacityChanged);
	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, QColor(Qt::black));
	m_initializing = false;
}

void ReferenceRangeDock::setReferenceRanges(QList<ReferenceRange*> list) {
	m_initializing = true;
	m_rangeList = list;
	m_range = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_range);

	// if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_range->name());
		ui.teComment->setText(m_range->comment());
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

	// show the properties of the first reference range
	this->load();

	updatePlotRanges();

	// SIGNALs/SLOTs
	connect(m_range, &AbstractAspect::aspectDescriptionChanged, this, &ReferenceRangeDock::aspectDescriptionChanged);
	connect(m_range, &WorksheetElement::plotRangeListChanged, this, &ReferenceRangeDock::updatePlotRanges);
	connect(m_range, &ReferenceRange::visibleChanged, this, &ReferenceRangeDock::rangeVisibilityChanged);

	// position
	connect(m_range, &ReferenceRange::orientationChanged, this, &ReferenceRangeDock::rangeOrientationChanged);
	connect(m_range, &ReferenceRange::positionLogicalStartChanged, this, &ReferenceRangeDock::rangePositionLogicalStartChanged);
	connect(m_range, &ReferenceRange::positionLogicalEndChanged, this, &ReferenceRangeDock::rangePositionLogicalEndChanged);

	// background
	connect(m_range, &ReferenceRange::backgroundTypeChanged, this, &ReferenceRangeDock::rangeBackgroundTypeChanged);
	connect(m_range, &ReferenceRange::backgroundColorStyleChanged, this, &ReferenceRangeDock::rangeBackgroundColorStyleChanged);
	connect(m_range, &ReferenceRange::backgroundImageStyleChanged, this, &ReferenceRangeDock::rangeBackgroundImageStyleChanged);
	connect(m_range, &ReferenceRange::backgroundBrushStyleChanged, this, &ReferenceRangeDock::rangeBackgroundBrushStyleChanged);
	connect(m_range, &ReferenceRange::backgroundFirstColorChanged, this, &ReferenceRangeDock::rangeBackgroundFirstColorChanged);
	connect(m_range, &ReferenceRange::backgroundSecondColorChanged, this, &ReferenceRangeDock::rangeBackgroundSecondColorChanged);
	connect(m_range, &ReferenceRange::backgroundFileNameChanged, this, &ReferenceRangeDock::rangeBackgroundFileNameChanged);
	connect(m_range, &ReferenceRange::backgroundOpacityChanged, this, &ReferenceRangeDock::rangeBackgroundOpacityChanged);

	// border
	connect(m_range, &ReferenceRange::borderPenChanged, this, &ReferenceRangeDock::rangeBorderPenChanged);
	connect(m_range, &ReferenceRange::borderOpacityChanged, this, &ReferenceRangeDock::rangeBorderOpacityChanged);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void ReferenceRangeDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbBorderWidth->setLocale(numberLocale);

	Lock lock(m_initializing);
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		if (plot->yRangeFormat() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().y()));
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalEnd().y()));
		}
		// TODO datetime
	} else {
		if (plot->xRangeFormat() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().x()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().x()));
		}
		// TODO datetime
	}
}

void ReferenceRangeDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
}

//**********************************************************
//*** SLOTs for changes triggered in ReferenceRangeDock *****
//**********************************************************
// Position
void ReferenceRangeDock::orientationChanged(int index) {
	auto orientation{ReferenceRange::Orientation(index)};
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	bool numeric;
	if (orientation == ReferenceRange::Orientation::Horizontal) {
		ui.lPositionStart->setText(QLatin1String("Start y:"));
		ui.lPositionEnd->setText(QLatin1String("End y:"));
		ui.lPositionDateTimeStart->setText(QLatin1String("Start y:"));
		ui.lPositionDateTimeEnd->setText(QLatin1String("End y:"));
		numeric = (plot->yRangeFormat() == RangeT::Format::Numeric);
	} else {
		ui.lPositionStart->setText(QLatin1String("Start x:"));
		ui.lPositionEnd->setText(QLatin1String("End x:"));
		ui.lPositionDateTimeStart->setText(QLatin1String("Start x:"));
		ui.lPositionDateTimeEnd->setText(QLatin1String("End x:"));
		numeric = (plot->xRangeFormat() == RangeT::Format::Numeric);
	}

	ui.lPositionStart->setVisible(numeric);
	ui.lePositionStart->setVisible(numeric);
	ui.lPositionEnd->setVisible(numeric);
	ui.lePositionEnd->setVisible(numeric);
	ui.lPositionDateTimeStart->setVisible(!numeric);
	ui.dtePositionStart->setVisible(!numeric);
	ui.lPositionDateTimeEnd->setVisible(!numeric);
	ui.dtePositionEnd->setVisible(!numeric);

	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setOrientation(orientation);

	// call these slots to show the start and end values depending on the new orientation
	rangePositionLogicalStartChanged(m_range->positionLogicalStart());
	rangePositionLogicalEndChanged(m_range->positionLogicalEnd());
}

void ReferenceRangeDock::positionLogicalStartChanged(const QString& value) {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* range : m_rangeList) {
			auto positionLogical = range->positionLogicalStart();
			if (range->orientation() == ReferenceRange::Orientation::Horizontal)
				positionLogical.setY(pos);
			else
				positionLogical.setX(pos);
			range->setPositionLogicalStart(positionLogical);
		}
	}
}

void ReferenceRangeDock::positionLogicalEndChanged(const QString& value) {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* range : m_rangeList) {
			auto positionLogical = range->positionLogicalEnd();
			if (range->orientation() == ReferenceRange::Orientation::Horizontal)
				positionLogical.setY(pos);
			else
				positionLogical.setX(pos);
			range->setPositionLogicalEnd(positionLogical);
		}
	}
}

void ReferenceRangeDock::positionLogicalDateTimeStartChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 pos = dateTime.toMSecsSinceEpoch();
	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalStart();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalStart(positionLogical);
	}
}

void ReferenceRangeDock::positionLogicalDateTimeEndChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 pos = dateTime.toMSecsSinceEpoch();
	for (auto* range : m_rangeList) {
		auto positionLogical = range->positionLogicalEnd();
		if (range->orientation() == ReferenceRange::Orientation::Horizontal)
			positionLogical.setY(pos);
		else
			positionLogical.setX(pos);
		range->setPositionLogicalEnd(positionLogical);
	}
}

void ReferenceRangeDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setVisible(state);
}

// background
void ReferenceRangeDock::backgroundTypeChanged(int index) {
	auto type = (WorksheetElement::BackgroundType)index;

	if (type == WorksheetElement::BackgroundType::Color) {
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();

		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		auto style = (WorksheetElement::BackgroundColorStyle)ui.cbBackgroundColorStyle->currentIndex();
		if (style == WorksheetElement::BackgroundColorStyle::SingleColor) {
			ui.lBackgroundFirstColor->setText(i18n("Color:"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		} else {
			ui.lBackgroundFirstColor->setText(i18n("First color:"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	} else if (type == WorksheetElement::BackgroundType::Image) {
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.leBackgroundFileName->show();
		ui.bOpen->show();

		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else if (type == WorksheetElement::BackgroundType::Pattern) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}

	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setBackgroundType(type);
}

void ReferenceRangeDock::backgroundColorStyleChanged(int index) {
	auto style = (WorksheetElement::BackgroundColorStyle)index;

	if (style == WorksheetElement::BackgroundColorStyle::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else {
		ui.lBackgroundFirstColor->setText(i18n("First color:"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
	}

	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setBackgroundColorStyle(style);
}

void ReferenceRangeDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (WorksheetElement::BackgroundImageStyle)index;
	for (auto* range : m_rangeList)
		range->setBackgroundImageStyle(style);
}

void ReferenceRangeDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* range : m_rangeList)
		range->setBackgroundBrushStyle(style);
}

void ReferenceRangeDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setBackgroundFirstColor(c);
}

void ReferenceRangeDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* range : m_rangeList)
		range->setBackgroundSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void ReferenceRangeDock::selectFile() {
	const QString& path = GuiTools::openImageFile(QLatin1String("ReferenceRangeDock"));
	if (path.isEmpty())
		return;

	ui.leBackgroundFileName->setText(path);
}

void ReferenceRangeDock::fileNameChanged() {
	if (m_initializing)
		return;

	const QString& fileName = ui.leBackgroundFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

	for (auto* range : m_rangeList)
		range->setBackgroundFileName(fileName);
}

void ReferenceRangeDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value / 100.;
	for (auto* range : m_rangeList)
		range->setBackgroundOpacity(opacity);
}

// border
void ReferenceRangeDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* range : m_rangeList) {
		pen = range->borderPen();
		pen.setStyle(penStyle);
		range->setBorderPen(pen);
	}
}

void ReferenceRangeDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* range : m_rangeList) {
		pen = range->borderPen();
		pen.setColor(color);
		range->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void ReferenceRangeDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* range : m_rangeList) {
		pen = range->borderPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		range->setBorderPen(pen);
	}
}

void ReferenceRangeDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value / 100.;
	for (auto* range : m_rangeList)
		range->setBorderOpacity(opacity);
}

//*************************************************************
//******* SLOTs for changes triggered in ReferenceRange ********
//*************************************************************
void ReferenceRangeDock::rangePositionLogicalStartChanged(const QPointF& positionLogical) {
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.lePositionStart->setText(numberLocale.toString(positionLogical.y()));
		ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.y()));
	} else {
		ui.lePositionStart->setText(numberLocale.toString(positionLogical.x()));
		ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.x()));
	}
}

void ReferenceRangeDock::rangePositionLogicalEndChanged(const QPointF& positionLogical) {
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	if (m_range->orientation() == ReferenceRange::Orientation::Horizontal) {
		ui.lePositionEnd->setText(numberLocale.toString(positionLogical.y()));
		ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.y()));
	} else {
		ui.lePositionEnd->setText(numberLocale.toString(positionLogical.x()));
		ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(positionLogical.x()));
	}
}

void ReferenceRangeDock::rangeOrientationChanged(ReferenceRange::Orientation orientation) {
	m_initializing = true;
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	m_initializing = false;
}

void ReferenceRangeDock::rangeVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

// background
void ReferenceRangeDock::rangeBackgroundTypeChanged(WorksheetElement::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(filename);
	m_initializing = false;
}

void ReferenceRangeDock::rangeBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue(round(opacity * 100.0));
	m_initializing = false;
}

void ReferenceRangeDock::rangeBorderPenChanged(QPen& pen) {
	m_initializing = true;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void ReferenceRangeDock::rangeBorderOpacityChanged(double value) {
	m_initializing = true;
	float v = (float)value * 100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ReferenceRangeDock::load() {
	if (!m_range)
		return;

	const Lock lock(m_initializing);

	SET_NUMBER_LOCALE
	auto orientation = m_range->orientation();
	ui.cbOrientation->setCurrentIndex(static_cast<int>(orientation));
	orientationChanged(ui.cbOrientation->currentIndex()); // call this to update the position widgets that depend on the orientation

	// position
	const auto* plot = static_cast<const CartesianPlot*>(m_range->plot());
	if (orientation == ReferenceRange::Orientation::Horizontal) {
		if (plot->yRangeFormat() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().y()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().y()));
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->yRangeDateTimeFormat());
			ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalStart().y()));
			ui.dtePositionEnd->setDisplayFormat(plot->yRangeDateTimeFormat());
			ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalEnd().y()));
		}
	} else {
		if (plot->xRangeFormat() == RangeT::Format::Numeric) {
			ui.lePositionStart->setText(numberLocale.toString(m_range->positionLogicalStart().x()));
			ui.lePositionEnd->setText(numberLocale.toString(m_range->positionLogicalEnd().x()));
		} else { // DateTime
			ui.dtePositionStart->setDisplayFormat(plot->xRangeDateTimeFormat());
			ui.dtePositionStart->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalStart().x()));
			ui.dtePositionEnd->setDisplayFormat(plot->xRangeDateTimeFormat());
			ui.dtePositionEnd->setDateTime(QDateTime::fromMSecsSinceEpoch(m_range->positionLogicalEnd().x()));
		}
	}

	// background
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());
	ui.cbBackgroundColorStyle->setCurrentIndex((int)m_range->backgroundColorStyle());
	ui.cbBackgroundImageStyle->setCurrentIndex((int)m_range->backgroundImageStyle());
	ui.cbBackgroundBrushStyle->setCurrentIndex((int)m_range->backgroundBrushStyle());
	ui.leBackgroundFileName->setText(m_range->backgroundFileName());
	ui.kcbBackgroundFirstColor->setColor(m_range->backgroundFirstColor());
	ui.kcbBackgroundSecondColor->setColor(m_range->backgroundSecondColor());
	ui.sbBackgroundOpacity->setValue(round(m_range->backgroundOpacity() * 100.0));

	// highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_range->backgroundFileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

	// Border
	ui.kcbBorderColor->setColor(m_range->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex((int)m_range->borderPen().style());
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(m_range->borderPen().widthF(), Worksheet::Unit::Point));
	ui.sbBorderOpacity->setValue(round(m_range->borderOpacity() * 100));
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
}
