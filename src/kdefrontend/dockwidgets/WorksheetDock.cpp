/*
    File                 : WorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for worksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetDock.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/TemplateHandler.h"

#include <QCompleter>
#include <QDirModel>
#include <QPageSize>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class WorksheetDock
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget *parent): BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//Background-tab
	ui.cbBackgroundColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.leBackgroundFileName->setCompleter(new QCompleter(new QDirModel, this));

	//Layout-tab
	ui.chScaleContent->setToolTip(i18n("If checked, rescale the content of the worksheet on size changes. Otherwise resize the canvas only."));

	ui.cbLayout->addItem(QIcon::fromTheme("labplot-editbreaklayout"), i18n("No Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme("labplot-edithlayout"), i18n("Vertical Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme("labplot-editvlayout"), i18n("Horizontal Layout"));
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

	WorksheetDock::updateLocale();

	//SLOTs
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &WorksheetDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &WorksheetDock::commentChanged);
	connect(ui.cbSizeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::sizeTypeChanged);
	connect(ui.cbPage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::pageChanged);
	connect(ui.sbWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &WorksheetDock::sizeChanged);
	connect(ui.sbHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &WorksheetDock::sizeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::orientationChanged);

	//Background
	connect(ui.cbBackgroundType, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundTypeChanged);
	connect(ui.cbBackgroundColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundColorStyleChanged);
	connect(ui.cbBackgroundImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundImageStyleChanged);
	connect(ui.cbBackgroundBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::backgroundBrushStyleChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &WorksheetDock::selectFile);
	connect(ui.leBackgroundFileName, &QLineEdit::returnPressed, this, &WorksheetDock::fileNameChanged);
	connect(ui.leBackgroundFileName, &QLineEdit::textChanged, this, &WorksheetDock::fileNameChanged);
	connect(ui.kcbBackgroundFirstColor, &KColorButton::changed, this, &WorksheetDock::backgroundFirstColorChanged);
	connect(ui.kcbBackgroundSecondColor, &KColorButton::changed, this, &WorksheetDock::backgroundSecondColorChanged);
	connect(ui.sbBackgroundOpacity, QOverload<int>::of(&QSpinBox::valueChanged),
			this, &WorksheetDock::backgroundOpacityChanged);

	//Layout
	connect(ui.cbLayout, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &WorksheetDock::layoutChanged);
	connect( ui.chScaleContent, &QCheckBox::clicked, this, &WorksheetDock::scaleContentChanged);
	connect( ui.sbLayoutTopMargin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutTopMarginChanged);
	connect( ui.sbLayoutBottomMargin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutBottomMarginChanged);
	connect( ui.sbLayoutLeftMargin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutLeftMarginChanged);
	connect( ui.sbLayoutRightMargin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutRightMarginChanged);
	connect( ui.sbLayoutHorizontalSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutHorizontalSpacingChanged);
	connect( ui.sbLayoutVerticalSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			 this, &WorksheetDock::layoutVerticalSpacingChanged);
	connect( ui.sbLayoutRowCount, QOverload<int>::of(&QSpinBox::valueChanged),
			 this, &WorksheetDock::layoutRowCountChanged);
	connect( ui.sbLayoutColumnCount, QOverload<int>::of(&QSpinBox::valueChanged),
			 this, &WorksheetDock::layoutColumnCountChanged);

	//theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, &ThemeHandler::loadThemeRequested, this, &WorksheetDock::loadTheme);
	connect(m_themeHandler, &ThemeHandler::info, this, &WorksheetDock::info);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
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
	m_aspect = list.first();

	//if there are more then one worksheet in the list, disable the name and comment field in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_worksheet->name());
		ui.teComment->setText(m_worksheet->comment());
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

	//show the properties of the first worksheet
	this->load();
	this->worksheetLayoutChanged(m_worksheet->layout());

	m_themeHandler->setCurrentTheme(m_worksheet->theme());

	connect(m_worksheet, &Worksheet::aspectDescriptionChanged, this, &WorksheetDock::worksheetDescriptionChanged);
	connect(m_worksheet, &Worksheet::pageRectChanged, this, &WorksheetDock::worksheetPageRectChanged);
	connect(m_worksheet, &Worksheet::scaleContentChanged, this, &WorksheetDock::worksheetScaleContentChanged);
	connect(m_worksheet, &Worksheet::useViewSizeChanged, this, &WorksheetDock::worksheetUseViewSizeChanged);

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

void WorksheetDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbLayoutTopMargin->setLocale(numberLocale);
	ui.sbLayoutBottomMargin->setLocale(numberLocale);
	ui.sbLayoutLeftMargin->setLocale(numberLocale);
	ui.sbLayoutRightMargin->setLocale(numberLocale);
	ui.sbLayoutHorizontalSpacing->setLocale(numberLocale);
	ui.sbLayoutVerticalSpacing->setLocale(numberLocale);
}

void WorksheetDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	Lock lock(m_initializing);
	QString suffix;
	if (m_units == Units::Metric) {
		//convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbWidth->setValue(ui.sbWidth->value()*2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()*2.54);
		ui.sbLayoutTopMargin->setValue(ui.sbLayoutTopMargin->value()*2.54);
		ui.sbLayoutBottomMargin->setValue(ui.sbLayoutBottomMargin->value()*2.54);
		ui.sbLayoutLeftMargin->setValue(ui.sbLayoutLeftMargin->value()*2.54);
		ui.sbLayoutRightMargin->setValue(ui.sbLayoutRightMargin->value()*2.54);
		ui.sbLayoutHorizontalSpacing->setValue(ui.sbLayoutHorizontalSpacing->value()*2.54);
		ui.sbLayoutVerticalSpacing->setValue(ui.sbLayoutVerticalSpacing->value()*2.54);
	} else {
		//convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbWidth->setValue(ui.sbWidth->value()/2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()/2.54);
		ui.sbLayoutTopMargin->setValue(ui.sbLayoutTopMargin->value()/2.54);
		ui.sbLayoutBottomMargin->setValue(ui.sbLayoutBottomMargin->value()/2.54);
		ui.sbLayoutLeftMargin->setValue(ui.sbLayoutLeftMargin->value()/2.54);
		ui.sbLayoutRightMargin->setValue(ui.sbLayoutRightMargin->value()/2.54);
		ui.sbLayoutHorizontalSpacing->setValue(ui.sbLayoutHorizontalSpacing->value()/2.54);
		ui.sbLayoutVerticalSpacing->setValue(ui.sbLayoutVerticalSpacing->value()/2.54);
	}

	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);
}

/*!
	Checks whether the size is one of the QPageSize::PageSizeId and
	updates Size and Orientation checkbox when width/height changes.
*/
void WorksheetDock::updatePaperSize() {
	if (m_worksheet->useViewSize()) {
		ui.cbSizeType->setCurrentIndex(0);
		return;
	}

	double w = ui.sbWidth->value();
	double h = ui.sbHeight->value();
	if (m_units == Units::Metric) {
		//In UI we use cm, so we need to convert to mm first before we check with QPageSize
		w *= 10;
		h *= 10;
	}

	const QSizeF s = QSizeF(w, h);
	const QSizeF st = s.transposed();

	//determine the position of the QPageSize::PageSizeId in the combobox
	bool found = false;
	for (int i = 0; i < ui.cbPage->count(); ++i) {
		const QVariant v = ui.cbPage->itemData(i);
		if (!v.isValid())
			continue;

		const auto id = v.value<QPageSize::PageSizeId>();
		QPageSize::Unit pageUnit = (m_units == Units::Metric) ? QPageSize::Millimeter : QPageSize::Inch;
		const QSizeF ps = QPageSize::size(id, pageUnit);
		if (s == ps) { //check the portrait-orientation first
			ui.cbPage->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(0);  //a QPageSize::PaperSize in portrait-orientation was found
			found = true;
			break;
		} else if (st == ps) { //check for the landscape-orientation
			ui.cbPage->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(1); //a QPageSize::PaperSize in landscape-orientation was found
			found = true;
			break;
		}
	}

	if (!found)
		ui.cbSizeType->setCurrentIndex(2); //select "Custom"
}

