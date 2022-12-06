/*
	File                 : CartesianPlotLegendDock.cpp
	Project              : LabPlot
	Description          : widget for cartesian plot legend properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotLegendDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KLocalizedString>

/*!
  \class CartesianPlotLegendDock
  \brief  Provides a widget for editing the properties of the cartesian plot legend currently selected in the project explorer.

  \ingroup kdefrontend
*/
CartesianPlotLegendDock::CartesianPlotLegendDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	//"Title"-tab
	auto hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	labelWidget->setGeometryAvailable(false);
	labelWidget->setBorderAvailable(false);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	//"Background"-tab
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabBackground->layout());
	backgroundWidget = new BackgroundWidget(ui.tabBackground);
	gridLayout->addWidget(backgroundWidget, 1, 0, 1, 3);

	borderLineWidget = new LineWidget(ui.tabBackground);
	gridLayout->addWidget(borderLineWidget, 4, 0, 1, 3);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	CartesianPlotLegendDock::updateLocale();

	// SIGNAL/SLOT

	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &CartesianPlotLegendDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &CartesianPlotLegendDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &CartesianPlotLegendDock::visibilityChanged);
	connect(ui.kfrLabelFont, &KFontRequester::fontSelected, this, &CartesianPlotLegendDock::labelFontChanged);
	connect(ui.kcbLabelColor, &KColorButton::changed, this, &CartesianPlotLegendDock::labelColorChanged);
	connect(ui.cbOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::labelOrderChanged);
	connect(ui.sbLineSymbolWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::lineSymbolWidthChanged);

	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &CartesianPlotLegendDock::bindingChanged);
	connect(ui.cbPositionX, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::positionYChanged);
	connect(ui.sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::customPositionYChanged);

	connect(ui.cbHorizontalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CartesianPlotLegendDock::verticalAlignmentChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotLegendDock::rotationChanged);

	// Border
	connect(ui.sbBorderCornerRadius, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::borderCornerRadiusChanged);

	// Layout
	connect(ui.sbLayoutTopMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutTopMarginChanged);
	connect(ui.sbLayoutBottomMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutBottomMarginChanged);
	connect(ui.sbLayoutLeftMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutLeftMarginChanged);
	connect(ui.sbLayoutRightMargin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutRightMarginChanged);
	connect(ui.sbLayoutHorizontalSpacing, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutHorizontalSpacingChanged);
	connect(ui.sbLayoutVerticalSpacing, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutVerticalSpacingChanged);
	connect(ui.sbLayoutColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotLegendDock::layoutColumnCountChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CartesianPlotLegend);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CartesianPlotLegendDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CartesianPlotLegendDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CartesianPlotLegendDock::info);

	ui.verticalLayout->addWidget(frame);

	init();
}

void CartesianPlotLegendDock::init() {
	this->retranslateUi();
}

