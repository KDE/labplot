/*
	File                 : ErrorBarWidget.cpp
	Project              : LabPlot
	Description          : error bar widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ErrorBarWidget.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Settings.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "kdefrontend/widgets/LineWidget.h"

#include <KConfigGroup>
#include <QPainter>

/*!
	\class ErrorBarWidget
	\brief Widget for editing the properties of a Line object, mostly used in an appropriate dock widget.

	\ingroup kdefrontend
 */
ErrorBarWidget::ErrorBarWidget(QWidget* parent, bool poissonAvailable)
	: QWidget(parent) {
	ui.setupUi(this);

	cbXPlusColumn = new TreeViewComboBox(this);
	cbXMinusColumn = new TreeViewComboBox(this);
	cbYPlusColumn = new TreeViewComboBox(this);
	cbYMinusColumn = new TreeViewComboBox(this);
	lineWidget = new LineWidget(this);

	auto* gridLayout = qobject_cast<QGridLayout*>(ui.gridLayout);
	gridLayout->addWidget(cbXPlusColumn, 2, 2, 1, 1);
	gridLayout->addWidget(cbXMinusColumn, 3, 2, 1, 1);
	gridLayout->addWidget(cbYPlusColumn, 7, 2, 1, 1);
	gridLayout->addWidget(cbYMinusColumn, 8, 2, 1, 1);
	gridLayout->addWidget(lineWidget, 13, 0, 1, 3);

	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry("GUMTerms", false)) {
		// x
		ui.lXError->setText(i18n("X Uncertainty"));
		ui.cbXErrorType->addItem(i18n("No Uncertainties"), static_cast<int>(ErrorBar::ErrorType::NoError));
		if (poissonAvailable)
			ui.cbXErrorType->addItem(i18n("Poisson variance, sqrt(N)"), static_cast<int>(ErrorBar::ErrorType::Poisson));
		ui.cbXErrorType->addItem(i18n("Custom Uncertainty Values, symmetric"), static_cast<int>(ErrorBar::ErrorType::Symmetric));
		ui.cbXErrorType->addItem(i18n("Custom Uncertainty Values, asymmetric"), static_cast<int>(ErrorBar::ErrorType::Asymmetric));

		// y
		ui.lYError->setText(i18n("Y Uncertainty"));
		ui.cbYErrorType->addItem(i18n("No Uncertainties"), static_cast<int>(ErrorBar::ErrorType::NoError));
		if (poissonAvailable)
			ui.cbYErrorType->addItem(i18n("Poisson variance, sqrt(N)"), static_cast<int>(ErrorBar::ErrorType::Poisson));
		ui.cbYErrorType->addItem(i18n("Custom Uncertainty Values, symmetric"), static_cast<int>(ErrorBar::ErrorType::Symmetric));
		ui.cbYErrorType->addItem(i18n("Custom Uncertainty Values, asymmetric"), static_cast<int>(ErrorBar::ErrorType::Asymmetric));
	} else {
		// x
		ui.cbXErrorType->addItem(i18n("No Errors"), static_cast<int>(ErrorBar::ErrorType::NoError));
		if (poissonAvailable)
			ui.cbXErrorType->addItem(i18n("Poisson variance, sqrt(N)"), static_cast<int>(ErrorBar::ErrorType::Poisson));
		ui.cbXErrorType->addItem(i18n("Custom Error Values, symmetric"), static_cast<int>(ErrorBar::ErrorType::Symmetric));
		ui.cbXErrorType->addItem(i18n("Custom Error Values, asymmetric"), static_cast<int>(ErrorBar::ErrorType::Asymmetric));

		// y
		ui.cbYErrorType->addItem(i18n("No Errors"), static_cast<int>(ErrorBar::ErrorType::NoError));
		if (poissonAvailable)
			ui.cbYErrorType->addItem(i18n("Poisson variance, sqrt(N)"), static_cast<int>(ErrorBar::ErrorType::Poisson));
		ui.cbYErrorType->addItem(i18n("Custom Error Values, symmetric"), static_cast<int>(ErrorBar::ErrorType::Symmetric));
		ui.cbYErrorType->addItem(i18n("Custom Error Values, asymmetric"), static_cast<int>(ErrorBar::ErrorType::Asymmetric));
	}

	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	pm.fill(Qt::transparent);

	QPainter pa;
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawLine(3, 10, 17, 10); // vert. line
	pa.drawLine(10, 3, 10, 17); // hor. line
	pa.end();
	ui.cbType->addItem(i18n("Bars"), static_cast<int>(ErrorBar::Type::Simple));
	ui.cbType->setItemIcon(0, pm);

	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3, 10, 17, 10); // vert. line
	pa.drawLine(10, 3, 10, 17); // hor. line
	pa.drawLine(7, 3, 13, 3); // upper cap
	pa.drawLine(7, 17, 13, 17); // bottom cap
	pa.drawLine(3, 7, 3, 13); // left cap
	pa.drawLine(17, 7, 17, 13); // right cap
	pa.end();
	ui.cbType->addItem(i18n("Bars with Ends"), static_cast<int>(ErrorBar::Type::WithEnds));
	ui.cbType->setItemIcon(1, pm);

	// x
	connect(ui.cbXErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ErrorBarWidget::xErrorTypeChanged);
	connect(cbXPlusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ErrorBarWidget::xPlusColumnChanged);
	connect(cbXMinusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ErrorBarWidget::xMinusColumnChanged);

	// y
	connect(ui.cbYErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ErrorBarWidget::yErrorTypeChanged);
	connect(cbYPlusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ErrorBarWidget::yPlusColumnChanged);
	connect(cbYMinusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ErrorBarWidget::yMinusColumnChanged);

	// styling
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ErrorBarWidget::typeChanged);
	connect(ui.sbCapSize, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ErrorBarWidget::capSizeChanged);
}