//*************************************************************
//****** SLOTs for changes triggered in WorksheetDock *********
//*************************************************************
void WorksheetDock::retranslateUi() {
	Lock lock(m_initializing);

	//Geometry
	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Portrait"));
	ui.cbOrientation->addItem(i18n("Landscape"));

	QString suffix;
	if (m_units == Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbLayoutTopMargin->setSuffix(suffix);
	ui.sbLayoutBottomMargin->setSuffix(suffix);
	ui.sbLayoutLeftMargin->setSuffix(suffix);
	ui.sbLayoutRightMargin->setSuffix(suffix);
	ui.sbLayoutHorizontalSpacing->setSuffix(suffix);
	ui.sbLayoutVerticalSpacing->setSuffix(suffix);

	ui.cbSizeType->clear();
	ui.cbSizeType->addItem(i18n("View Size"));
	ui.cbSizeType->addItem(i18n("Standard Page"));
	ui.cbSizeType->addItem(i18n("Custom"));

	const QVector<QPageSize::PageSizeId> pageSizeIds = {
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
		QPageSize::Comm10E
	};
	ui.cbPage->clear();
	for (auto id : pageSizeIds)
		ui.cbPage->addItem(QPageSize::name(id), id);

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
}

// "General"-tab
void WorksheetDock::scaleContentChanged(bool scaled) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setScaleContent(scaled);
}

