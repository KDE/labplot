/***************************************************************************
    File                 : WorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for worksheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "WorksheetDock.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/TemplateHandler.h"

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QPageSize>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class WorksheetDock
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget *parent): QWidget(parent) {
	ui.setupUi(this);

	//Background-tab
	ui.cbBackgroundColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.leBackgroundFileName->setCompleter(new QCompleter(new QDirModel, this));

	//Layout-tab
	ui.chScaleContent->setToolTip(i18n("If checked, rescale the content of the worksheet on size changes. Otherwise resize the canvas only."));

	ui.cbLayout->addItem(QIcon::fromTheme("labplot-editbreaklayout"), i18n("No Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme("labplot-editvlayout"), i18n("Vertical Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme("labplot-edithlayout"), i18n("Horizontal Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme("labplot-editgrid"), i18n("Grid Layout"));

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//SLOTs
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &WorksheetDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &WorksheetDock::commentChanged);
	connect(ui.cbSize, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, static_cast<void (WorksheetDock::*)(int)>(&WorksheetDock::sizeChanged));
	connect(ui.sbWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, static_cast<void (WorksheetDock::*)()>(&WorksheetDock::sizeChanged));
	connect(ui.sbHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, static_cast<void (WorksheetDock::*)()>(&WorksheetDock::sizeChanged));
	connect(ui.cbOrientation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::orientationChanged);

	//Background
	connect(ui.cbBackgroundType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundTypeChanged);
	connect(ui.cbBackgroundColorStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundColorStyleChanged);
	connect(ui.cbBackgroundImageStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundImageStyleChanged);
	connect(ui.cbBackgroundBrushStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundBrushStyleChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &WorksheetDock::selectFile);
	connect(ui.leBackgroundFileName, &QLineEdit::returnPressed, this, &WorksheetDock::fileNameChanged);
	connect(ui.leBackgroundFileName, &QLineEdit::textChanged, this, &WorksheetDock::fileNameChanged);
	connect(ui.kcbBackgroundFirstColor, &KColorButton::changed, this, &WorksheetDock::backgroundFirstColorChanged);
	connect(ui.kcbBackgroundSecondColor, &KColorButton::changed, this, &WorksheetDock::backgroundSecondColorChanged);
	connect(ui.sbBackgroundOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &WorksheetDock::backgroundOpacityChanged);

	//Layout
	connect(ui.cbLayout, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::layoutChanged);
	connect( ui.chScaleContent, &QCheckBox::clicked, this, &WorksheetDock::scaleContentChanged);
	connect( ui.sbLayoutTopMargin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutTopMarginChanged);
	connect( ui.sbLayoutBottomMargin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutBottomMarginChanged);
	connect( ui.sbLayoutLeftMargin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutLeftMarginChanged);
	connect( ui.sbLayoutRightMargin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutRightMarginChanged);
	connect( ui.sbLayoutHorizontalSpacing, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutHorizontalSpacingChanged);
	connect( ui.sbLayoutVerticalSpacing, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutVerticalSpacingChanged);
	connect( ui.sbLayoutRowCount, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			 this, &WorksheetDock::layoutRowCountChanged);
	connect( ui.sbLayoutColumnCount, static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged),
			 this, &WorksheetDock::layoutColumnCountChanged);

	//theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);

	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, &ThemeHandler::loadThemeRequested, this, &WorksheetDock::loadTheme);
	connect(m_themeHandler, &ThemeHandler::info, this, &WorksheetDock::info);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &WorksheetDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &WorksheetDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &WorksheetDock::info);

	ui.verticalLayout->addWidget(frame);

	this->retranslateUi();
}

void WorksheetDock::setWorksheets(QList<Worksheet*> list) {
	m_initializing = true;
	m_worksheetList = list;
	m_worksheet = list.first();

	//if there are more then one worksheet in the list, disable the name and comment field in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_worksheet->name());
		ui.leComment->setText(m_worksheet->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	//show the properties of the first worksheet
	this->load();
	this->worksheetLayoutChanged(m_worksheet->layout());

	m_themeHandler->setCurrentTheme(m_worksheet->theme());

	connect(m_worksheet, &Worksheet::aspectDescriptionChanged, this, &WorksheetDock::worksheetDescriptionChanged);
	connect(m_worksheet, &Worksheet::pageRectChanged, this, &WorksheetDock::worksheetPageRectChanged);
	connect(m_worksheet, &Worksheet::scaleContentChanged, this, &WorksheetDock::worksheetScaleContentChanged);

	connect(m_worksheet, &Worksheet::backgroundTypeChanged, this, &WorksheetDock::worksheetBackgroundTypeChanged);
	connect(m_worksheet, &Worksheet::backgroundColorStyleChanged, this, &WorksheetDock::worksheetBackgroundColorStyleChanged);
	connect(m_worksheet, &Worksheet::backgroundImageStyleChanged, this, &WorksheetDock::worksheetBackgroundImageStyleChanged);
	connect(m_worksheet, &Worksheet::backgroundBrushStyleChanged, this, &WorksheetDock::worksheetBackgroundBrushStyleChanged);
	connect(m_worksheet, &Worksheet::backgroundFirstColorChanged, this, &WorksheetDock::worksheetBackgroundFirstColorChanged);
	connect(m_worksheet, &Worksheet::backgroundSecondColorChanged, this, &WorksheetDock::worksheetBackgroundSecondColorChanged);
	connect(m_worksheet, &Worksheet::backgroundFileNameChanged, this, &WorksheetDock::worksheetBackgroundFileNameChanged);
	connect(m_worksheet, &Worksheet::backgroundOpacityChanged, this, &WorksheetDock::worksheetBackgroundOpacityChanged);

	connect(m_worksheet, &Worksheet::layoutChanged, this, &WorksheetDock::worksheetLayoutChanged);
	connect(m_worksheet, &Worksheet::layoutTopMarginChanged, this, &WorksheetDock::worksheetLayoutTopMarginChanged);
	connect(m_worksheet, &Worksheet::layoutBottomMarginChanged, this, &WorksheetDock::worksheetLayoutBottomMarginChanged);
	connect(m_worksheet, &Worksheet::layoutLeftMarginChanged, this, &WorksheetDock::worksheetLayoutLeftMarginChanged);
	connect(m_worksheet, &Worksheet::layoutRightMarginChanged, this, &WorksheetDock::worksheetLayoutRightMarginChanged);
	connect(m_worksheet, &Worksheet::layoutVerticalSpacingChanged, this, &WorksheetDock::worksheetLayoutVerticalSpacingChanged);
	connect(m_worksheet, &Worksheet::layoutHorizontalSpacingChanged, this, &WorksheetDock::worksheetLayoutHorizontalSpacingChanged);
	connect(m_worksheet, &Worksheet::layoutRowCountChanged, this, &WorksheetDock::worksheetLayoutRowCountChanged);
	connect(m_worksheet, &Worksheet::layoutColumnCountChanged, this, &WorksheetDock::worksheetLayoutColumnCountChanged);

	connect(m_worksheet, &Worksheet::themeChanged, m_themeHandler, &ThemeHandler::setCurrentTheme);

	m_initializing = false;
}

/*!
	Checks whether the size is one of the QPageSize::PageSizeId and
	updates Size and Orientation checkbox when width/height changes.
*/
void WorksheetDock::updatePaperSize() {
	if (m_worksheet->useViewSize()) {
		ui.cbSize->setCurrentIndex(0);
		return;
	}

	//In UI we use cm, so we need to convert to mm first before we check with QPageSize
	float w = (float)ui.sbWidth->value()*10;
	float h = (float)ui.sbHeight->value()*10;
	const QSizeF s = QSizeF(w, h);
	const QSizeF st = s.transposed();

	//determine the position of the QPageSize::PageSizeId in the combobox
	bool found = false;
	for (int i = 0; i < ui.cbSize->count(); ++i) {
		const QVariant v = ui.cbSize->itemData(i);
		if (!v.isValid())
			continue;

		const auto id = v.value<QPageSize::PageSizeId>();
		const QSizeF ps = QPageSize::size(id, QPageSize::Millimeter);
		if (s == ps) { //check the portrait-orientation first
			ui.cbSize->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(0);  //a QPageSize::PaperSize in portrait-orientation was found
			found = true;
			break;
		} else if (st == ps) { //check for the landscape-orientation
			ui.cbSize->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(1); //a QPageSize::PaperSize in landscape-orientation was found
			found = true;
			break;
		}
	}

	if (!found)
		ui.cbSize->setCurrentIndex(ui.cbSize->count() - 1); //select "Custom" size
}