void CartesianPlotLegendDock::setLegends(QList<CartesianPlotLegend*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_legendList = list;
	m_legend = list.first();
	setAspects(list);

	// if there is more then one legend in the list, disable the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_legend->name());
		ui.teComment->setText(m_legend->comment());
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

	// show the properties of the first curve
	this->load();

	// on the very first start the column count shown in UI is 1.
	// if the this count for m_legend is also 1 then the slot layoutColumnCountChanged is not called
	// and we need to disable the "order" widgets here.
	ui.lOrder->setVisible(m_legend->layoutColumnCount() != 1);
	ui.cbOrder->setVisible(m_legend->layoutColumnCount() != 1);

	// SIGNALs/SLOTs
	// General
	connect(m_legend, &AbstractAspect::aspectDescriptionChanged, this, &CartesianPlotLegendDock::aspectDescriptionChanged);
	connect(m_legend, &CartesianPlotLegend::labelFontChanged, this, &CartesianPlotLegendDock::legendLabelFontChanged);
	connect(m_legend, &CartesianPlotLegend::labelColorChanged, this, &CartesianPlotLegendDock::legendLabelColorChanged);
	connect(m_legend, &CartesianPlotLegend::labelColumnMajorChanged, this, &CartesianPlotLegendDock::legendLabelOrderChanged);
	connect(m_legend, &CartesianPlotLegend::positionChanged, this, &CartesianPlotLegendDock::legendPositionChanged);
	connect(m_legend, &CartesianPlotLegend::positionLogicalChanged, this, &CartesianPlotLegendDock::legendPositionLogicalChanged);
	connect(m_legend, &CartesianPlotLegend::horizontalAlignmentChanged, this, &CartesianPlotLegendDock::legendHorizontalAlignmentChanged);
	connect(m_legend, &CartesianPlotLegend::verticalAlignmentChanged, this, &CartesianPlotLegendDock::legendVerticalAlignmentChanged);
	connect(m_legend, &CartesianPlotLegend::rotationAngleChanged, this, &CartesianPlotLegendDock::legendRotationAngleChanged);
	connect(m_legend, &CartesianPlotLegend::lineSymbolWidthChanged, this, &CartesianPlotLegendDock::legendLineSymbolWidthChanged);
	connect(m_legend, &CartesianPlotLegend::visibleChanged, this, &CartesianPlotLegendDock::legendVisibilityChanged);

	// layout
	connect(m_legend, &CartesianPlotLegend::layoutTopMarginChanged, this, &CartesianPlotLegendDock::legendLayoutTopMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutBottomMarginChanged, this, &CartesianPlotLegendDock::legendLayoutBottomMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutLeftMarginChanged, this, &CartesianPlotLegendDock::legendLayoutLeftMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutRightMarginChanged, this, &CartesianPlotLegendDock::legendLayoutRightMarginChanged);
	connect(m_legend, &CartesianPlotLegend::layoutVerticalSpacingChanged, this, &CartesianPlotLegendDock::legendLayoutVerticalSpacingChanged);
	connect(m_legend, &CartesianPlotLegend::layoutHorizontalSpacingChanged, this, &CartesianPlotLegendDock::legendLayoutHorizontalSpacingChanged);
	connect(m_legend, &CartesianPlotLegend::layoutColumnCountChanged, this, &CartesianPlotLegendDock::legendLayoutColumnCountChanged);
}

void CartesianPlotLegendDock::activateTitleTab() const {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CartesianPlotLegendDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbLineSymbolWidth->setLocale(numberLocale);
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbPositionXLogical->setLocale(numberLocale);
	ui.sbPositionYLogical->setLocale(numberLocale);
	ui.sbBorderCornerRadius->setLocale(numberLocale);
	ui.sbLayoutTopMargin->setLocale(numberLocale);
	ui.sbLayoutBottomMargin->setLocale(numberLocale);
	ui.sbLayoutLeftMargin->setLocale(numberLocale);
	ui.sbLayoutRightMargin->setLocale(numberLocale);
	borderLineWidget->updateLocale();
}

void CartesianPlotLegendDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	if (m_units == Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbLineSymbolWidth->setValue(ui.sbLineSymbolWidth->value() * 2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value() * 2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value() * 2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value() * 2.54);
		ui.sbLayoutTopMargin->setValue(ui.sbLayoutTopMargin->value() * 2.54);
		ui.sbLayoutBottomMargin->setValue(ui.sbLayoutBottomMargin->value() * 2.54);
		ui.sbLayoutLeftMargin->setValue(ui.sbLayoutLeftMargin->value() * 2.54);
		ui.sbLayoutRightMargin->setValue(ui.sbLayoutRightMargin->value() * 2.54);
		ui.sbLayoutHorizontalSpacing->setValue(ui.sbLayoutHorizontalSpacing->value() * 2.54);
		ui.sbLayoutVerticalSpacing->setValue(ui.sbLayoutVerticalSpacing->value() * 2.54);
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbLineSymbolWidth->setValue(ui.sbLineSymbolWidth->value() / 2.54);
		ui.sbPositionX->setValue(ui.sbPositionX->value() / 2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value() / 2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value() / 2.54);
		ui.sbLayoutTopMargin->setValue(ui.sbLayoutTopMargin->value() / 2.54);
		ui.sbLayoutBottomMargin->setValue(ui.sbLayoutBottomMargin->value() / 2.54);
		ui.sbLayoutLeftMargin->setValue(ui.sbLayoutLeftMargin->value() / 2.54);
		ui.sbLayoutRightMargin->setValue(ui.sbLayoutRightMargin->value() / 2.54);
		ui.sbLayoutHorizontalSpacing->setValue(ui.sbLayoutHorizontalSpacing->value() / 2.54);
		ui.sbLayoutVerticalSpacing->setValue(ui.sbLayoutVerticalSpacing->value() / 2.54);
	}

	ui.sbLineSymbolWidth->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);

	labelWidget->updateUnits();
}

