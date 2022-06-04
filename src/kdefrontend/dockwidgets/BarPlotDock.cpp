/*
	File                 : BarPlotDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the bar plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlotDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QPushButton>

#include <KConfig>
#include <KLocalizedString>

BarPlotDock::BarPlotDock(QWidget* parent)
	: BaseDock(parent),
	cbXColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	// Tab "General"

	// x-data
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	cbXColumn->setSizePolicy(sizePolicy);
	static_cast<QVBoxLayout*>(ui.frameXColumn->layout())->insertWidget(0, cbXColumn);
	ui.bRemoveXColumn->setIcon(QIcon::fromTheme("edit-clear"));

	// y-data
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme("list-add"));

	m_gridLayout = new QGridLayout(ui.frameDataColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameDataColumns->setLayout(m_gridLayout);

	ui.cbType->addItem(i18n("Grouped"));
	ui.cbType->addItem(i18n("Stacked"));

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	// Tab "Bars"
	QString msg = i18n("Specify the factor in percent to control the width of the box relative to its default value.");
	ui.lWidthFactor->setToolTip(msg);
	ui.sbWidthFactor->setToolTip(msg);

	// filling
	ui.cbFillingType->addItem(i18n("Color"));
	ui.cbFillingType->addItem(i18n("Image"));
	ui.cbFillingType->addItem(i18n("Pattern"));

	ui.cbFillingColorStyle->addItem(i18n("Single Color"));
	ui.cbFillingColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbFillingImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbFillingImageStyle->addItem(i18n("Centered"));
	ui.cbFillingImageStyle->addItem(i18n("Tiled"));
	ui.cbFillingImageStyle->addItem(i18n("Center Tiled"));
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, Qt::SolidPattern);

	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bFillingOpen->setIcon(QIcon::fromTheme("document-open"));


	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// SLOTS
	// Tab "General"
	connect(ui.leName, &QLineEdit::textChanged, this, &BarPlotDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &BarPlotDock::commentChanged);
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &BarPlotDock::xColumnChanged);
	connect(ui.bRemoveXColumn, &QPushButton::clicked, this, &BarPlotDock::removeXColumn);
	connect(m_buttonNew, &QPushButton::clicked, this, &BarPlotDock::addDataColumn);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::typeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::orientationChanged);
	connect(ui.chkVisible, &QCheckBox::toggled, this, &BarPlotDock::visibilityChanged);

	// Tab "Bars"
	connect(ui.sbWidthFactor, QOverload<int>::of(&QSpinBox::valueChanged), this, &BarPlotDock::widthFactorChanged);

	// box filling
	connect(ui.chkFillingEnabled, &QCheckBox::toggled, this, &BarPlotDock::fillingEnabledChanged);
	connect(ui.cbFillingType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::fillingTypeChanged);
	connect(ui.cbFillingColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::fillingColorStyleChanged);
	connect(ui.cbFillingImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::fillingImageStyleChanged);
	connect(ui.cbFillingBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::fillingBrushStyleChanged);
	connect(ui.bFillingOpen, &QPushButton::clicked, this, &BarPlotDock::selectFile);
	connect(ui.leFillingFileName, &QLineEdit::returnPressed, this, &BarPlotDock::fileNameChanged);
	connect(ui.leFillingFileName, &QLineEdit::textChanged, this, &BarPlotDock::fileNameChanged);
	connect(ui.kcbFillingFirstColor, &KColorButton::changed, this, &BarPlotDock::fillingFirstColorChanged);
	connect(ui.kcbFillingSecondColor, &KColorButton::changed, this, &BarPlotDock::fillingSecondColorChanged);
	connect(ui.sbFillingOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BarPlotDock::fillingOpacityChanged);

	// box border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &BarPlotDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BarPlotDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BarPlotDock::borderOpacityChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &BarPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &BarPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &BarPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void BarPlotDock::setBarPlots(QList<BarPlot*> list) {
	const Lock lock(m_initializing);
	m_barPlots = list;
	m_barPlot = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_barPlot);
	m_aspectTreeModel = new AspectTreeModel(m_barPlot->project());
	setModel();

	// if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_barPlot->name());
		ui.teComment->setText(m_barPlot->comment());

		ui.lDataColumn->setEnabled(true);
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());

		ui.lDataColumn->setEnabled(false);
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	// show the properties of the first box plot
	ui.chkVisible->setChecked(m_barPlot->isVisible());
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);
	loadDataColumns();

	// set the current locale
	updateLocale();

	// SIGNALs/SLOTs
	// general
	connect(m_barPlot, &AbstractAspect::aspectDescriptionChanged, this, &BarPlotDock::aspectDescriptionChanged);
	connect(m_barPlot, &WorksheetElement::plotRangeListChanged, this, &BarPlotDock::updatePlotRanges);
	connect(m_barPlot, &BarPlot::visibleChanged, this, &BarPlotDock::plotVisibilityChanged);
	connect(m_barPlot, &BarPlot::typeChanged, this, &BarPlotDock::plotTypeChanged);
	connect(m_barPlot, &BarPlot::xColumnChanged, this, &BarPlotDock::plotXColumnChanged);
	connect(m_barPlot, &BarPlot::dataColumnsChanged, this, &BarPlotDock::plotDataColumnsChanged);

	connect(m_barPlot, &BarPlot::widthFactorChanged, this, &BarPlotDock::plotWidthFactorChanged);

	// box filling
	connect(m_barPlot, &BarPlot::fillingEnabledChanged, this, &BarPlotDock::plotFillingEnabledChanged);
	connect(m_barPlot, &BarPlot::fillingTypeChanged, this, &BarPlotDock::plotFillingTypeChanged);
	connect(m_barPlot, &BarPlot::fillingColorStyleChanged, this, &BarPlotDock::plotFillingColorStyleChanged);
	connect(m_barPlot, &BarPlot::fillingImageStyleChanged, this, &BarPlotDock::plotFillingImageStyleChanged);
	connect(m_barPlot, &BarPlot::fillingBrushStyleChanged, this, &BarPlotDock::plotFillingBrushStyleChanged);
	connect(m_barPlot, &BarPlot::fillingFirstColorChanged, this, &BarPlotDock::plotFillingFirstColorChanged);
	connect(m_barPlot, &BarPlot::fillingSecondColorChanged, this, &BarPlotDock::plotFillingSecondColorChanged);
	connect(m_barPlot, &BarPlot::fillingFileNameChanged, this, &BarPlotDock::plotFillingFileNameChanged);
	connect(m_barPlot, &BarPlot::fillingOpacityChanged, this, &BarPlotDock::plotFillingOpacityChanged);

	// box border
	connect(m_barPlot, &BarPlot::borderPenChanged, this, &BarPlotDock::plotBorderPenChanged);
	connect(m_barPlot, &BarPlot::borderOpacityChanged, this, &BarPlotDock::plotBorderOpacityChanged);
}

void BarPlotDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);

	list = {AspectType::Folder,
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

	cbXColumn->setTopLevelClasses(list);
	cbXColumn->setModel(m_aspectTreeModel);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void BarPlotDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbBorderWidth->setLocale(numberLocale);
}

void BarPlotDock::updatePlotRanges() {
	updatePlotRangeList(ui.cbPlotRanges);
}

void BarPlotDock::loadDataColumns() {
	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_barPlot->dataColumns().count();
	if (count != 0) {
		// box plot has already data columns, make sure we have the proper number of comboboxes
		int diff = count - m_dataComboBoxes.count();
		if (diff > 0) {
			for (int i = 0; i < diff; ++i)
				addDataColumn();
		} else if (diff < 0) {
			for (int i = diff; i != 0; ++i)
				removeDataColumn();
		}

		// show the columns in the comboboxes
		for (int i = 0; i < count; ++i)
			m_dataComboBoxes.at(i)->setAspect(m_barPlot->dataColumns().at(i));
	} else {
		// no data columns set in the box plot yet, we show the first combo box only
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 1; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	// disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_barPlots.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);
}

void BarPlotDock::setDataColumns() const {
	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_barPlot->setDataColumns(columns);
}

//**********************************************************
//*** SLOTs for changes triggered in BarPlotDock *****
//**********************************************************
void BarPlotDock::xColumnChanged(const QModelIndex& index) {
	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	ui.bRemoveXColumn->setEnabled(column != nullptr);

	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setXColumn(column);
}

void BarPlotDock::removeXColumn() {
	cbXColumn->setCurrentModelIndex(QModelIndex());
	ui.bRemoveXColumn->setEnabled(false);
	for (auto* barPlot : m_barPlots)
		barPlot->setXColumn(nullptr);
}

void BarPlotDock::addDataColumn() {
	auto* cb = new TreeViewComboBox;

	static const QList<AspectType> list{AspectType::Folder,
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
	cb->setTopLevelClasses(list);
	cb->setModel(m_aspectTreeModel);
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &BarPlotDock::dataColumnChanged);

	int index = m_dataComboBoxes.size();

	if (index == 0) {
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(cb->sizePolicy().hasHeightForWidth());
		cb->setSizePolicy(sizePolicy1);
	} else {
		auto* button = new QPushButton();
		button->setIcon(QIcon::fromTheme("list-remove"));
		connect(button, &QPushButton::clicked, this, &BarPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void BarPlotDock::removeDataColumn() {
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (sender) {
		// remove button was clicked, determin which one and
		// delete it together with the corresponding combobox
		for (int i = 0; i < m_removeButtons.count(); ++i) {
			if (sender == m_removeButtons.at(i)) {
				delete m_dataComboBoxes.takeAt(i + 1);
				delete m_removeButtons.takeAt(i);
			}
		}
	} else {
		// no sender is available, the function is being called directly in loadDataColumns().
		// delete the last remove button together with the corresponding combobox
		int index = m_removeButtons.count() - 1;
		if (index >= 0) {
			delete m_dataComboBoxes.takeAt(index + 1);
			delete m_removeButtons.takeAt(index);
		}
	}

	// TODO
	if (!m_removeButtons.isEmpty()) {
		ui.lDataColumn->setText(i18n("Columns:"));
	} else {
		ui.lDataColumn->setText(i18n("Column:"));
	}

	if (!m_initializing)
		setDataColumns();
}

void BarPlotDock::dataColumnChanged(const QModelIndex&) const {
	if (m_initializing)
		return;

	setDataColumns();
}

void BarPlotDock::typeChanged(int index) const {
	if (m_initializing)
		return;

	auto type = static_cast<BarPlot::Type>(index);
	for (auto* barPlot : m_barPlots)
		barPlot->setType(type);
}

void BarPlotDock::orientationChanged(int index) const {
	if (m_initializing)
		return;

	auto orientation = BarPlot::Orientation(index);
	for (auto* barPlot : m_barPlots)
		barPlot->setOrientation(orientation);
}

void BarPlotDock::visibilityChanged(bool state) const {
	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setVisible(state);
}

//"Box"-tab
void BarPlotDock::widthFactorChanged(int value) const {
	if (m_initializing)
		return;

	double factor = (double)value / 100.;
	for (auto* barPlot : m_barPlots)
		barPlot->setWidthFactor(factor);
}

// box filling
void BarPlotDock::fillingEnabledChanged(bool state) const {
	ui.cbFillingType->setEnabled(state);
	ui.cbFillingColorStyle->setEnabled(state);
	ui.cbFillingBrushStyle->setEnabled(state);
	ui.cbFillingImageStyle->setEnabled(state);
	ui.kcbFillingFirstColor->setEnabled(state);
	ui.kcbFillingSecondColor->setEnabled(state);
	ui.leFillingFileName->setEnabled(state);
	ui.bFillingOpen->setEnabled(state);
	ui.sbFillingOpacity->setEnabled(state);

	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setFillingEnabled(state);
}

void BarPlotDock::fillingTypeChanged(int index) const {
	if (index == -1)
		return;

	auto type = (WorksheetElement::BackgroundType)index;

	if (type == WorksheetElement::BackgroundType::Color) {
		ui.lFillingColorStyle->show();
		ui.cbFillingColorStyle->show();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();

		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();

		auto style = (WorksheetElement::BackgroundColorStyle)ui.cbFillingColorStyle->currentIndex();
		if (style == WorksheetElement::BackgroundColorStyle::SingleColor) {
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		} else {
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	} else if (type == WorksheetElement::BackgroundType::Image) {
		ui.lFillingFirstColor->hide();
		ui.kcbFillingFirstColor->hide();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();

		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->show();
		ui.cbFillingImageStyle->show();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();
		ui.lFillingFileName->show();
		ui.leFillingFileName->show();
		ui.bFillingOpen->show();
	} else if (type == WorksheetElement::BackgroundType::Pattern) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();

		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->show();
		ui.cbFillingBrushStyle->show();
		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();
	}

	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setFillingType(type);
}

void BarPlotDock::fillingColorStyleChanged(int index) const {
	if (index == -1)
		return;

	auto style = (WorksheetElement::BackgroundColorStyle)index;

	if (style == WorksheetElement::BackgroundColorStyle::SingleColor) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	} else {
		ui.lFillingFirstColor->setText(i18n("First color:"));
		ui.lFillingSecondColor->show();
		ui.kcbFillingSecondColor->show();
	}

	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setFillingColorStyle(style);
}

void BarPlotDock::fillingImageStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto style = (WorksheetElement::BackgroundImageStyle)index;
	for (auto* barPlot : m_barPlots)
		barPlot->setFillingImageStyle(style);
}

void BarPlotDock::fillingBrushStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* barPlot : m_barPlots)
		barPlot->setFillingBrushStyle(style);
}

void BarPlotDock::fillingFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setFillingFirstColor(c);

	Lock lock(m_initializing);
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, c);
}

void BarPlotDock::fillingSecondColorChanged(const QColor& c) const {
	if (m_initializing)
		return;

	for (auto* barPlot : m_barPlots)
		barPlot->setFillingSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void BarPlotDock::selectFile() {
	const QString& path = GuiTools::openImageFile(QLatin1String("BarPlotDock"));
	if (path.isEmpty())
		return;

	ui.leFillingFileName->setText(path);
}

void BarPlotDock::fileNameChanged() const {
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* barPlot : m_barPlots)
		barPlot->setFillingFileName(fileName);
}

void BarPlotDock::fillingOpacityChanged(int value) const {
	if (m_initializing)
		return;

	float opacity = (float)value / 100;
	for (auto* barPlot : m_barPlots)
		barPlot->setFillingOpacity(opacity);
}

// box border
void BarPlotDock::borderStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* barPlot : m_barPlots) {
		pen = barPlot->borderPen();
		pen.setStyle(penStyle);
		barPlot->setBorderPen(pen);
	}
}

void BarPlotDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* barPlot : m_barPlots) {
		pen = barPlot->borderPen();
		pen.setColor(color);
		barPlot->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void BarPlotDock::borderWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* barPlot : m_barPlots) {
		pen = barPlot->borderPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		barPlot->setBorderPen(pen);
	}
}

void BarPlotDock::borderOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value / 100.;
	for (auto* barPlot : m_barPlots)
		barPlot->setBorderOpacity(opacity);
}

//*************************************************************
//******* SLOTs for changes triggered in BarPlot ********
//*************************************************************
// general
void BarPlotDock::plotXColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXColumn->setColumn(column, m_barPlot->xColumnPath());
	m_initializing = false;
}
void BarPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
	Lock lock(m_initializing);
	loadDataColumns();
}
void BarPlotDock::plotTypeChanged(BarPlot::Type type) {
	Lock lock(m_initializing);
	ui.cbType->setCurrentIndex((int)type);
}
void BarPlotDock::plotOrientationChanged(BarPlot::Orientation orientation) {
	Lock lock(m_initializing);
	ui.cbOrientation->setCurrentIndex((int)orientation);
}
void BarPlotDock::plotVisibilityChanged(bool on) {
	Lock lock(m_initializing);
	ui.chkVisible->setChecked(on);
}

// box
void BarPlotDock::plotWidthFactorChanged(double factor) {
	Lock lock(m_initializing);
	// 	float v = (float)value*100.;
	ui.sbWidthFactor->setValue(factor * 100);
}

// box filling
void BarPlotDock::plotFillingEnabledChanged(bool status) {
	Lock lock(m_initializing);
	ui.chkFillingEnabled->setChecked(status);
}
void BarPlotDock::plotFillingTypeChanged(WorksheetElement::BackgroundType type) {
	Lock lock(m_initializing);
	ui.cbFillingType->setCurrentIndex(static_cast<int>(type));
}
void BarPlotDock::plotFillingColorStyleChanged(WorksheetElement::BackgroundColorStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingColorStyle->setCurrentIndex(static_cast<int>(style));
}
void BarPlotDock::plotFillingImageStyleChanged(WorksheetElement::BackgroundImageStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingImageStyle->setCurrentIndex(static_cast<int>(style));
}
void BarPlotDock::plotFillingBrushStyleChanged(Qt::BrushStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingBrushStyle->setCurrentIndex(style);
}
void BarPlotDock::plotFillingFirstColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingFirstColor->setColor(color);
}
void BarPlotDock::plotFillingSecondColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingSecondColor->setColor(color);
}
void BarPlotDock::plotFillingFileNameChanged(QString& filename) {
	Lock lock(m_initializing);
	ui.leFillingFileName->setText(filename);
}
void BarPlotDock::plotFillingOpacityChanged(double opacity) {
	Lock lock(m_initializing);
	ui.sbFillingOpacity->setValue(round(opacity * 100.0));
}

// box border
void BarPlotDock::plotBorderPenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}
void BarPlotDock::plotBorderOpacityChanged(float value) {
	Lock lock(m_initializing);
	float v = (float)value * 100.;
	ui.sbBorderOpacity->setValue(v);
}


//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BarPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("BarPlot"));

	// general
	ui.cbType->setCurrentIndex(group.readEntry("Type", (int)m_barPlot->type()));
	ui.cbOrientation->setCurrentIndex(group.readEntry("Orientation", (int)m_barPlot->orientation()));

	// box
	ui.sbWidthFactor->setValue(round(group.readEntry("WidthFactor", m_barPlot->widthFactor()) * 100));

	// box filling
	ui.chkFillingEnabled->setChecked(group.readEntry("FillingEnabled", m_barPlot->fillingEnabled()));
	ui.cbFillingType->setCurrentIndex(group.readEntry("FillingType", (int)m_barPlot->fillingType()));
	ui.cbFillingColorStyle->setCurrentIndex(group.readEntry("FillingColorStyle", (int)m_barPlot->fillingColorStyle()));
	ui.cbFillingImageStyle->setCurrentIndex(group.readEntry("FillingImageStyle", (int)m_barPlot->fillingImageStyle()));
	ui.cbFillingBrushStyle->setCurrentIndex(group.readEntry("FillingBrushStyle", (int)m_barPlot->fillingBrushStyle()));
	ui.leFillingFileName->setText(group.readEntry("FillingFileName", m_barPlot->fillingFileName()));
	ui.kcbFillingFirstColor->setColor(group.readEntry("FillingFirstColor", m_barPlot->fillingFirstColor()));
	ui.kcbFillingSecondColor->setColor(group.readEntry("FillingSecondColor", m_barPlot->fillingSecondColor()));
	ui.sbFillingOpacity->setValue(round(group.readEntry("FillingOpacity", m_barPlot->fillingOpacity()) * 100));

	// update the box filling widgets
	fillingEnabledChanged(ui.chkFillingEnabled->isChecked());
	fillingTypeChanged(ui.cbFillingType->currentIndex());

	// box border
	const QPen& penBorder = m_barPlot->borderPen();
	ui.cbBorderStyle->setCurrentIndex(group.readEntry("BorderStyle", (int)penBorder.style()));
	ui.kcbBorderColor->setColor(group.readEntry("BorderColor", penBorder.color()));
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", penBorder.widthF()), Worksheet::Unit::Point));
	ui.sbBorderOpacity->setValue(group.readEntry("BorderOpacity", m_barPlot->borderOpacity()) * 100);

	Lock lock(m_initializing);
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
}

void BarPlotDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_barPlots.size();
	if (size > 1)
		m_barPlot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_barPlot->beginMacro(i18n("%1: template \"%2\" loaded", m_barPlot->name(), name));

	this->loadConfig(config);

	m_barPlot->endMacro();
}

void BarPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("BarPlot");

	// general
	group.writeEntry("Type", ui.cbType->currentIndex());
	group.writeEntry("Orientation", ui.cbOrientation->currentIndex());

	// box
	group.writeEntry("WidthFactor", ui.sbWidthFactor->value() / 100.0);

	// box filling
	group.writeEntry("FillingEnabled", ui.chkFillingEnabled->isChecked());
	group.writeEntry("FillingType", ui.cbFillingType->currentIndex());
	group.writeEntry("FillingColorStyle", ui.cbFillingColorStyle->currentIndex());
	group.writeEntry("FillingImageStyle", ui.cbFillingImageStyle->currentIndex());
	group.writeEntry("FillingBrushStyle", ui.cbFillingBrushStyle->currentIndex());
	group.writeEntry("FillingFileName", ui.leFillingFileName->text());
	group.writeEntry("FillingFirstColor", ui.kcbFillingFirstColor->color());
	group.writeEntry("FillingSecondColor", ui.kcbFillingSecondColor->color());
	group.writeEntry("FillingOpacity", ui.sbFillingOpacity->value() / 100.0);

	// box border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value() / 100.0);

	config.sync();
}