void WorksheetDock::sizeTypeChanged(int index) {
	if (index == 0) { //view size
		ui.lPage->hide();
		ui.cbPage->hide();
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
	} else if (index == 1) { //standard page
		ui.lPage->show();
		ui.cbPage->show();
		ui.lOrientation->show();
		ui.cbOrientation->show();
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
	} else { //custom size
		ui.lPage->hide();
		ui.cbPage->hide();
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
	}

	if (m_initializing)
		return;

	if (index == 0) { //viewSize
		Lock lock(m_initializing);
		for (auto* worksheet : m_worksheetList)
			worksheet->setUseViewSize(true);
	} else if (index == 1) { //standard page
		pageChanged(ui.cbPage->currentIndex());
	} else { //custom size
		if (m_worksheet->useViewSize()) {
			for (auto* worksheet : m_worksheetList)
				worksheet->setUseViewSize(false);
		}
		sizeChanged();
	}
}

void WorksheetDock::pageChanged(int i) {
	if (m_initializing)
		return;

	Lock lock(m_initializing);

	//determine the width and the height of the to be used predefined layout
	const auto index = ui.cbPage->itemData(i).value<QPageSize::PageSizeId>();
	QSizeF s = QPageSize::size(index, QPageSize::Millimeter);
	if (ui.cbOrientation->currentIndex() == 1)
		s.transpose();

	//s is in mm, in UI we show everything in cm/in
	if (m_units == Units::Metric) {
		ui.sbWidth->setValue(s.width()/10);
		ui.sbHeight->setValue(s.height()/10);
	} else {
		ui.sbWidth->setValue(s.width()/25.4);
		ui.sbHeight->setValue(s.height()/25.4);
	}

	double w = Worksheet::convertToSceneUnits(s.width(), Worksheet::Unit::Millimeter);
	double h = Worksheet::convertToSceneUnits(s.height(), Worksheet::Unit::Millimeter);
	for (auto* worksheet : m_worksheetList) {
		worksheet->setUseViewSize(false);
		worksheet->setPageRect(QRectF(0,0,w,h));
	}
}

void WorksheetDock::sizeChanged() {
	if (m_initializing)
		return;

	double w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), m_worksheetUnit);
	double h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), m_worksheetUnit);
	for (auto* worksheet : m_worksheetList)
		worksheet->setPageRect(QRectF(0,0,w,h));
}

void WorksheetDock::orientationChanged(int /*index*/) {
	if (m_initializing)
		return;

	this->pageChanged(ui.cbPage->currentIndex());
}

// "Background"-tab
void WorksheetDock::backgroundTypeChanged(int index) {
	if (index == -1)
		return;

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
	} else if (type == WorksheetElement::BackgroundType::Pattern) {
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
	if (index == -1)
		return;

	auto style = (WorksheetElement::BackgroundColorStyle)index;

	if (style == WorksheetElement::BackgroundColorStyle::SingleColor) {
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

	auto style = (WorksheetElement::BackgroundImageStyle)index;
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
	auto layout = (Worksheet::Layout)index;

	bool b = (layout != Worksheet::Layout::NoLayout);
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
		bool grid = (layout == Worksheet::Layout::GridLayout);
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
		worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void WorksheetDock::layoutBottomMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void WorksheetDock::layoutLeftMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void WorksheetDock::layoutRightMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, m_worksheetUnit));
}

void WorksheetDock::layoutHorizontalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
}

void WorksheetDock::layoutVerticalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, m_worksheetUnit));
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
	const QString& path = GuiTools::openImageFile(QLatin1String("WorksheetDock"));
	if (path.isEmpty())
		return;

	ui.leBackgroundFileName->setText(path);
}