void ErrorBarWidget::setErrorBars(const QList<ErrorBar*>& errorBars) {
	CONDITIONAL_LOCK_RETURN;
	m_errorBars = errorBars;
	m_errorBar = m_errorBars.first();

	QList<Line*> lines;
	for (auto* errorBar : errorBars)
		lines << errorBar->line();
	lineWidget->setLines(lines);

	load();

	if (m_errorBar->dimension() == ErrorBar::Dimension::Y) {
		ui.lXError->hide();
		ui.lXErrorType->hide();
		ui.cbXErrorType->hide();
		ui.lXDataPlus->hide();
		cbXPlusColumn->hide();
		ui.lXDataMinus->hide();
		cbXMinusColumn->hide();
	}

	// TODO
	if (m_errorBars.size() == 1) {
		cbXPlusColumn->setColumn(m_errorBar->xPlusColumn(), m_errorBar->xPlusColumnPath());
		cbXMinusColumn->setColumn(m_errorBar->xMinusColumn(), m_errorBar->xMinusColumnPath());
		cbYPlusColumn->setColumn(m_errorBar->yPlusColumn(), m_errorBar->yPlusColumnPath());
		cbYMinusColumn->setColumn(m_errorBar->yMinusColumn(), m_errorBar->yMinusColumnPath());
	} else {
		cbXPlusColumn->setCurrentModelIndex(QModelIndex());
		cbXMinusColumn->setCurrentModelIndex(QModelIndex());
		cbYPlusColumn->setCurrentModelIndex(QModelIndex());
		cbYMinusColumn->setCurrentModelIndex(QModelIndex());
	}

	// x
	connect(m_errorBar, &ErrorBar::xErrorTypeChanged, this, &ErrorBarWidget::errorBarXErrorTypeChanged);
	connect(m_errorBar, &ErrorBar::xPlusColumnChanged, this, &ErrorBarWidget::errorBarXPlusColumnChanged);
	connect(m_errorBar, &ErrorBar::xMinusColumnChanged, this, &ErrorBarWidget::errorBarXMinusColumnChanged);

	// y
	connect(m_errorBar, &ErrorBar::yErrorTypeChanged, this, &ErrorBarWidget::errorBarYErrorTypeChanged);
	connect(m_errorBar, &ErrorBar::yPlusColumnChanged, this, &ErrorBarWidget::errorBarYPlusColumnChanged);
	connect(m_errorBar, &ErrorBar::yMinusColumnChanged, this, &ErrorBarWidget::errorBarYMinusColumnChanged);

	// styling
	connect(m_errorBar, &ErrorBar::typeChanged, this, &ErrorBarWidget::errorBarTypeChanged);
	connect(m_errorBar, &ErrorBar::capSizeChanged, this, &ErrorBarWidget::errorBarCapSizeChanged);
}