//*************************************************************
//****** SLOTs for changes triggered in WorksheetDock *********
//*************************************************************
void WorksheetDock::retranslateUi() {
	m_initializing = true;

	//Geometry
	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Portrait"));
	ui.cbOrientation->addItem(i18n("Landscape"));

	const QVector<QPageSize::PageSizeId> pageSizes = {
		QPageSize::A0,
		QPageSize::A1,
		QPageSize::A2,
		QPageSize::A3,
		QPageSize::A4,
		QPageSize::A5,
		QPageSize::A6,
		QPageSize::A7,
		QPageSize::A8,
		QPageSize::A9,
		QPageSize::B0,
		QPageSize::B1,
		QPageSize::B2,
		QPageSize::B3,
		QPageSize::B4,
		QPageSize::B5,
		QPageSize::B6,
		QPageSize::B7,
		QPageSize::B8,
		QPageSize::B9,
		QPageSize::B10,
		QPageSize::C5E,
		QPageSize::DLE,
		QPageSize::Executive,
		QPageSize::Folio,
		QPageSize::Ledger,
		QPageSize::Legal,
		QPageSize::Letter,
		QPageSize::Tabloid,
		QPageSize::Comm10E,
		QPageSize::Custom,
	};
	ui.cbSize->clear();
	ui.cbSize->addItem(i18n("View Size"));
	for (QPageSize::PageSizeId id : pageSizes) {
		ui.cbSize->addItem(QPageSize::name(id), id);
	}
	ui.cbSize->insertSeparator(1);

	//Background
	ui.cbBackgroundType->clear();
	ui.cbBackgroundType->addItem(i18n("Color"));
	ui.cbBackgroundType->addItem(i18n("Image"));
	ui.cbBackgroundType->addItem(i18n("Pattern"));

	ui.cbBackgroundColorStyle->clear();
	ui.cbBackgroundColorStyle->addItem(i18n("Single Color"));
	ui.cbBackgroundColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbBackgroundImageStyle->clear();
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("Centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("Tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Center Tiled"));
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	m_initializing = false;
}

// "General"-tab
void WorksheetDock::nameChanged() {
	if (m_initializing)
		return;

	m_worksheet->setName(ui.leName->text());
}

void WorksheetDock::commentChanged() {
	if (m_initializing)
		return;

	m_worksheet->setComment(ui.leComment->text());
}

void WorksheetDock::scaleContentChanged(bool scaled) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setScaleContent(scaled);
}