void WorksheetDock::fileNameChanged() {
	if (m_initializing)
		return;

	const QString& fileName = ui.leBackgroundFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

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
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());
	m_initializing = false;
}

void WorksheetDock::worksheetScaleContentChanged(bool scaled) {
	m_initializing = true;
	ui.chScaleContent->setChecked(scaled);
	m_initializing = false;
}
void WorksheetDock::worksheetUseViewSizeChanged(bool useViewSize) {
	m_initializing = true;
	if (useViewSize)
		ui.cbSizeType->setCurrentIndex(0);
	else
		updatePaperSize();
	m_initializing = false;
}
void WorksheetDock::worksheetPageRectChanged(const QRectF& rect) {
	m_initializing = true;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), m_worksheetUnit));
	updatePaperSize();
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundTypeChanged(WorksheetElement::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(static_cast<int>(style));
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
	ui.cbLayout->setCurrentIndex(static_cast<int>(layout));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutTopMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutBottomMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutLeftMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutRightMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutVerticalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutHorizontalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
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
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().width(), m_worksheetUnit) );
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().height(), m_worksheetUnit) );
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
	const QString& fileName = m_worksheet->backgroundFileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

	// Layout
	ui.cbLayout->setCurrentIndex( (int) m_worksheet->layout() );
	ui.sbLayoutTopMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin(), m_worksheetUnit) );
	ui.sbLayoutBottomMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), m_worksheetUnit) );
	ui.sbLayoutLeftMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), m_worksheetUnit) );
	ui.sbLayoutRightMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), m_worksheetUnit) );
	ui.sbLayoutHorizontalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), m_worksheetUnit) );
	ui.sbLayoutVerticalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), m_worksheetUnit) );

	ui.sbLayoutRowCount->setValue( m_worksheet->layoutRowCount() );
	ui.sbLayoutColumnCount->setValue( m_worksheet->layoutColumnCount() );
}

void WorksheetDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
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
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width", m_worksheet->pageRect().width()), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height", m_worksheet->pageRect().height()), m_worksheetUnit));
	if (group.readEntry("UseViewSize", false))
		ui.cbSizeType->setCurrentIndex(0);
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
	ui.cbLayout->setCurrentIndex( group.readEntry("Layout", (int)m_worksheet->layout()) );
	ui.sbLayoutTopMargin->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutTopMargin", m_worksheet->layoutTopMargin()), m_worksheetUnit) );
	ui.sbLayoutBottomMargin->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutBottomMargin", m_worksheet->layoutBottomMargin()), m_worksheetUnit) );
	ui.sbLayoutLeftMargin->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutLeftMargin", m_worksheet->layoutLeftMargin()), m_worksheetUnit) );
	ui.sbLayoutRightMargin->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutRightMargin", m_worksheet->layoutRightMargin()), m_worksheetUnit) );
	ui.sbLayoutHorizontalSpacing->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutHorizontalSpacing", m_worksheet->layoutHorizontalSpacing()), m_worksheetUnit) );
	ui.sbLayoutVerticalSpacing->setValue( Worksheet::convertFromSceneUnits(group.readEntry("LayoutVerticalSpacing", m_worksheet->layoutVerticalSpacing()), m_worksheetUnit) );

	ui.sbLayoutRowCount->setValue(group.readEntry("LayoutRowCount", m_worksheet->layoutRowCount()));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_worksheet->layoutColumnCount()));
}

void WorksheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Worksheet" );

	//General
	group.writeEntry("ScaleContent",ui.chScaleContent->isChecked());
	group.writeEntry("UseViewSize", ui.cbSizeType->currentIndex() == 0);
	group.writeEntry("Width",Worksheet::convertToSceneUnits(ui.sbWidth->value(), m_worksheetUnit));
	group.writeEntry("Height",Worksheet::convertToSceneUnits(ui.sbHeight->value(), m_worksheetUnit));

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
	group.writeEntry("Layout", ui.cbLayout->currentIndex());
	group.writeEntry("LayoutTopMargin",Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutBottomMargin",Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutLeftMargin",Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutRightMargin",Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), m_worksheetUnit));
	group.writeEntry("LayoutVerticalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), m_worksheetUnit));
	group.writeEntry("LayoutHorizontalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), m_worksheetUnit));
	group.writeEntry("LayoutRowCount", ui.sbLayoutRowCount->value());
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	config.sync();
}

void WorksheetDock::loadTheme(const QString& theme) {
	for (auto* worksheet : m_worksheetList)
		worksheet->setTheme(theme);
}