void ErrorBarWidget::setModel(AspectTreeModel* model) {
	QList<AspectType> list{AspectType::Folder,
						   AspectType::Workbook,
						   AspectType::Datapicker,
						   AspectType::DatapickerCurve,
						   AspectType::Spreadsheet,
						   AspectType::LiveDataSource,
						   AspectType::Column,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYFitCurve,
						   AspectType::XYSmoothCurve,
						   AspectType::CantorWorksheet};

	cbXPlusColumn->setModel(model);
	cbXMinusColumn->setModel(model);
	cbXPlusColumn->setTopLevelClasses(list);
	cbXMinusColumn->setTopLevelClasses(list);

	cbYPlusColumn->setModel(model);
	cbYMinusColumn->setModel(model);
	cbYPlusColumn->setTopLevelClasses(list);
	cbYMinusColumn->setTopLevelClasses(list);
}

void ErrorBarWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	adjustLayout();
}

/*!
 * this functions adjusts the width of the first column in the layout of ErrorBarWidget
 * to the width of the first column in the layout of the parent widget
 * which ErrorBarWidget is being embedded into.
 */
void ErrorBarWidget::adjustLayout() {
	auto* parentGridLayout = dynamic_cast<QGridLayout*>(parentWidget()->layout());
	if (!parentGridLayout)
		return;

	auto* parentWidget = parentGridLayout->itemAtPosition(0, 0)->widget();
	if (!parentWidget)
		return;

	if (parentWidget->width() >= ui.lYError->width()) // use lYError since it's always visible
		ui.lYError->setMinimumWidth(parentWidget->width());
	else
		parentWidget->setMinimumWidth(ui.lYError->width());
}

void ErrorBarWidget::updateLocale() {
	ui.sbCapSize->setLocale(QLocale());
	lineWidget->updateLocale();
}

/*!
 * updates the visibility of styling related widgets upon changes in the error type (no error, etc.) settings.
 */
void ErrorBarWidget::updateStylingWidgets() {
	const auto xErrorType = static_cast<ErrorBar::ErrorType>(ui.cbXErrorType->currentData().toInt());
	const auto yErrorType = static_cast<ErrorBar::ErrorType>(ui.cbYErrorType->currentData().toInt());

	const bool visible = (xErrorType != ErrorBar::ErrorType::NoError || yErrorType != ErrorBar::ErrorType::NoError);
	ui.lFormat->setVisible(visible);
	ui.lType->setVisible(visible);
	ui.cbType->setVisible(visible);
	lineWidget->setVisible(visible);

	if (visible) {
		const auto type = static_cast<ErrorBar::Type>(ui.cbType->currentData().toInt());
		const bool b = (type == ErrorBar::Type::WithEnds);
		ui.lCapSize->setVisible(b);
		ui.sbCapSize->setVisible(b);
	} else {
		ui.lCapSize->hide();
		ui.sbCapSize->hide();
	}
}