void WorksheetDock::sizeChanged(int i) {
	const auto index = ui.cbSize->itemData(i).value<QPageSize::PageSizeId>();

	if (index == QPageSize::Custom) {
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
	} else {
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
		if (i == 0) { //no orientation available when using the complete view size (first item in the combox is selected)
			ui.lOrientation->hide();
			ui.cbOrientation->hide();
		} else {
			ui.lOrientation->show();
			ui.cbOrientation->show();
		}
	}

	if (m_initializing)
		return;

	if (i == 0) {
		//use the complete view size (first item in the combox is selected)
		for (auto* worksheet : m_worksheetList)
			worksheet->setUseViewSize(true);
	} else if (index == QPageSize::Custom) {
		if (m_worksheet->useViewSize()) {
			for (auto* worksheet : m_worksheetList)
				worksheet->setUseViewSize(false);
		}
	} else {
		//determine the width and the height of the to be used predefined layout
		QSizeF s = QPageSize::size(index, QPageSize::Millimeter);
		if (ui.cbOrientation->currentIndex() == 1) {
			s.transpose();
		}

		m_initializing = true;
		//s is in mm, in UI we show everything in cm
		ui.sbWidth->setValue(s.width()/10);
		ui.sbHeight->setValue(s.height()/10);
		m_initializing = false;

		float w = Worksheet::convertToSceneUnits(s.width(), Worksheet::Millimeter);
		float h = Worksheet::convertToSceneUnits(s.height(), Worksheet::Millimeter);
		for (auto* worksheet : m_worksheetList) {
			worksheet->setUseViewSize(false);
			worksheet->setPageRect(QRect(0,0,w,h));
		}
	}
}

void WorksheetDock::sizeChanged() {
	if (m_initializing)
		return;

	int w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	int h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);
	for (auto* worksheet : m_worksheetList)
		worksheet->setPageRect(QRect(0,0,w,h));
}

