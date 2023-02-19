/*
	File                 : CustomPointDock.cpp
	Project              : LabPlot
	Description          : widget for CustomPoint properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CustomPointDock.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <KConfig>
#include <KLocalizedString>

CustomPointDock::CustomPointDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//"Symbol"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	CustomPointDock::updateLocale();

	// Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));

	// SLOTS
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &CustomPointDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &CustomPointDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &CustomPointDock::visibilityChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPointDock::plotRangeChanged);

	// positioning
	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &CustomPointDock::bindingChanged);
	connect(ui.cbPositionX, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CustomPointDock::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &CustomPointDock::positionYChanged);
	connect(ui.sbPositionX, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &CustomPointDock::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &CustomPointDock::customPositionYChanged);
	connect(ui.sbPositionXLogical,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&CustomPointDock::positionXLogicalChanged);
	connect(ui.dtePositionXLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &CustomPointDock::positionXLogicalDateTimeChanged);
	connect(ui.sbPositionYLogical,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&CustomPointDock::positionYLogicalChanged);
	connect(ui.dtePositionYLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &CustomPointDock::positionYLogicalDateTimeChanged);

	// Template handler
	auto* frame = new QFrame(this);
	auto* hlayout = new QHBoxLayout(frame);
	hlayout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CustomPoint);
	hlayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CustomPointDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CustomPointDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CustomPointDock::info);

	ui.verticalLayout->addWidget(frame);
}

void CustomPointDock::setPoints(QList<CustomPoint*> points) {
	CONDITIONAL_LOCK_RETURN;
	m_points = points;
	m_point = m_points.first();
	setAspects(points);
	Q_ASSERT(m_point);

	QList<Symbol*> symbols;
	for (auto* point : m_points)
		symbols << point->symbol();

	symbolWidget->setSymbols(symbols);

	// if there is more than one point in the list, disable the comment and name widgets in "general"
	if (m_points.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_point->name());
		ui.teComment->setText(m_point->comment());
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

	// show the properties of the first custom point
	this->load();
	initConnections();
	updatePlotRanges(); // needed when loading project

	// for custom points being children of an InfoElement, the position is changed
	// via the parent settings -> disable the positioning here.
	bool enabled = (m_point->parentAspect()->type() != AspectType::InfoElement);
	ui.chbBindLogicalPos->setEnabled(enabled);
	ui.sbPositionXLogical->setEnabled(enabled);
	ui.lPositionXLogicalDateTime->setEnabled(enabled);
	ui.sbPositionYLogical->setEnabled(enabled);
	ui.lPositionYLogicalDateTime->setEnabled(enabled);
}

void CustomPointDock::initConnections() const {
	// SIGNALs/SLOTs
	//  general
	connect(m_point, &CustomPoint::aspectDescriptionChanged, this, &CustomPointDock::aspectDescriptionChanged);
	connect(m_point, &WorksheetElement::plotRangeListChanged, this, &CustomPointDock::updatePlotRanges);
	connect(m_point, &CustomPoint::visibleChanged, this, &CustomPointDock::pointVisibilityChanged);
	connect(m_point, &CustomPoint::positionChanged, this, &CustomPointDock::pointPositionChanged);
	connect(m_point, &CustomPoint::positionLogicalChanged, this, &CustomPointDock::pointPositionLogicalChanged);
	connect(m_point, &CustomPoint::coordinateBindingEnabledChanged, this, &CustomPointDock::pointCoordinateBindingEnabledChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CustomPointDock::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbPositionXLogical->setLocale(numberLocale);
	ui.sbPositionYLogical->setLocale(numberLocale);
	symbolWidget->updateLocale();
}

void CustomPointDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
}

//**********************************************************
//**** SLOTs for changes triggered in CustomPointDock ******
//**********************************************************
//"General"-tab
/*!
	called when label's current horizontal position relative to its parent (left, center, right ) is changed.
*/
void CustomPointDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto horPos = WorksheetElement::HorizontalPosition(index);
	for (auto* point : m_points) {
		auto position = point->position();
		position.horizontalPosition = horPos;
		point->setPosition(position);
	}
}

/*!
	called when label's current horizontal position relative to its parent (top, center, bottom) is changed.
*/
void CustomPointDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto verPos = WorksheetElement::VerticalPosition(index);
	for (auto* point : m_points) {
		auto position = point->position();
		position.verticalPosition = verPos;
		point->setPosition(position);
	}
}

void CustomPointDock::customPositionXChanged(Common::ExpressionValue value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double x = Worksheet::convertToSceneUnits(value.value<double>(), m_worksheetUnit);
	for (auto* point : m_points) {
		auto position = point->position();
		position.point.setX(x);
		point->setPosition(position);
	}
}

void CustomPointDock::customPositionYChanged(Common::ExpressionValue value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double y = Worksheet::convertToSceneUnits(value.value<double>(), m_worksheetUnit);
	for (auto* point : m_points) {
		auto position = point->position();
		position.point.setY(y);
		point->setPosition(position);
	}
}

// positioning using logical plot coordinates
void CustomPointDock::positionXLogicalChanged(Common::ExpressionValue value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPointF pos = m_point->positionLogical();
	pos.setX(value.value<double>());
	for (auto* point : m_points)
		point->setPositionLogical(pos);
}

