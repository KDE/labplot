/*
    File                 : CustomPointDock.cpp
    Project              : LabPlot
    Description          : widget for CustomPoint properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CustomPointDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <KLocalizedString>
#include <KConfig>

CustomPointDock::CustomPointDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//"Symbol"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	CustomPointDock::updateLocale();

	//SLOTS
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CustomPointDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &CustomPointDock::commentChanged);
	connect(ui.lePositionX, &QLineEdit::textChanged, this, &CustomPointDock::positionXChanged);
	connect(ui.dateTimeEditPositionX, &QDateTimeEdit::dateTimeChanged, this, &CustomPointDock::positionXDateTimeChanged);
	connect(ui.lePositionY, &QLineEdit::textChanged, this, &CustomPointDock::positionYChanged);
	connect(ui.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPointDock::plotRangeChanged);
	connect( ui.chkVisible, &QCheckBox::clicked, this, &CustomPointDock::visibilityChanged);

	//Template handler
	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CustomPoint);
	ui.verticalLayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CustomPointDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CustomPointDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CustomPointDock::info);
}

void CustomPointDock::setPoints(QList<CustomPoint*> points) {
	const Lock lock(m_initializing);
	m_points = points;
	m_point = m_points.first();
	m_aspect = m_points.first();
	Q_ASSERT(m_point);

	QList<Symbol*> symbols;
	for (auto* point : m_points)
		symbols << point->symbol();

	symbolWidget->setSymbols(symbols);

	//if there is more than one point in the list, disable the comment and name widgets in "general"
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
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first custom point
	this->load();

	updatePlotRanges();	// needed when loading project

	//for custom points being children of an InfoElement, the position is changed
	//via the parent settings -> disable the positioning here.
	bool enabled = (m_point->parentAspect()->type() != AspectType::InfoElement);
	ui.lePositionX->setEnabled(enabled);
	ui.dateTimeEditPositionX->setEnabled(enabled);
	ui.lePositionY->setEnabled(enabled);

	//SIGNALs/SLOTs
	// general
	connect(m_point, &CustomPoint::aspectDescriptionChanged, this, &CustomPointDock::aspectDescriptionChanged);
	connect(m_point, &CustomPoint::positionChanged, this, &CustomPointDock::pointPositionChanged);
	connect(m_point, &WorksheetElement::plotRangeListChanged, this, &CustomPointDock::updatePlotRanges);
	connect(m_point, &CustomPoint::visibleChanged, this, &CustomPointDock::pointVisibilityChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CustomPointDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.lePositionX->setLocale(numberLocale);
	ui.lePositionY->setLocale(numberLocale);
	symbolWidget->updateLocale();
}

void CustomPointDock::updatePlotRanges() const {
	updatePlotRangeList(ui.cbPlotRanges);
}

//**********************************************************
//**** SLOTs for changes triggered in CustomPointDock ******
//**********************************************************
//"General"-tab
void CustomPointDock::positionXChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double x = numberLocale.toDouble(ui.lePositionX->text(), &ok);
	if (ok) {
		QPointF pos{m_point->position().point};
		pos.setX(x);
		for (auto* point : m_points)
			point->setPosition(pos);
	}
}

void CustomPointDock::positionXDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	qint64 x = dateTime.toMSecsSinceEpoch();
	QPointF pos{m_point->position().point};
	pos.setX(x);
	for (auto* point : m_points)
		point->setPosition(pos);
}

void CustomPointDock::positionYChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double y = numberLocale.toDouble(ui.lePositionY->text(), &ok);
	if (ok) {
		QPointF pos{m_point->position().point};
		pos.setY(y);
		for (auto* point : m_points)
			point->setPosition(pos);
	}
}

void CustomPointDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	m_point->beginMacro(i18n("%1 CustomPoints: visibility changed", m_points.count()));
	for (auto* point : m_points)
		point->setVisible(state);
	m_point->endMacro();
}

//*********************************************************
//**** SLOTs for changes triggered in CustomPoint *********
//*********************************************************
//"General"-tab
void CustomPointDock::pointPositionChanged(const WorksheetElement::PositionWrapper &position) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.lePositionX->setText(numberLocale.toString(position.point.x()));
	ui.dateTimeEditPositionX->setDateTime(QDateTime::fromMSecsSinceEpoch(position.point.x()));
	ui.lePositionY->setText(numberLocale.toString(position.point.y()));
	m_initializing = false;
}

void CustomPointDock::pointVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void CustomPointDock::load() {
	if (!m_point)
		return;

	SET_NUMBER_LOCALE
	auto* plot = static_cast<CartesianPlot*>(m_point->parent(AspectType::CartesianPlot));
	if (plot->xRangeFormat() == RangeT::Format::Numeric) {
		ui.lPositionX->show();
		ui.lePositionX->show();
		ui.lPositionXDateTime->hide();
		ui.dateTimeEditPositionX->hide();

		ui.lePositionX->setText(numberLocale.toString(m_point->position().point.x()));
	} else {
		ui.lPositionX->hide();
		ui.lePositionX->hide();
		ui.lPositionXDateTime->show();
		ui.dateTimeEditPositionX->show();

		ui.dateTimeEditPositionX->setDisplayFormat(plot->xRangeDateTimeFormat());
		ui.dateTimeEditPositionX->setDateTime(QDateTime::fromMSecsSinceEpoch(m_point->position().point.x()));
	}

	ui.lePositionY->setText(numberLocale.toString(m_point->position().point.y()));
	ui.chkVisible->setChecked( m_point->isVisible() );
}

void CustomPointDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
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

	this->loadConfig(config);

	m_point->endMacro();
}

void CustomPointDock::loadConfig(KConfig& config) {
	symbolWidget->loadConfig(config.group("CustomPoint"));
}

void CustomPointDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CustomPoint");
	symbolWidget->saveConfig(group);
}