void WorksheetDock::orientationChanged(int index) {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	this->sizeChanged(ui.cbSize->currentIndex());
}

// "Background"-tab
void WorksheetDock::backgroundTypeChanged(int index) {
	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color) {
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

		auto style = (PlotArea::BackgroundColorStyle)ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::SingleColor) {
			ui.lBackgroundFirstColor->setText(i18n("Color:"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		} else {
			ui.lBackgroundFirstColor->setText(i18n("First color:"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	} else if (type == PlotArea::Image) {
		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.leBackgroundFileName->show();
		ui.bOpen->show();
	} else if (type == PlotArea::Pattern) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();
	}

	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundType(type);
}

void WorksheetDock::backgroundColorStyleChanged(int index) {
	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else {
		ui.lBackgroundFirstColor->setText(i18n("First color:"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
	}

	if (m_initializing)
		return;

	int size = m_worksheetList.size();
	if (size>1) {
		m_worksheet->beginMacro(i18n("%1 worksheets: background color style changed", size));
		for (auto* w : m_worksheetList)
			w->setBackgroundColorStyle(style);
		m_worksheet->endMacro();
	} else
		m_worksheet->setBackgroundColorStyle(style);
}

void WorksheetDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundImageStyle(style);
}

void WorksheetDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundBrushStyle(style);
}

void WorksheetDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFirstColor(c);
}

void WorksheetDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundSecondColor(c);
}

void WorksheetDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundOpacity(opacity);
}

//"Layout"-tab
void WorksheetDock::layoutChanged(int index) {
	Worksheet::Layout layout = (Worksheet::Layout)index;

	bool b = (layout != Worksheet::NoLayout);
	ui.sbLayoutTopMargin->setEnabled(b);
	ui.sbLayoutBottomMargin->setEnabled(b);
	ui.sbLayoutLeftMargin->setEnabled(b);
	ui.sbLayoutRightMargin->setEnabled(b);
	ui.sbLayoutHorizontalSpacing->setEnabled(b);
	ui.sbLayoutVerticalSpacing->setEnabled(b);
	ui.sbLayoutRowCount->setEnabled(b);
	ui.sbLayoutColumnCount->setEnabled(b);

	//show the "scale content" option if no layout active
	ui.lScaleContent->setVisible(!b);
	ui.chScaleContent->setVisible(!b);

	if (b) {
		//show grid specific settings if grid layout selected
		bool grid = (layout == Worksheet::GridLayout);
		ui.lGrid->setVisible(grid);
		ui.lRowCount->setVisible(grid);
		ui.sbLayoutRowCount->setVisible(grid);
		ui.lColumnCount->setVisible(grid);
		ui.sbLayoutColumnCount->setVisible(grid);
	} else {
		//no layout selected, hide grid specific settings that were potentially shown before
		ui.lGrid->setVisible(false);
		ui.lRowCount->setVisible(false);
		ui.sbLayoutRowCount->setVisible(false);
		ui.lColumnCount->setVisible(false);
		ui.sbLayoutColumnCount->setVisible(false);
	}

	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayout(layout);
}

void WorksheetDock::layoutTopMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutBottomMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutLeftMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutRightMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutHorizontalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void WorksheetDock::layoutVerticalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void WorksheetDock::layoutRowCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRowCount(count);
}

void WorksheetDock::layoutColumnCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutColumnCount(count);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void WorksheetDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "WorksheetDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leBackgroundFileName->setText( path );

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFileName(path);
}

void WorksheetDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leBackgroundFileName->text();
	if (!fileName.isEmpty() && !QFile::exists(fileName))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet(QString());

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFileName(fileName);
}

//*************************************************************
//******** SLOTs for changes triggered in Worksheet ***********
//*************************************************************
void WorksheetDock::worksheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_worksheet != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}

void WorksheetDock::worksheetScaleContentChanged(bool scaled) {
	m_initializing = true;
	ui.chScaleContent->setChecked(scaled);
	m_initializing = false;
}