void CustomPointDock::positionXLogicalDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	QPointF pos = m_point->positionLogical();
	pos.setX(value);
	for (auto* point : m_points)
		point->setPositionLogical(pos);
}

void CustomPointDock::positionYLogicalChanged(Common::ExpressionValue value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPointF pos = m_point->positionLogical();
	pos.setY(value.value<double>());
	for (auto* point : m_points)
		point->setPositionLogical(pos);
}

void CustomPointDock::positionYLogicalDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	QPointF pos = m_point->positionLogical();
	pos.setY(value);
	for (auto* point : m_points)
		point->setPositionLogical(pos);
}

void CustomPointDock::visibilityChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	m_point->beginMacro(i18n("%1 CustomPoints: visibility changed", m_points.count()));
	for (auto* point : m_points)
		point->setVisible(state);
	m_point->endMacro();
}

/*!
 * \brief CustomPointDock::bindingChanged
 * Bind CustomPoint to the cartesian plot coords or not
 * \param checked
 */
void CustomPointDock::bindingChanged(bool checked) {
	ui.chbBindLogicalPos->setChecked(checked);

	// widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_point->parent(AspectType::CartesianPlot));
	if (plot) {
		// x
		bool numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);
		if (numeric) {
			ui.lPositionXLogical->setVisible(checked);
			ui.sbPositionXLogical->setVisible(checked);
		} else {
			ui.lPositionXLogicalDateTime->setVisible(checked);
			ui.dtePositionXLogical->setVisible(checked);
		}

		// y
		numeric = (plot->yRangeFormatDefault() == RangeT::Format::Numeric);
		if (numeric) {
			ui.lPositionYLogical->setVisible(checked);
			ui.sbPositionYLogical->setVisible(checked);
		} else {
			ui.lPositionYLogicalDateTime->setVisible(checked);
			ui.dtePositionYLogical->setVisible(checked);
		}
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* point : m_points)
		point->setCoordinateBindingEnabled(checked);
}

//*********************************************************
//**** SLOTs for changes triggered in CustomPoint *********
//*********************************************************
//"General"-tab
void CustomPointDock::pointPositionChanged(const WorksheetElement::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit));
	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
}

void CustomPointDock::pointCoordinateBindingEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	bindingChanged(enabled);
}

void CustomPointDock::pointPositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionXLogical->setValue(pos.x());
	ui.dtePositionXLogical->setMSecsSinceEpochUTC(pos.x());
	ui.sbPositionYLogical->setValue(pos.y());
	ui.dtePositionYLogical->setMSecsSinceEpochUTC(pos.y());
}

void CustomPointDock::pointVisibilityChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(on);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void CustomPointDock::load() {
	if (!m_point)
		return;

	// Geometry
	// widgets for positioning using absolute plot distances
	ui.cbPositionX->setCurrentIndex((int)m_point->position().horizontalPosition);
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(m_point->position().point.x(), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_point->position().verticalPosition);
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(m_point->position().point.y(), m_worksheetUnit));

	// widgets for positioning using logical plot coordinates
	bool allowLogicalCoordinates = (m_point->plot() != nullptr);
	ui.lBindLogicalPos->setVisible(allowLogicalCoordinates);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_point->plot());

		// x
		bool numeric = (plot->xRangeFormatDefault() == RangeT::Format::Numeric);
		ui.lPositionXLogical->setVisible(numeric);
		ui.sbPositionXLogical->setVisible(numeric);
		ui.lPositionXLogicalDateTime->setVisible(!numeric);
		ui.dtePositionXLogical->setVisible(!numeric);
		if (numeric)
			ui.sbPositionXLogical->setValue(m_point->positionLogical().x());
		else {
			ui.dtePositionXLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionXLogical->setMSecsSinceEpochUTC(m_point->positionLogical().x());
		}

		// y
		numeric = (plot->yRangeFormatDefault() == RangeT::Format::Numeric);
		ui.lPositionYLogical->setVisible(numeric);
		ui.sbPositionYLogical->setVisible(numeric);
		ui.lPositionYLogicalDateTime->setVisible(!numeric);
		ui.dtePositionYLogical->setVisible(!numeric);
		if (numeric)
			ui.sbPositionYLogical->setValue(m_point->positionLogical().y());
		else {
			ui.dtePositionYLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::Y));
			ui.dtePositionYLogical->setMSecsSinceEpochUTC(m_point->positionLogical().y());
		}

		bindingChanged(m_point->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.sbPositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.sbPositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
		ui.lPositionYLogicalDateTime->hide();
		ui.dtePositionYLogical->hide();
	}
	ui.chkVisible->setChecked(m_point->isVisible());
}

void CustomPointDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_points.size();
	if (size > 1)
		m_point->beginMacro(i18n("%1 custom points: template \"%2\" loaded", size, name));
	else
		m_point->beginMacro(i18n("%1: template \"%2\" loaded", m_point->name(), name));

	symbolWidget->loadConfig(config.group("CustomPoint"));

	m_point->endMacro();
}

void CustomPointDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CustomPoint");
	symbolWidget->saveConfig(group);
}