//*************************************************************
//******** SLOTs for changes triggered in ErrorBarWidget **********
//*************************************************************
// x
void ErrorBarWidget::xErrorTypeChanged(int) {
	const auto errorType = static_cast<ErrorBar::ErrorType>(ui.cbXErrorType->currentData().toInt());
	switch (errorType) {
	case ErrorBar::ErrorType::NoError:
	case ErrorBar::ErrorType::Poisson:
		ui.lXDataPlus->setVisible(false);
		cbXPlusColumn->setVisible(false);
		ui.lXDataMinus->setVisible(false);
		cbXMinusColumn->setVisible(false);
		break;
	case ErrorBar::ErrorType::Symmetric:
		ui.lXDataPlus->setVisible(true);
		cbXPlusColumn->setVisible(true);
		ui.lXDataMinus->setVisible(false);
		cbXMinusColumn->setVisible(false);
		ui.lXDataPlus->setText(i18n("Data, +-:"));
		break;
	case ErrorBar::ErrorType::Asymmetric:
		ui.lXDataPlus->setVisible(true);
		cbXPlusColumn->setVisible(true);
		ui.lXDataMinus->setVisible(true);
		cbXMinusColumn->setVisible(true);
		ui.lXDataPlus->setText(i18n("Data, +:"));
	}

	updateStylingWidgets();

	CONDITIONAL_LOCK_RETURN;

	for (auto* errorBar : m_errorBars)
		errorBar->setXErrorType(errorType);
}

void ErrorBarWidget::xPlusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* errorBar : m_errorBars)
		errorBar->setXPlusColumn(column);
}

void ErrorBarWidget::xMinusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* errorBar : m_errorBars)
		errorBar->setXMinusColumn(column);
}

// y
void ErrorBarWidget::yErrorTypeChanged(int) {
	const auto errorType = static_cast<ErrorBar::ErrorType>(ui.cbYErrorType->currentData().toInt());
	switch (errorType) {
	case ErrorBar::ErrorType::NoError:
	case ErrorBar::ErrorType::Poisson:
		ui.lYDataPlus->setVisible(false);
		cbYPlusColumn->setVisible(false);
		ui.lYDataMinus->setVisible(false);
		cbYMinusColumn->setVisible(false);
		break;
	case ErrorBar::ErrorType::Symmetric:
		ui.lYDataPlus->setVisible(true);
		cbYPlusColumn->setVisible(true);
		ui.lYDataMinus->setVisible(false);
		cbYMinusColumn->setVisible(false);
		ui.lYDataPlus->setText(i18n("Data, +-:"));
		break;
	case ErrorBar::ErrorType::Asymmetric:
		ui.lYDataPlus->setVisible(true);
		cbYPlusColumn->setVisible(true);
		ui.lYDataMinus->setVisible(true);
		cbYMinusColumn->setVisible(true);
		ui.lYDataPlus->setText(i18n("Data, +:"));
	}

	updateStylingWidgets();

	CONDITIONAL_LOCK_RETURN;

	for (auto* errorBar : m_errorBars)
		errorBar->setYErrorType(errorType);
}

void ErrorBarWidget::yPlusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* errorBar : m_errorBars)
		errorBar->setYPlusColumn(column);
}

void ErrorBarWidget::yMinusColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* errorBar : m_errorBars)
		errorBar->setYMinusColumn(column);
}

// styling
void ErrorBarWidget::typeChanged(int) {
	const auto type = static_cast<ErrorBar::Type>(ui.cbType->currentData().toInt());
	const bool b = (type == ErrorBar::Type::WithEnds);
	ui.lCapSize->setVisible(b);
	ui.sbCapSize->setVisible(b);

	CONDITIONAL_RETURN_NO_LOCK;
	for (auto* errorBar : m_errorBars)
		errorBar->setType(type);
}

void ErrorBarWidget::capSizeChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* errorBar : m_errorBars)
		errorBar->setCapSize(size);
}

//*************************************************************
//*********** SLOTs for changes triggered in Line *************
//*************************************************************
// x
void ErrorBarWidget::errorBarXErrorTypeChanged(ErrorBar::ErrorType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXErrorType->setCurrentIndex(static_cast<int>(type));
}