//************************************************************
//** SLOTs for changes triggered in CartesianPlotLegendDock **
//************************************************************
void CartesianPlotLegendDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbOrder->addItem(i18n("Column Major"));
	ui.cbOrder->addItem(i18n("Row Major"));

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
	if (m_units == Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbLineSymbolWidth->setSuffix(suffix);
	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);
}

// "General"-tab
void CartesianPlotLegendDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setVisible(state);
}

// General
void CartesianPlotLegendDock::labelFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	QFont labelsFont = font;
	labelsFont.setPixelSize(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* legend : m_legendList)
		legend->setLabelFont(labelsFont);
}

void CartesianPlotLegendDock::labelColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setLabelColor(color);
}

void CartesianPlotLegendDock::labelOrderChanged(const int index) {
	CONDITIONAL_LOCK_RETURN;

	bool columnMajor = (index == 0);
	for (auto* legend : m_legendList)
		legend->setLabelColumnMajor(columnMajor);
}

void CartesianPlotLegendDock::lineSymbolWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLineSymbolWidth(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

/*!
	called when legend's current horizontal position relative to its parent (left, center, right ) is changed.
*/
void CartesianPlotLegendDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.horizontalPosition = CartesianPlotLegend::HorizontalPosition(index);
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void CartesianPlotLegendDock::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}

/*!
	called when legend's current horizontal position relative to its parent (top, center, bottom ) is changed.
*/
void CartesianPlotLegendDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.verticalPosition = CartesianPlotLegend::VerticalPosition(index);
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setRotationAngle(value);
}

/*!
 * \brief CartesianPlotLegendDock::bindingChanged
 * Bind CartesianPlotLegend to the cartesian plot coords or not
 * \param checked
 */
void CartesianPlotLegendDock::bindingChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	// widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_legend->parent(AspectType::CartesianPlot));
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

	for (auto* legend : m_legendList)
		legend->setCoordinateBindingEnabled(checked);
}