void WorksheetDock::worksheetPageRectChanged(const QRectF& rect) {
	m_initializing = true;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Centimeter));
	updatePaperSize();
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(type);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundFirstColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundSecondColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(name);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutChanged(Worksheet::Layout layout) {
	m_initializing = true;
	ui.cbLayout->setCurrentIndex(layout);
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutTopMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutBottomMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutLeftMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutRightMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutVerticalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutHorizontalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutRowCountChanged(int value) {
	m_initializing = true;
	ui.sbLayoutRowCount->setValue(value);
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutColumnCountChanged(int value) {
	m_initializing = true;
	ui.sbLayoutColumnCount->setValue(value);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void WorksheetDock::load() {
	// Geometry
	ui.chScaleContent->setChecked(m_worksheet->scaleContent());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().width(), Worksheet::Centimeter) );
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().height(), Worksheet::Centimeter) );
	updatePaperSize();

	// Background-tab
	ui.cbBackgroundType->setCurrentIndex( (int) m_worksheet->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_worksheet->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_worksheet->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_worksheet->backgroundBrushStyle() );
	ui.leBackgroundFileName->setText( m_worksheet->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_worksheet->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_worksheet->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( qRound(m_worksheet->backgroundOpacity()*100) );

	//highlight the text field for the background image red if an image is used and cannot be found
	if (!m_worksheet->backgroundFileName().isEmpty() && !QFile::exists(m_worksheet->backgroundFileName()))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet(QString());

	// Layout
	ui.cbLayout->setCurrentIndex( (int) m_worksheet->layout() );
	ui.sbLayoutTopMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin(), Worksheet::Centimeter) );
	ui.sbLayoutBottomMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), Worksheet::Centimeter) );
	ui.sbLayoutLeftMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), Worksheet::Centimeter) );
	ui.sbLayoutRightMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), Worksheet::Centimeter) );
	ui.sbLayoutHorizontalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), Worksheet::Centimeter) );
	ui.sbLayoutVerticalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), Worksheet::Centimeter) );

	ui.sbLayoutRowCount->setValue( m_worksheet->layoutRowCount() );
	ui.sbLayoutColumnCount->setValue( m_worksheet->layoutColumnCount() );
}

void WorksheetDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_worksheetList.size();
	if (size > 1)
		m_worksheet->beginMacro(i18n("%1 worksheets: template \"%2\" loaded", size, name));
	else
		m_worksheet->beginMacro(i18n("%1: template \"%2\" loaded", m_worksheet->name(), name));

	this->loadConfig(config);
	m_worksheet->endMacro();
}

void WorksheetDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "Worksheet" );

	// Geometry
	ui.chScaleContent->setChecked(group.readEntry("ScaleContent", false));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width", m_worksheet->pageRect().width()), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height", m_worksheet->pageRect().height()), Worksheet::Centimeter));
	if (group.readEntry("UseViewSize", false))
		ui.cbSize->setCurrentIndex(0);
	else
		updatePaperSize();

	// Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_worksheet->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_worksheet->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_worksheet->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_worksheet->backgroundBrushStyle()) );
	ui.leBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_worksheet->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_worksheet->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_worksheet->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( qRound(group.readEntry("BackgroundOpacity", m_worksheet->backgroundOpacity())*100) );

	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry("LayoutTopMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutBottomMargin->setValue(group.readEntry("LayoutBottomMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutLeftMargin->setValue(group.readEntry("LayoutLeftMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutRightMargin->setValue(group.readEntry("LayoutRightMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutHorizontalSpacing->setValue(group.readEntry("LayoutHorizontalSpacing", Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), Worksheet::Centimeter)) );
	ui.sbLayoutVerticalSpacing->setValue(group.readEntry("LayoutVerticalSpacing", Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), Worksheet::Centimeter)) );

	ui.sbLayoutRowCount->setValue(group.readEntry("LayoutRowCount", m_worksheet->layoutRowCount()));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_worksheet->layoutColumnCount()));
}

void WorksheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Worksheet" );

	//General
	group.writeEntry("ScaleContent",ui.chScaleContent->isChecked());
	group.writeEntry("UseViewSize",ui.cbSize->currentIndex() == 0);
	group.writeEntry("Width",Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter));
	group.writeEntry("Height",Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter));

	//Background
	group.writeEntry("BackgroundType",ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);

	//Layout
	group.writeEntry("LayoutTopMargin",Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutBottomMargin",Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutLeftMargin",Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRightMargin",Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutVerticalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutHorizontalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRowCount", ui.sbLayoutRowCount->value());
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	config.sync();
}

void WorksheetDock::loadTheme(const QString& theme) {
	for (auto* worksheet : m_worksheetList)
		worksheet->setTheme(theme);
}
