/***************************************************************************
    File                 : BoxPlotDock.cpp
    Project              : LabPlot
    Description          : Dock widget for the reference line on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#include "BoxPlotDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <QFileDialog>
#include <QImageReader>
#include <QPushButton>

#include <KLocalizedString>
#include <KConfig>

BoxPlotDock::BoxPlotDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	// Tab "General"
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme("list-add"));
	connect(m_buttonNew, &QPushButton::clicked, this, &BoxPlotDock::addDataColumn);

	m_gridLayout = new QGridLayout(ui.frameDataColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameDataColumns->setLayout(m_gridLayout);

	ui.cbWhiskersType->addItem(i18n("min/max"));
	ui.cbWhiskersType->addItem(i18n("1.5 IQR"));
	ui.cbWhiskersType->addItem(i18n("1 stddev"));
	ui.cbWhiskersType->addItem(i18n("1/99 percentiles"));
	ui.cbWhiskersType->addItem(i18n("2/98 percentiles"));

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	//Tab "Box"
	//filling
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
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	//box border
	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

	//median line
	GuiTools::updatePenStyles(ui.cbMedianLineStyle, Qt::black);

	//Tab "Markers"
	GuiTools::addSymbolStyles(ui.cbSymbolOutliersStyle);
	ui.cbSymbolOutliersStyle->insertItem(0, i18n("None"));
	GuiTools::addSymbolStyles(ui.cbSymbolMeanStyle);
	ui.cbSymbolMeanStyle->insertItem(0, i18n("None"));
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);

	//Tab "Whiskers"
	GuiTools::updatePenStyles(ui.cbWhiskersStyle, Qt::black);

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//Validators

	//set the current locale
	updateLocale();

	//SLOTS
	//Tab "General"
	connect(ui.leName, &QLineEdit::textChanged, this, &BoxPlotDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &BoxPlotDock::commentChanged);

	//Tab "Box"
	//box filling
	connect(ui.chkFillingEnabled, &QCheckBox::stateChanged, this, &BoxPlotDock::fillingEnabledChanged);
	connect(ui.cbFillingType, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::fillingTypeChanged);
	connect(ui.cbFillingColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::fillingColorStyleChanged);
	connect(ui.cbFillingImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::fillingImageStyleChanged);
	connect(ui.cbFillingBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::fillingBrushStyleChanged);
	connect(ui.bFillingOpen, &QPushButton::clicked, this, &BoxPlotDock::selectFile);
	connect(ui.leFillingFileName, &QLineEdit::returnPressed, this, &BoxPlotDock::fileNameChanged);
	connect(ui.leFillingFileName, &QLineEdit::textChanged, this, &BoxPlotDock::fileNameChanged);
	connect(ui.kcbFillingFirstColor, &KColorButton::changed, this, &BoxPlotDock::fillingFirstColorChanged);
	connect(ui.kcbFillingSecondColor, &KColorButton::changed, this, &BoxPlotDock::fillingSecondColorChanged);
	connect(ui.sbFillingOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &BoxPlotDock::fillingOpacityChanged);

	//box border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &BoxPlotDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::borderOpacityChanged);

	//median line
	connect(ui.cbMedianLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::medianLineStyleChanged);
	connect(ui.kcbMedianLineColor, &KColorButton::changed, this, &BoxPlotDock::medianLineColorChanged);
	connect(ui.sbMedianLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::medianLineWidthChanged);
	connect(ui.sbMedianLineOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::medianLineOpacityChanged);

	//Tab "Markers"
	connect(ui.cbSymbolOutliersStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::symbolOutliersStyleChanged);
	connect(ui.cbSymbolMeanStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::symbolMeanStyleChanged);
	connect(ui.sbSymbolSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::symbolsSizeChanged);
	connect(ui.sbSymbolRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::symbolsRotationChanged);
	connect(ui.sbSymbolOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::symbolsOpacityChanged);
	connect(ui.cbSymbolFillingStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::symbolsFillingStyleChanged);
	connect(ui.kcbSymbolFillingColor, &KColorButton::changed, this, &BoxPlotDock::symbolsFillingColorChanged);
	connect(ui.cbSymbolBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::symbolsBorderStyleChanged);
	connect(ui.kcbSymbolBorderColor, &KColorButton::changed, this, &BoxPlotDock::symbolsBorderColorChanged);
	connect(ui.sbSymbolBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::symbolsBorderWidthChanged);

	//Tab "Whiskers"
	connect(ui.cbWhiskersType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::whiskersTypeChanged);
	connect(ui.cbWhiskersStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::whiskersStyleChanged);
	connect(ui.sbWhiskersCapSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::whiskersCapSizeChanged);
	connect(ui.kcbWhiskersColor, &KColorButton::changed, this, &BoxPlotDock::whiskersColorChanged);
	connect(ui.sbWhiskersWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::whiskersWidthChanged);
	connect(ui.sbWhiskersOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::whiskersOpacityChanged);

	//template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &BoxPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &BoxPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &BoxPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void BoxPlotDock::setBoxPlots(QList<BoxPlot*> list) {
	const Lock lock(m_initializing);
	m_boxPlots = list;
	m_boxPlot = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_boxPlot);
	m_aspectTreeModel = new AspectTreeModel(m_boxPlot->project());
	setModel();

	//if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_boxPlot->name());
		ui.leComment->setText(m_boxPlot->comment());

		ui.lDataColumn->setEnabled(true);
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.leComment->setText(QString());

		ui.lDataColumn->setEnabled(false);
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first box plot
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);
	loadDataColumns();

	//SIGNALs/SLOTs
	//general
	connect(m_boxPlot, &AbstractAspect::aspectDescriptionChanged,this, &BoxPlotDock::plotDescriptionChanged);
	connect(m_boxPlot, &BoxPlot::visibilityChanged, this, &BoxPlotDock::plotVisibilityChanged);
	connect(m_boxPlot, &BoxPlot::dataColumnsChanged, this, &BoxPlotDock::plotDataColumnsChanged);

	//box filling
	connect(m_boxPlot, &BoxPlot::fillingEnabledChanged, this, &BoxPlotDock::plotFillingEnabledChanged);
	connect(m_boxPlot, &BoxPlot::fillingTypeChanged, this, &BoxPlotDock::plotFillingTypeChanged);
	connect(m_boxPlot, &BoxPlot::fillingColorStyleChanged, this, &BoxPlotDock::plotFillingColorStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingImageStyleChanged, this, &BoxPlotDock::plotFillingImageStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingBrushStyleChanged, this, &BoxPlotDock::plotFillingBrushStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingFirstColorChanged, this, &BoxPlotDock::plotFillingFirstColorChanged);
	connect(m_boxPlot, &BoxPlot::fillingSecondColorChanged, this, &BoxPlotDock::plotFillingSecondColorChanged);
	connect(m_boxPlot, &BoxPlot::fillingFileNameChanged, this, &BoxPlotDock::plotFillingFileNameChanged);
	connect(m_boxPlot, &BoxPlot::fillingOpacityChanged, this, &BoxPlotDock::plotFillingOpacityChanged);

	//box border
	connect(m_boxPlot, &BoxPlot::borderPenChanged, this, &BoxPlotDock::plotBorderPenChanged);
	connect(m_boxPlot, &BoxPlot::borderOpacityChanged, this, &BoxPlotDock::plotBorderOpacityChanged);

	//median line
	connect(m_boxPlot, &BoxPlot::medianLinePenChanged, this, &BoxPlotDock::plotMedianLinePenChanged);
	connect(m_boxPlot, &BoxPlot::medianLineOpacityChanged, this, &BoxPlotDock::plotMedianLineOpacityChanged);

	//markers
	connect(m_boxPlot, &BoxPlot::symbolOutliersStyleChanged, this, &BoxPlotDock::plotSymbolOutliersStyleChanged);
	connect(m_boxPlot, &BoxPlot::symbolMeanStyleChanged, this, &BoxPlotDock::plotSymbolMeanStyleChanged);
	connect(m_boxPlot, &BoxPlot::symbolsSizeChanged, this, &BoxPlotDock::plotSymbolsSizeChanged);
	connect(m_boxPlot, &BoxPlot::symbolsRotationAngleChanged, this, &BoxPlotDock::plotSymbolsRotationAngleChanged);
	connect(m_boxPlot, &BoxPlot::symbolsOpacityChanged, this, &BoxPlotDock::plotSymbolsOpacityChanged);
	connect(m_boxPlot, &BoxPlot::symbolsBrushChanged, this, &BoxPlotDock::plotSymbolsBrushChanged);
	connect(m_boxPlot, &BoxPlot::symbolsPenChanged, this, &BoxPlotDock::plotSymbolsPenChanged);

	//whiskers
	connect(m_boxPlot, &BoxPlot::whiskersTypeChanged, this, &BoxPlotDock::plotWhiskersTypeChanged);
	connect(m_boxPlot, &BoxPlot::whiskersPenChanged, this, &BoxPlotDock::plotWhiskersPenChanged);
	connect(m_boxPlot, &BoxPlot::whiskersCapSizeChanged, this, &BoxPlotDock::plotWhiskersCapSizeChanged);
	connect(m_boxPlot, &BoxPlot::whiskersOpacityChanged, this, &BoxPlotDock::plotWhiskersOpacityChanged);
}

void BoxPlotDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void BoxPlotDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbBorderWidth->setLocale(numberLocale);

// 	Lock lock(m_initializing);
// 	ui.lePosition->setText(numberLocale.toString(m_boxPlot->position()));
}

void BoxPlotDock::loadDataColumns() {
	//add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_boxPlot->dataColumns().count();
	if (count != 0) {
		//box plot has already data columns, make sure we have the proper number of comboboxes
		int diff = count - m_dataComboBoxes.count();
		if (diff > 0) {
			for (int i = 0; i < diff; ++i)
				addDataColumn();
		} else if (diff < 0) {
			for (int i = diff; i != 0; ++i)
				removeDataColumn();
		}

		//show the columns in the comboboxes
		for (int i = 0; i < count; ++i)
			m_dataComboBoxes.at(i)->setAspect(m_boxPlot->dataColumns().at(i));
	} else {
		//no data columns set in the box plot yet, we show the first combo box only
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 1; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	//disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_boxPlots.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);
}

void BoxPlotDock::setDataColumns() const {
	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_boxPlot->setDataColumns(columns);
}

//**********************************************************
//*** SLOTs for changes triggered in BoxPlotDock *****
//**********************************************************
void BoxPlotDock::addDataColumn() {
	TreeViewComboBox* cb = new TreeViewComboBox;

	static const QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	                       AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	                       AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	                       AspectType::XYFitCurve, AspectType::XYSmoothCurve, AspectType::CantorWorksheet};
	cb->setTopLevelClasses(list);
	cb->setModel(m_aspectTreeModel);
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &BoxPlotDock::dataColumnChanged);

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
		connect(button, &QPushButton::clicked, this, &BoxPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void BoxPlotDock::removeDataColumn() {
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (sender) {
		//remove button was clicked, determin which one and
		//delete it together with the corresponding combobox
		for (int i = 0; i < m_removeButtons.count(); ++i) {
			if (sender == m_removeButtons.at(i)) {
				delete m_dataComboBoxes.takeAt(i+1);
				delete m_removeButtons.takeAt(i);
			}
		}
	} else {
		//no sender is available, the function is being called directly in loadDataColumns().
		//delete the last remove button together with the corresponding combobox
		int index = m_removeButtons.count() - 1;
		if (index >= 0) {
			delete m_dataComboBoxes.takeAt(index+1);
			delete m_removeButtons.takeAt(index);
		}
	}

	if (!m_removeButtons.isEmpty())
		ui.lDataColumn->setText(i18n("Columns:"));
	else
		ui.lDataColumn->setText(i18n("Column:"));

	if (!m_initializing)
		setDataColumns();
}

void BoxPlotDock::dataColumnChanged(const QModelIndex&) const {
	if (m_initializing)
		return;

	setDataColumns();
}

void BoxPlotDock::visibilityChanged(bool state) const {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setVisible(state);
}

//"Box"-tab
//box filling
void BoxPlotDock::fillingEnabledChanged(int state) const {
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

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingEnabled(state);
}

void BoxPlotDock::fillingTypeChanged(int index) const {
	if (index == -1)
		return;

	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::BackgroundType::Color) {
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

		auto style = (PlotArea::BackgroundColorStyle)ui.cbFillingColorStyle->currentIndex();
		if (style == PlotArea::BackgroundColorStyle::SingleColor) {
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		} else {
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	} else if (type == PlotArea::BackgroundType::Image) {
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
	} else if (type == PlotArea::BackgroundType::Pattern) {
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

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingType(type);
}

void BoxPlotDock::fillingColorStyleChanged(int index) const {
	if (index == -1)
		return;

	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::BackgroundColorStyle::SingleColor) {
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

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingColorStyle(style);
}

void BoxPlotDock::fillingImageStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingImageStyle(style);
}

void BoxPlotDock::fillingBrushStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingBrushStyle(style);
}

void BoxPlotDock::fillingFirstColorChanged(const QColor& c) const {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFirstColor(c);
}

void BoxPlotDock::fillingSecondColorChanged(const QColor& c) const {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void BoxPlotDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "BoxPlotDock");
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

	ui.leFillingFileName->setText( path );

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFileName(path);
}

void BoxPlotDock::fileNameChanged() const {
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFileName(fileName);
}

void BoxPlotDock::fillingOpacityChanged(int value) const {
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingOpacity(opacity);
}

//box border
void BoxPlotDock::borderStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setStyle(penStyle);
		boxPlot->setBorderPen(pen);
	}
}

void BoxPlotDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setColor(color);
		boxPlot->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void BoxPlotDock::borderWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		boxPlot->setBorderPen(pen);
	}
}

void BoxPlotDock::borderOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setBorderOpacity(opacity);
}

//median line
void BoxPlotDock::medianLineStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->medianLinePen();
		pen.setStyle(penStyle);
		boxPlot->setMedianLinePen(pen);
	}
}

void BoxPlotDock::medianLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->medianLinePen();
		pen.setColor(color);
		boxPlot->setMedianLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbMedianLineStyle, color);
	m_initializing = false;
}

void BoxPlotDock::medianLineWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->medianLinePen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		boxPlot->setMedianLinePen(pen);
	}
}

void BoxPlotDock::medianLineOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setMedianLineOpacity(opacity);
}

//markers
void BoxPlotDock::symbolOutliersStyleChanged(int index) {
	updateSymbolWidgets();

	if (m_initializing)
		return;

	const auto style = Symbol::Style(index);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setSymbolOutliersStyle(style);
}

void BoxPlotDock::symbolMeanStyleChanged(int index) {
	updateSymbolWidgets();

	if (m_initializing)
		return;

	const auto style = Symbol::Style(index);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setSymbolMeanStyle(style);
}

void BoxPlotDock::updateSymbolWidgets() {
	const auto styleOutliers = Symbol::Style(ui.cbSymbolOutliersStyle->currentIndex());
	const auto styleMean = Symbol::Style(ui.cbSymbolMeanStyle->currentIndex());

	if (styleOutliers == Symbol::Style::NoSymbols && styleMean == Symbol::Style::NoSymbols) {
		ui.sbSymbolSize->setEnabled(false);
		ui.sbSymbolRotation->setEnabled(false);
		ui.sbSymbolOpacity->setEnabled(false);

		ui.kcbSymbolFillingColor->setEnabled(false);
		ui.cbSymbolFillingStyle->setEnabled(false);

		ui.cbSymbolBorderStyle->setEnabled(false);
		ui.kcbSymbolBorderColor->setEnabled(false);
		ui.sbSymbolBorderWidth->setEnabled(false);
	} else {
		ui.sbSymbolSize->setEnabled(true);
		ui.sbSymbolRotation->setEnabled(true);
		ui.sbSymbolOpacity->setEnabled(true);

		//enable/disable the symbol filling options in the GUI depending on the currently selected symbol.
		if ( (styleOutliers != Symbol::Style::Line && styleOutliers != Symbol::Style::Cross)
			|| (styleMean != Symbol::Style::Line && styleMean != Symbol::Style::Cross) ) {
			ui.cbSymbolFillingStyle->setEnabled(true);
			bool noBrush = (Qt::BrushStyle(ui.cbSymbolFillingStyle->currentIndex()) == Qt::NoBrush);
			ui.kcbSymbolFillingColor->setEnabled(!noBrush);
		} else {
			ui.kcbSymbolFillingColor->setEnabled(false);
			ui.cbSymbolFillingStyle->setEnabled(false);
		}

		ui.cbSymbolBorderStyle->setEnabled(true);
		bool noLine = (Qt::PenStyle(ui.cbSymbolBorderStyle->currentIndex()) == Qt::NoPen);
		ui.kcbSymbolBorderColor->setEnabled(!noLine);
		ui.sbSymbolBorderWidth->setEnabled(!noLine);
	}

}
void BoxPlotDock::symbolsSizeChanged(double value) const {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setSymbolsSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void BoxPlotDock::symbolsRotationChanged(int value) const {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setSymbolsRotationAngle(value);
}

void BoxPlotDock::symbolsOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setSymbolsOpacity(opacity);
}

void BoxPlotDock::symbolsFillingStyleChanged(int index) const {
	if (index == -1)
		return;

	const auto brushStyle = Qt::BrushStyle(index);
	ui.kcbSymbolFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	for (auto* boxPlot : m_boxPlots) {
		brush = boxPlot->symbolsBrush();
		brush.setStyle(brushStyle);
		boxPlot->setSymbolsBrush(brush);
	}
}

void BoxPlotDock::symbolsFillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	for (auto* boxPlot : m_boxPlots) {
		brush = boxPlot->symbolsBrush();
		brush.setColor(color);
		boxPlot->setSymbolsBrush(brush);
	}

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color );
	m_initializing = false;
}

void BoxPlotDock::symbolsBorderStyleChanged(int index) const {
	if (index == -1)
		return;

	const auto penStyle = Qt::PenStyle(index);
	if ( penStyle == Qt::NoPen ) {
		ui.kcbSymbolBorderColor->setEnabled(false);
		ui.sbSymbolBorderWidth->setEnabled(false);
	} else {
		ui.kcbSymbolBorderColor->setEnabled(true);
		ui.sbSymbolBorderWidth->setEnabled(true);
	}

	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->symbolsPen();
		pen.setStyle(penStyle);
		boxPlot->setSymbolsPen(pen);
	}
}

void BoxPlotDock::symbolsBorderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->symbolsPen();
		pen.setColor(color);
		boxPlot->setSymbolsPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
	m_initializing = false;
}

void BoxPlotDock::symbolsBorderWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->symbolsPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		boxPlot->setSymbolsPen(pen);
	}
}

//whiskers
void BoxPlotDock::whiskersTypeChanged(int index) const {
	if (m_initializing)
		return;

	auto type = BoxPlot::WhiskersType(index);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersType(type);
}

void BoxPlotDock::whiskersStyleChanged(int index) const {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->whiskersPen();
		pen.setStyle(penStyle);
		boxPlot->setWhiskersPen(pen);
	}
}

void BoxPlotDock::whiskersColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->whiskersPen();
		pen.setColor(color);
		boxPlot->setWhiskersPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbWhiskersStyle, color);
	m_initializing = false;
}

void BoxPlotDock::whiskersWidthChanged(double value) const {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->whiskersPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		boxPlot->setWhiskersPen(pen);
	}
}

void BoxPlotDock::whiskersOpacityChanged(int value) const {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersOpacity(opacity);
}

void BoxPlotDock::whiskersCapSizeChanged(double value) const {
	if (m_initializing)
		return;

	float size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersCapSize(size);
}

//*************************************************************
//******* SLOTs for changes triggered in BoxPlot ********
//*************************************************************
//general
void BoxPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_boxPlot != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

void BoxPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
	Lock lock(m_initializing);
	loadDataColumns();
}

void BoxPlotDock::plotVisibilityChanged(bool on) {
	Lock lock(m_initializing);
	ui.chkVisible->setChecked(on);
}

//box filling
//Filling
void BoxPlotDock::plotFillingEnabledChanged(bool status) {
	Lock lock(m_initializing);
	ui.chkFillingEnabled->setChecked(status);
}
void BoxPlotDock::plotFillingTypeChanged(PlotArea::BackgroundType type) {
	Lock lock(m_initializing);
	ui.cbFillingType->setCurrentIndex(static_cast<int>(type));
}
void BoxPlotDock::plotFillingColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingColorStyle->setCurrentIndex(static_cast<int>(style));
}
void BoxPlotDock::plotFillingImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingImageStyle->setCurrentIndex(static_cast<int>(style));
}
void BoxPlotDock::plotFillingBrushStyleChanged(Qt::BrushStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingBrushStyle->setCurrentIndex(style);
}
void BoxPlotDock::plotFillingFirstColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingFirstColor->setColor(color);
}
void BoxPlotDock::plotFillingSecondColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingSecondColor->setColor(color);
}
void BoxPlotDock::plotFillingFileNameChanged(QString& filename) {
	Lock lock(m_initializing);
	ui.leFillingFileName->setText(filename);
}
void BoxPlotDock::plotFillingOpacityChanged(double opacity) {
	Lock lock(m_initializing);
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
}

//box border
void BoxPlotDock::plotBorderPenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}
void BoxPlotDock::plotBorderOpacityChanged(float value) {
	Lock lock(m_initializing);
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
}

//median line
void BoxPlotDock::plotMedianLinePenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbMedianLineStyle->currentIndex() != pen.style())
		ui.cbMedianLineStyle->setCurrentIndex(pen.style());
	if (ui.kcbMedianLineColor->color() != pen.color())
		ui.kcbMedianLineColor->setColor(pen.color());
	if (ui.sbMedianLineWidth->value() != pen.widthF())
		ui.sbMedianLineWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}
void BoxPlotDock::plotMedianLineOpacityChanged(float value) {
	Lock lock(m_initializing);
	float v = (float)value*100.;
	ui.sbMedianLineOpacity->setValue(v);
}

//markers
void BoxPlotDock::plotSymbolOutliersStyleChanged(Symbol::Style style) {
	Lock lock(m_initializing);
	ui.cbSymbolOutliersStyle->setCurrentIndex((int)style);
}
void BoxPlotDock::plotSymbolMeanStyleChanged(Symbol::Style style) {
	Lock lock(m_initializing);
	ui.cbSymbolMeanStyle->setCurrentIndex((int)style);
}
void BoxPlotDock::plotSymbolsSizeChanged(qreal size) {
	Lock lock(m_initializing);
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
}
void BoxPlotDock::plotSymbolsRotationAngleChanged(qreal angle) {
	Lock lock(m_initializing);
	ui.sbSymbolRotation->setValue(angle);
}
void BoxPlotDock::plotSymbolsOpacityChanged(qreal opacity) {
	Lock lock(m_initializing);
	ui.sbSymbolOpacity->setValue( round(opacity*100.0) );
}
void BoxPlotDock::plotSymbolsBrushChanged(const QBrush& brush) {
	Lock lock(m_initializing);
	ui.cbSymbolFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbSymbolFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, brush.color());
}
void BoxPlotDock::plotSymbolsPenChanged(const QPen& pen) {
	Lock lock(m_initializing);
	ui.cbSymbolBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbSymbolBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, pen.color());
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

//whiskers
void BoxPlotDock::plotWhiskersTypeChanged(BoxPlot::WhiskersType type) {
	Lock lock(m_initializing);
	ui.cbWhiskersType->setCurrentIndex((int)type);
}

void BoxPlotDock::plotWhiskersPenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbWhiskersStyle->currentIndex() != pen.style())
		ui.cbWhiskersStyle->setCurrentIndex(pen.style());
	if (ui.kcbWhiskersColor->color() != pen.color())
		ui.kcbWhiskersColor->setColor(pen.color());
	if (ui.sbWhiskersWidth->value() != pen.widthF())
		ui.sbWhiskersWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}
void BoxPlotDock::plotWhiskersOpacityChanged(float value) {
	Lock lock(m_initializing);
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
}
void BoxPlotDock::plotWhiskersCapSizeChanged(double size) {
	Lock lock(m_initializing);
	ui.sbWhiskersCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BoxPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("BoxPlot"));

	//box filling
	ui.chkFillingEnabled->setChecked( group.readEntry("FillingEnabled", m_boxPlot->fillingEnabled()) );
	ui.cbFillingType->setCurrentIndex( group.readEntry("FillingType", (int) m_boxPlot->fillingType()) );
	ui.cbFillingColorStyle->setCurrentIndex( group.readEntry("FillingColorStyle", (int) m_boxPlot->fillingColorStyle()) );
	ui.cbFillingImageStyle->setCurrentIndex( group.readEntry("FillingImageStyle", (int) m_boxPlot->fillingImageStyle()) );
	ui.cbFillingBrushStyle->setCurrentIndex( group.readEntry("FillingBrushStyle", (int) m_boxPlot->fillingBrushStyle()) );
	ui.leFillingFileName->setText( group.readEntry("FillingFileName", m_boxPlot->fillingFileName()) );
	ui.kcbFillingFirstColor->setColor( group.readEntry("FillingFirstColor", m_boxPlot->fillingFirstColor()) );
	ui.kcbFillingSecondColor->setColor( group.readEntry("FillingSecondColor", m_boxPlot->fillingSecondColor()) );
	ui.sbFillingOpacity->setValue( round(group.readEntry("FillingOpacity", m_boxPlot->fillingOpacity())*100.0) );

	//update the box filling widgets
	fillingEnabledChanged(ui.chkFillingEnabled->isChecked());
	fillingTypeChanged(ui.cbFillingType->currentIndex());

	//box border
	const QPen& penBorder = m_boxPlot->borderPen();
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int)penBorder.style()) );
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", penBorder.color()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", penBorder.widthF()), Worksheet::Unit::Point) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_boxPlot->borderOpacity())*100 );

	//median line
	const QPen& penMedian = m_boxPlot->medianLinePen();
	ui.cbMedianLineStyle->setCurrentIndex( group.readEntry("MedianLineStyle", (int)penMedian.style()) );
	ui.kcbMedianLineColor->setColor( group.readEntry("MedianLineColor", penMedian.color()) );
	ui.sbMedianLineWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("MedianLineWidth", penMedian.widthF()), Worksheet::Unit::Point) );
	ui.sbMedianLineOpacity->setValue( group.readEntry("MedianLineOpacity", m_boxPlot->borderOpacity())*100 );

	//symbols
	const QPen& penSymbol = m_boxPlot->symbolsPen();
	ui.cbSymbolOutliersStyle->setCurrentIndex( group.readEntry("SymbolOutliersStyle", (int)m_boxPlot->symbolOutliersStyle()) );
	ui.cbSymbolMeanStyle->setCurrentIndex( group.readEntry("SymbolMeanStyle", (int)m_boxPlot->symbolMeanStyle()) );
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolSize", m_boxPlot->symbolsSize()), Worksheet::Unit::Point) );
	ui.sbSymbolRotation->setValue( group.readEntry("SymbolRotation", m_boxPlot->symbolsRotationAngle()) );
	ui.sbSymbolOpacity->setValue( round(group.readEntry("SymbolOpacity", m_boxPlot->symbolsOpacity())*100.0) );
	ui.cbSymbolFillingStyle->setCurrentIndex( group.readEntry("SymbolFillingStyle", (int) m_boxPlot->symbolsBrush().style()) );
	ui.kcbSymbolFillingColor->setColor(  group.readEntry("SymbolFillingColor", m_boxPlot->symbolsBrush().color()) );
	ui.cbSymbolBorderStyle->setCurrentIndex( group.readEntry("SymbolBorderStyle", (int)penSymbol.style()) );
	ui.kcbSymbolBorderColor->setColor( group.readEntry("SymbolBorderColor", penSymbol.color()) );
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolBorderWidth", penSymbol.widthF()), Worksheet::Unit::Point) );

	//whiskers
	const QPen& penWhiskers = m_boxPlot->whiskersPen();
	ui.cbWhiskersType->setCurrentIndex( group.readEntry("WhiskersType", (int)m_boxPlot->whiskersType()) );
	ui.cbWhiskersStyle->setCurrentIndex( group.readEntry("WhiskersStyle", (int)penWhiskers.style()) );
	ui.kcbWhiskersColor->setColor( group.readEntry("WhiskersColor", penWhiskers.color()) );
	ui.sbWhiskersWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("WhiskersWidth", penWhiskers.widthF()), Worksheet::Unit::Point) );
	ui.sbWhiskersOpacity->setValue( group.readEntry("WhiskersOpacity", m_boxPlot->whiskersOpacity())*100 );
	ui.sbWhiskersCapSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("WhiskersCapSize", m_boxPlot->whiskersCapSize()), Worksheet::Unit::Point) );
}

void BoxPlotDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_boxPlots.size();
	if (size > 1)
		m_boxPlot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_boxPlot->beginMacro(i18n("%1: template \"%2\" loaded", m_boxPlot->name(), name));

	this->loadConfig(config);

	m_boxPlot->endMacro();
}

void BoxPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("BoxPlot");

	//box filling
	group.writeEntry("FillingEnabled", ui.chkFillingEnabled->isChecked());
	group.writeEntry("FillingType", ui.cbFillingType->currentIndex());
	group.writeEntry("FillingColorStyle", ui.cbFillingColorStyle->currentIndex());
	group.writeEntry("FillingImageStyle", ui.cbFillingImageStyle->currentIndex());
	group.writeEntry("FillingBrushStyle", ui.cbFillingBrushStyle->currentIndex());
	group.writeEntry("FillingFileName", ui.leFillingFileName->text());
	group.writeEntry("FillingFirstColor", ui.kcbFillingFirstColor->color());
	group.writeEntry("FillingSecondColor", ui.kcbFillingSecondColor->color());
	group.writeEntry("FillingOpacity", ui.sbFillingOpacity->value()/100.0);

	//box border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	//median line
	group.writeEntry("MedianLineStyle", ui.cbMedianLineStyle->currentIndex());
	group.writeEntry("MedianLineColor", ui.kcbMedianLineColor->color());
	group.writeEntry("MedianLineWidth", Worksheet::convertToSceneUnits(ui.sbMedianLineWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("MedianLineOpacity", ui.sbMedianLineOpacity->value()/100.0);

	//symbols for the outliers and for the mean
	group.writeEntry("SymbolOutliersStyle", ui.cbSymbolOutliersStyle->currentIndex());
	group.writeEntry("SymbolMeanStyle", ui.cbSymbolMeanStyle->currentIndex());
	group.writeEntry("SymbolSize", Worksheet::convertToSceneUnits(ui.sbSymbolSize->value(),Worksheet::Unit::Point));
	group.writeEntry("SymbolRotation", ui.sbSymbolRotation->value());
	group.writeEntry("SymbolOpacity", ui.sbSymbolOpacity->value()/100.0);
	group.writeEntry("SymbolFillingStyle", ui.cbSymbolFillingStyle->currentIndex());
	group.writeEntry("SymbolFillingColor", ui.kcbSymbolFillingColor->color());
	group.writeEntry("SymbolBorderStyle", ui.cbSymbolBorderStyle->currentIndex());
	group.writeEntry("SymbolBorderColor", ui.kcbSymbolBorderColor->color());
	group.writeEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(ui.sbSymbolBorderWidth->value(), Worksheet::Unit::Point));

	//whiskers
	group.writeEntry("WhiskersType", ui.cbWhiskersType->currentIndex());
	group.writeEntry("WhiskersStyle", ui.cbWhiskersStyle->currentIndex());
	group.writeEntry("WhiskersColor", ui.kcbWhiskersColor->color());
	group.writeEntry("WhiskersWidth", Worksheet::convertToSceneUnits(ui.sbWhiskersWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("WhiskersOpacity", ui.sbWhiskersOpacity->value()/100.0);
	group.writeEntry("WhiskersCapSize", Worksheet::convertToSceneUnits(ui.sbWhiskersCapSize->value(), Worksheet::Unit::Point));

	config.sync();
}