void ErrorBarWidget::errorBarXPlusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXPlusColumn->setColumn(column, m_errorBar->xPlusColumnPath());
}

void ErrorBarWidget::errorBarXMinusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXMinusColumn->setColumn(column, m_errorBar->xMinusColumnPath());
}

void ErrorBarWidget::errorBarYErrorTypeChanged(ErrorBar::ErrorType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbYErrorType->setCurrentIndex(static_cast<int>(type));
}

void ErrorBarWidget::errorBarYPlusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYPlusColumn->setColumn(column, m_errorBar->yPlusColumnPath());
}

void ErrorBarWidget::errorBarYMinusColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYMinusColumn->setColumn(column, m_errorBar->yMinusColumnPath());
}

// styling
void ErrorBarWidget::errorBarTypeChanged(ErrorBar::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex(static_cast<int>(type));
}

void ErrorBarWidget::errorBarCapSizeChanged(double size) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ErrorBarWidget::load() {
	if (m_errorBar->dimension() == ErrorBar::Dimension::XY) {
		ui.cbXErrorType->setCurrentIndex(static_cast<int>(m_errorBar->xErrorType()));
		xErrorTypeChanged(ui.cbXErrorType->currentIndex());
	}

	ui.cbYErrorType->setCurrentIndex(static_cast<int>(m_errorBar->yErrorType()));
	yErrorTypeChanged(ui.cbYErrorType->currentIndex());

	ui.cbType->setCurrentIndex(static_cast<int>(m_errorBar->type()));
	typeChanged(ui.cbType->currentIndex());

	const double size = Worksheet::convertFromSceneUnits(m_errorBar->capSize(), Worksheet::Unit::Point);
	ui.sbCapSize->setValue(size);
}

void ErrorBarWidget::loadConfig(const KConfigGroup& group) {
	switch (m_errorBar->dimension()) {
	case ErrorBar::Dimension::XY:
		ui.cbXErrorType->setCurrentIndex(group.readEntry(QStringLiteral("XErrorType"), static_cast<int>(m_errorBar->xErrorType())));
		xErrorTypeChanged(ui.cbXErrorType->currentIndex());
		ui.cbYErrorType->setCurrentIndex(group.readEntry(QStringLiteral("YErrorType"), static_cast<int>(m_errorBar->yErrorType())));
		yErrorTypeChanged(ui.cbYErrorType->currentIndex());
		break;
	case ErrorBar::Dimension::Y:
		ui.cbYErrorType->setCurrentIndex(group.readEntry(QStringLiteral("ErrorType"), static_cast<int>(m_errorBar->yErrorType())));
		yErrorTypeChanged(ui.cbYErrorType->currentIndex());
	}

	ui.cbType->setCurrentIndex(group.readEntry(QStringLiteral("ErrorBarsType"), static_cast<int>(m_errorBar->type())));
	typeChanged(ui.cbType->currentIndex());

	const double size = group.readEntry(QStringLiteral("ErrorBarsCapSize"), m_errorBar->capSize());
	ui.sbCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

void ErrorBarWidget::saveConfig(KConfigGroup& group) const {
	switch (m_errorBar->dimension()) {
	case ErrorBar::Dimension::XY:
		group.writeEntry(QStringLiteral("XErrorType"), ui.cbXErrorType->currentIndex());
		group.writeEntry(QStringLiteral("YErrorType"), ui.cbYErrorType->currentIndex());
		break;
	case ErrorBar::Dimension::Y:
		group.writeEntry(QStringLiteral("ErrorType"), ui.cbYErrorType->currentIndex());
	}
	group.writeEntry(QStringLiteral("ErrorBarsType"), ui.cbType->currentIndex());
	group.writeEntry(QStringLiteral("ErrorBarsCapSize"), Worksheet::convertToSceneUnits(ui.sbCapSize->value(), Worksheet::Unit::Point));
}