// "Border"-tab
void CartesianPlotLegendDock::borderCornerRadiusChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setBorderCornerRadius(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

// Layout
void CartesianPlotLegendDock::layoutTopMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutBottomMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutLeftMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutRightMarginChanged(double margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutHorizontalSpacingChanged(double spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutVerticalSpacingChanged(double spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* legend : m_legendList)
		legend->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
}

void CartesianPlotLegendDock::layoutColumnCountChanged(int count) {
	ui.lOrder->setVisible(count != 1);
	ui.cbOrder->setVisible(count != 1);

	CONDITIONAL_LOCK_RETURN;

	for (auto* legend : m_legendList)
		legend->setLayoutColumnCount(count);
}

//*************************************************************
//**** SLOTs for changes triggered in CartesianPlotLegend *****
//*************************************************************
// General
void CartesianPlotLegendDock::legendLabelFontChanged(QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	// we need to set the font size in points for KFontRequester
	QFont f(font);
	f.setPointSizeF(round(Worksheet::convertFromSceneUnits(f.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(f);
}

void CartesianPlotLegendDock::legendLabelColorChanged(QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbLabelColor->setColor(color);
}

void CartesianPlotLegendDock::legendLabelOrderChanged(bool b) {
	CONDITIONAL_LOCK_RETURN;
	if (b)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major
}

void CartesianPlotLegendDock::legendLineSymbolWidthChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLineSymbolWidth->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void CartesianPlotLegendDock::legendVerticalAlignmentChanged(TextLabel::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}

void CartesianPlotLegendDock::legendPositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	SET_NUMBER_LOCALE
	ui.sbPositionXLogical->setValue(pos.x());
	ui.dtePositionXLogical->setDateTime(QDateTime::fromMSecsSinceEpoch(pos.x()));
	ui.sbPositionYLogical->setValue(pos.y());
}

void CartesianPlotLegendDock::legendPositionChanged(const CartesianPlotLegend::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit));
	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
}

void CartesianPlotLegendDock::legendRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void CartesianPlotLegendDock::legendVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

// Border
void CartesianPlotLegendDock::legendBorderCornerRadiusChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

// Layout
void CartesianPlotLegendDock::legendLayoutTopMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutBottomMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutLeftMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutRightMarginChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutVerticalSpacingChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutHorizontalSpacingChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotLegendDock::legendLayoutColumnCountChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutColumnCount->setValue(value);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotLegendDock::load() {
	// General-tab

	// Format
	// we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF(qRound(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(font);

	ui.kcbLabelColor->setColor(m_legend->labelColor());
	bool columnMajor = m_legend->labelColumnMajor();
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major

	ui.sbLineSymbolWidth->setValue(Worksheet::convertFromSceneUnits(m_legend->lineSymbolWidth(), m_worksheetUnit));

	// Geometry

	// widgets for positioning using absolute plot distances
	ui.cbPositionX->setCurrentIndex((int)m_legend->position().horizontalPosition);
	//	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(m_legend->position().point.x(), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_legend->position().verticalPosition);
	//	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(m_legend->position().point.y(), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_legend->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_legend->verticalAlignment());

	// widgets for positioning using logical plot coordinates
	SET_NUMBER_LOCALE
	bool allowLogicalCoordinates = (m_legend->plot() != nullptr);
	ui.lBindLogicalPos->setVisible(allowLogicalCoordinates);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_legend->plot());
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lPositionXLogical->show();
			ui.sbPositionXLogical->show();
			ui.lPositionXLogicalDateTime->hide();
			ui.dtePositionXLogical->hide();

			ui.sbPositionXLogical->setValue(m_legend->positionLogical().x());
			ui.sbPositionYLogical->setValue(m_legend->positionLogical().y());
		} else { // DateTime
			ui.lPositionXLogical->hide();
			ui.sbPositionXLogical->hide();
			ui.lPositionXLogicalDateTime->show();
			ui.dtePositionXLogical->show();

			ui.dtePositionXLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionXLogical->setDateTime(QDateTime::fromMSecsSinceEpoch(m_legend->positionLogical().x()));
		}

		ui.chbBindLogicalPos->setChecked(m_legend->coordinateBindingEnabled());
		bindingChanged(m_legend->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.sbPositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.sbPositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
	}
	ui.sbRotation->setValue(m_legend->rotationAngle());

	ui.chkVisible->setChecked(m_legend->isVisible());

	// legend title, background and border line
	QList<Background*> backgrounds;
	QList<TextLabel*> labels;
	QList<Line*> borderLines;
	for (auto* legend : m_legendList) {
		labels << legend->title();
		backgrounds << legend->background();
		borderLines << legend->borderLine();
	}

	labelWidget->setLabels(labels);
	backgroundWidget->setBackgrounds(backgrounds);
	borderLineWidget->setLines(borderLines);

	// Border
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(m_legend->borderCornerRadius(), m_worksheetUnit));

	// Layout
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutTopMargin(), m_worksheetUnit));
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutBottomMargin(), m_worksheetUnit));
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutLeftMargin(), m_worksheetUnit));
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutRightMargin(), m_worksheetUnit));
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutHorizontalSpacing(), m_worksheetUnit));
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(m_legend->layoutVerticalSpacing(), m_worksheetUnit));

	ui.sbLayoutColumnCount->setValue(m_legend->layoutColumnCount());
}

void CartesianPlotLegendDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_legendList.size();
	if (size > 1)
		m_legend->beginMacro(i18n("%1 cartesian plot legends: template \"%2\" loaded", size, name));
	else
		m_legend->beginMacro(i18n("%1: template \"%2\" loaded", m_legend->name(), name));

	this->loadConfig(config);

	m_legend->endMacro();
}

void CartesianPlotLegendDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlotLegend");

	// General-tab

	// Format
	// we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF(qRound(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrLabelFont->setFont(group.readEntry("LabelFont", font));

	ui.kcbLabelColor->setColor(group.readEntry("LabelColor", m_legend->labelColor()));

	bool columnMajor = group.readEntry("LabelColumMajor", m_legend->labelColumnMajor());
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); // column major
	else
		ui.cbOrder->setCurrentIndex(1); // row major

	ui.sbLineSymbolWidth->setValue(group.readEntry("LineSymbolWidth", Worksheet::convertFromSceneUnits(m_legend->lineSymbolWidth(), m_worksheetUnit)));

	// Geometry
	ui.cbPositionX->setCurrentIndex(group.readEntry("PositionX", (int)m_legend->position().horizontalPosition));
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionXValue", m_legend->position().point.x()), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex(group.readEntry("PositionY", (int)m_legend->position().verticalPosition));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionYValue", m_legend->position().point.y()), m_worksheetUnit));
	ui.sbRotation->setValue(group.readEntry("Rotation", (int)m_legend->rotationAngle()));

	ui.chkVisible->setChecked(group.readEntry("Visible", m_legend->isVisible()));

	// Background-tab
	backgroundWidget->loadConfig(group);

	// Border
	borderLineWidget->loadConfig(group);
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BorderCornerRadius", m_legend->borderCornerRadius()), m_worksheetUnit));

	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry("LayoutTopMargin", Worksheet::convertFromSceneUnits(m_legend->layoutTopMargin(), m_worksheetUnit)));
	ui.sbLayoutBottomMargin->setValue(group.readEntry("LayoutBottomMargin", Worksheet::convertFromSceneUnits(m_legend->layoutBottomMargin(), m_worksheetUnit)));
	ui.sbLayoutLeftMargin->setValue(group.readEntry("LayoutLeftMargin", Worksheet::convertFromSceneUnits(m_legend->layoutLeftMargin(), m_worksheetUnit)));
	ui.sbLayoutRightMargin->setValue(group.readEntry("LayoutRightMargin", Worksheet::convertFromSceneUnits(m_legend->layoutRightMargin(), m_worksheetUnit)));
	ui.sbLayoutHorizontalSpacing->setValue(
		group.readEntry("LayoutHorizontalSpacing", Worksheet::convertFromSceneUnits(m_legend->layoutHorizontalSpacing(), m_worksheetUnit)));
	ui.sbLayoutVerticalSpacing->setValue(
		group.readEntry("LayoutVerticalSpacing", Worksheet::convertFromSceneUnits(m_legend->layoutVerticalSpacing(), m_worksheetUnit)));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_legend->layoutColumnCount()));

	// Title
	group = config.group("PlotLegend");
	labelWidget->loadConfig(group);
}

void CartesianPlotLegendDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlotLegend");

	// General-tab
	// Format
	QFont font = m_legend->labelFont();
	font.setPointSizeF(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	group.writeEntry("LabelFont", font);
	group.writeEntry("LabelColor", ui.kcbLabelColor->color());
	group.writeEntry("LabelColumMajorOrder", ui.cbOrder->currentIndex() == 0); // true for "column major", false for "row major"
	group.writeEntry("LineSymbolWidth", Worksheet::convertToSceneUnits(ui.sbLineSymbolWidth->value(), m_worksheetUnit));

	// Geometry
	group.writeEntry("PositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("PositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(), m_worksheetUnit));
	group.writeEntry("PositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("PositionYValue", Worksheet::convertToSceneUnits(ui.sbPositionY->value(), m_worksheetUnit));
	group.writeEntry("Rotation", ui.sbRotation->value());

	group.writeEntry("Visible", ui.chkVisible->isChecked());

	// Background
	backgroundWidget->saveConfig(group);

	// Border
	borderLineWidget->saveConfig(group);
	group.writeEntry("BorderCornerRadius", Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), m_worksheetUnit));

	// Layout
	group.writeEntry("LayoutTopMargin", Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutBottomMargin", Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutLeftMargin", Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutRightMargin", Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutVerticalSpacing", Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), m_worksheetUnit));
	group.writeEntry("LayoutHorizontalSpacing", Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), m_worksheetUnit));
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	// Title
	group = config.group("PlotLegend");
	labelWidget->saveConfig(group);

	config.sync();
}
