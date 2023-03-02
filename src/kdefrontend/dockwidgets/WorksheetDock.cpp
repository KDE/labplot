/*
	File                 : WorksheetDock.cpp
	Project              : LabPlot
	Description          : widget for worksheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetDock.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"

#include <QPageSize>

#include <KConfig>
#include <KLocalizedString>

/*!
  \class WorksheetDock
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	// Background-tab
	auto* layout = static_cast<QHBoxLayout*>(ui.tabBackground->layout());
	backgroundWidget = new BackgroundWidget(ui.tabBackground);
	layout->insertWidget(0, backgroundWidget);

	// Layout-tab
	ui.chScaleContent->setToolTip(i18n("If checked, rescale the content of the worksheet on size changes. Otherwise resize the canvas only."));

	ui.cbLayout->addItem(QIcon::fromTheme(QStringLiteral("labplot-editbreaklayout")), i18n("No Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme(QStringLiteral("labplot-edithlayout")), i18n("Vertical Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme(QStringLiteral("labplot-editvlayout")), i18n("Horizontal Layout"));
	ui.cbLayout->addItem(QIcon::fromTheme(QStringLiteral("labplot-editgrid")), i18n("Grid Layout"));

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	WorksheetDock::updateLocale();

	// SLOTs
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &WorksheetDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &WorksheetDock::commentChanged);
	connect(ui.cbSizeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::sizeTypeChanged);
	connect(ui.cbPage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::pageChanged);
	connect(ui.sbWidth, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &WorksheetDock::sizeChanged);
	connect(ui.sbHeight, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &WorksheetDock::sizeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::orientationChanged);

	// Layout
	connect(ui.cbLayout, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WorksheetDock::layoutChanged);
	connect(ui.chScaleContent, &QCheckBox::clicked, this, &WorksheetDock::scaleContentChanged);
	connect(ui.sbLayoutTopMargin, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &WorksheetDock::layoutTopMarginChanged);
	connect(ui.sbLayoutBottomMargin,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&WorksheetDock::layoutBottomMarginChanged);
	connect(ui.sbLayoutLeftMargin, QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged), this, &WorksheetDock::layoutLeftMarginChanged);
	connect(ui.sbLayoutRightMargin,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&WorksheetDock::layoutRightMarginChanged);
	connect(ui.sbLayoutHorizontalSpacing,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&WorksheetDock::layoutHorizontalSpacingChanged);
	connect(ui.sbLayoutVerticalSpacing,
			QOverload<const Common::ExpressionValue&>::of(&NumberSpinBox::valueChanged),
			this,
			&WorksheetDock::layoutVerticalSpacingChanged);
	connect(ui.sbLayoutRowCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &WorksheetDock::layoutRowCountChanged);
	connect(ui.sbLayoutColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &WorksheetDock::layoutColumnCountChanged);

	// theme and template handlers
	auto* frame = new QFrame(this);
	layout = new QHBoxLayout(frame);
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
	setAspects(list);

	// if there are more then one worksheet in the list, disable the name and comment field in the tab "general"
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
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first worksheet
	this->load();
	this->worksheetLayoutChanged(m_worksheet->layout());

	m_themeHandler->setCurrentTheme(m_worksheet->theme());

	connect(m_worksheet, &Worksheet::aspectDescriptionChanged, this, &WorksheetDock::worksheetDescriptionChanged);
	connect(m_worksheet, &Worksheet::pageRectChanged, this, &WorksheetDock::worksheetPageRectChanged);
	connect(m_worksheet, &Worksheet::scaleContentChanged, this, &WorksheetDock::worksheetScaleContentChanged);
	connect(m_worksheet, &Worksheet::useViewSizeChanged, this, &WorksheetDock::worksheetUseViewSizeChanged);

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
	const auto numberLocale = QLocale();
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
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	if (m_units == Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbWidth->setScaling(2.54);
		ui.sbHeight->setScaling(2.54);
		ui.sbLayoutTopMargin->setScaling(2.54);
		ui.sbLayoutBottomMargin->setScaling(2.54);
		ui.sbLayoutLeftMargin->setScaling(2.54);
		ui.sbLayoutRightMargin->setScaling(2.54);
		ui.sbLayoutHorizontalSpacing->setScaling(2.54);
		ui.sbLayoutVerticalSpacing->setScaling(2.54);
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbWidth->setScaling(1 / 2.54);
		ui.sbHeight->setScaling(1 / 2.54);
		ui.sbLayoutTopMargin->setScaling(1 / 2.54);
		ui.sbLayoutBottomMargin->setScaling(1 / 2.54);
		ui.sbLayoutLeftMargin->setScaling(1 / 2.54);
		ui.sbLayoutRightMargin->setScaling(1 / 2.54);
		ui.sbLayoutHorizontalSpacing->setScaling(1 / 2.54);
		ui.sbLayoutVerticalSpacing->setScaling(1 / 2.54);
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
		ui.cbSizeType->setCurrentIndex(ui.cbSizeType->findData((int)SizeType::ViewSize));
		return;
	}

	double w = ui.sbWidth->value().value<double>();
	double h = ui.sbHeight->value().value<double>();
	if (m_units == Units::Metric) {
		// In UI we use cm, so we need to convert to mm first before we check with QPageSize
		w *= 10;
		h *= 10;
	}

	const QSizeF s = QSizeF(w, h);
	const QSizeF st = s.transposed();

	// determine the position of the QPageSize::PageSizeId in the combobox
	bool found = false;
	for (int i = 0; i < ui.cbPage->count(); ++i) {
		const QVariant v = ui.cbPage->itemData(i);
		if (!v.isValid())
			continue;

		const auto id = v.value<QPageSize::PageSizeId>();
		QPageSize::Unit pageUnit = (m_units == Units::Metric) ? QPageSize::Millimeter : QPageSize::Inch;
		const QSizeF ps = QPageSize::size(id, pageUnit);
		if (s == ps) { // check the portrait-orientation first
			ui.cbPage->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(0); // a QPageSize::PaperSize in portrait-orientation was found
			found = true;
			break;
		} else if (st == ps) { // check for the landscape-orientation
			ui.cbPage->setCurrentIndex(i);
			ui.cbOrientation->setCurrentIndex(1); // a QPageSize::PaperSize in landscape-orientation was found
			found = true;
			break;
		}
	}

	if (!found)
		ui.cbSizeType->setCurrentIndex(ui.cbSizeType->findData((int)SizeType::Custom));
	else
		ui.cbSizeType->setCurrentIndex(ui.cbSizeType->findData((int)SizeType::StandardPage));
}

//*************************************************************
//****** SLOTs for changes triggered in WorksheetDock *********
//*************************************************************
void WorksheetDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// Geometry
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
	ui.cbSizeType->addItem(i18n("View Size"), (int)SizeType::ViewSize);
	ui.cbSizeType->addItem(i18n("Standard Page"), (int)SizeType::StandardPage);
	ui.cbSizeType->addItem(i18n("Custom"), (int)SizeType::Custom);

	const QVector<QPageSize::PageSizeId> pageSizeIds = {
		QPageSize::A0,	  QPageSize::A1,	 QPageSize::A2,	   QPageSize::A3,	  QPageSize::A4,	  QPageSize::A5,	 QPageSize::A6,	 QPageSize::A7,
		QPageSize::A8,	  QPageSize::A9,	 QPageSize::B0,	   QPageSize::B1,	  QPageSize::B2,	  QPageSize::B3,	 QPageSize::B4,	 QPageSize::B5,
		QPageSize::B6,	  QPageSize::B7,	 QPageSize::B8,	   QPageSize::B9,	  QPageSize::B10,	  QPageSize::C5E,	 QPageSize::DLE, QPageSize::Executive,
		QPageSize::Folio, QPageSize::Ledger, QPageSize::Legal, QPageSize::Letter, QPageSize::Tabloid, QPageSize::Comm10E};
	ui.cbPage->clear();
	for (auto id : pageSizeIds)
		ui.cbPage->addItem(QPageSize::name(id), id);
}

// "General"-tab
void WorksheetDock::scaleContentChanged(bool scaled) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* worksheet : m_worksheetList)
		worksheet->setScaleContent(scaled);
}

void WorksheetDock::sizeTypeChanged(int index) {
	const auto sizeType = static_cast<SizeType>(ui.cbSizeType->itemData(index).toInt());

	switch (sizeType) {
	case SizeType::ViewSize:
		ui.lPage->hide();
		ui.cbPage->hide();
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
		break;
	case SizeType::StandardPage:
		ui.lPage->show();
		ui.cbPage->show();
		ui.lOrientation->show();
		ui.cbOrientation->show();
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
		break;
	case SizeType::Custom:
		ui.lPage->hide();
		ui.cbPage->hide();
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
		break;
	}

	if (m_initializing) // don't lock here since we potentially need to call setters in pageChanged() below
		return;

	if (index == 0) { // viewSize
		for (auto* worksheet : m_worksheetList)
			worksheet->setUseViewSize(true);
	} else if (index == 1) { // standard page
		pageChanged(ui.cbPage->currentIndex());
	} else { // custom size
		if (m_worksheet->useViewSize()) {
			for (auto* worksheet : m_worksheetList)
				worksheet->setUseViewSize(false);
		}
		sizeChanged();
	}
}

void WorksheetDock::pageChanged(int i) {
	// determine the width and the height of the to be used predefined layout
	const auto index = ui.cbPage->itemData(i).value<QPageSize::PageSizeId>();
	QSizeF s = QPageSize::size(index, QPageSize::Millimeter);
	if (ui.cbOrientation->currentIndex() == 1)
		s.transpose();

	// s is in mm, in UI we show everything in cm/in
	if (m_units == Units::Metric) {
		ui.sbWidth->setValue(s.width() / 10);
		ui.sbHeight->setValue(s.height() / 10);
	} else {
		ui.sbWidth->setValue(s.width() / 25.4);
		ui.sbHeight->setValue(s.height() / 25.4);
	}

	CONDITIONAL_LOCK_RETURN;

	double w = Worksheet::convertToSceneUnits(s.width(), Worksheet::Unit::Millimeter);
	double h = Worksheet::convertToSceneUnits(s.height(), Worksheet::Unit::Millimeter);
	for (auto* worksheet : m_worksheetList) {
		worksheet->setUseViewSize(false);
		worksheet->setPageRect(QRectF(0, 0, w, h));
	}
}

void WorksheetDock::sizeChanged() {
	CONDITIONAL_RETURN_NO_LOCK;

	double w = Worksheet::convertToSceneUnits(ui.sbWidth->value().value<double>(), m_worksheetUnit);
	double h = Worksheet::convertToSceneUnits(ui.sbHeight->value().value<double>(), m_worksheetUnit);
	for (auto* worksheet : m_worksheetList)
		worksheet->setPageRect(QRectF(0, 0, w, h));
}

void WorksheetDock::orientationChanged(int /*index*/) {
	CONDITIONAL_LOCK_RETURN;

	this->pageChanged(ui.cbPage->currentIndex());
}

//"Layout"-tab
void WorksheetDock::layoutChanged(int index) {
	auto layout = static_cast<Worksheet::Layout>(index);

	bool b = (layout != Worksheet::Layout::NoLayout);
	ui.sbLayoutTopMargin->setEnabled(b);
	ui.sbLayoutBottomMargin->setEnabled(b);
	ui.sbLayoutLeftMargin->setEnabled(b);
	ui.sbLayoutRightMargin->setEnabled(b);
	ui.sbLayoutHorizontalSpacing->setEnabled(b);
	ui.sbLayoutVerticalSpacing->setEnabled(b);
	ui.sbLayoutRowCount->setEnabled(b);
	ui.sbLayoutColumnCount->setEnabled(b);

	// show the "scale content" option if no layout active
	ui.lScaleContent->setVisible(!b);
	ui.chScaleContent->setVisible(!b);

	if (b) {
		// show grid specific settings if grid layout selected
		bool grid = (layout == Worksheet::Layout::GridLayout);
		ui.lGrid->setVisible(grid);
		ui.lRowCount->setVisible(grid);
		ui.sbLayoutRowCount->setVisible(grid);
		ui.lColumnCount->setVisible(grid);
		ui.sbLayoutColumnCount->setVisible(grid);
	} else {
		// no layout selected, hide grid specific settings that were potentially shown before
		ui.lGrid->setVisible(false);
		ui.lRowCount->setVisible(false);
		ui.sbLayoutRowCount->setVisible(false);
		ui.lColumnCount->setVisible(false);
		ui.sbLayoutColumnCount->setVisible(false);
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayout(layout);
}

void WorksheetDock::layoutTopMarginChanged(const Common::ExpressionValue& margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
        worksheet->setLayoutTopMargin(margin);
}

void WorksheetDock::layoutBottomMarginChanged(Common::ExpressionValue margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin.value<double>(), m_worksheetUnit));
}

void WorksheetDock::layoutLeftMarginChanged(Common::ExpressionValue margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin.value<double>(), m_worksheetUnit));
}

void WorksheetDock::layoutRightMarginChanged(Common::ExpressionValue margin) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin.value<double>(), m_worksheetUnit));
}

void WorksheetDock::layoutHorizontalSpacingChanged(Common::ExpressionValue spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing.value<double>(), m_worksheetUnit));
}

void WorksheetDock::layoutVerticalSpacingChanged(Common::ExpressionValue spacing) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing.value<double>(), m_worksheetUnit));
}

void WorksheetDock::layoutRowCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRowCount(count);
}

void WorksheetDock::layoutColumnCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutColumnCount(count);
}

//*************************************************************
//******** SLOTs for changes triggered in Worksheet ***********
//*************************************************************
void WorksheetDock::worksheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_worksheet != aspect)
		return;

	CONDITIONAL_LOCK_RETURN;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());
}

void WorksheetDock::worksheetScaleContentChanged(bool scaled) {
	CONDITIONAL_LOCK_RETURN;
	ui.chScaleContent->setChecked(scaled);
}
void WorksheetDock::worksheetUseViewSizeChanged(bool useViewSize) {
	CONDITIONAL_LOCK_RETURN;
	if (useViewSize)
		ui.cbSizeType->setCurrentIndex(0);
	else
		updatePaperSize();
}
void WorksheetDock::worksheetPageRectChanged(const QRectF& rect) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), m_worksheetUnit));
	updatePaperSize();
}

void WorksheetDock::worksheetLayoutChanged(Worksheet::Layout layout) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLayout->setCurrentIndex(static_cast<int>(layout));
}

void WorksheetDock::worksheetLayoutTopMarginChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutBottomMarginChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutLeftMarginChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutRightMarginChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutVerticalSpacingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutHorizontalSpacingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
}

void WorksheetDock::worksheetLayoutRowCountChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutRowCount->setValue(value);
}

void WorksheetDock::worksheetLayoutColumnCountChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLayoutColumnCount->setValue(value);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void WorksheetDock::load() {
	// Geometry
	ui.chScaleContent->setChecked(m_worksheet->scaleContent());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_worksheet->pageRect().width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_worksheet->pageRect().height(), m_worksheetUnit));
	updatePaperSize();

	// Background
	QList<Background*> backgrounds;
	for (auto* worksheet : m_worksheetList)
		backgrounds << worksheet->background();

	backgroundWidget->setBackgrounds(backgrounds);

	// Layout
	ui.cbLayout->setCurrentIndex((int)m_worksheet->layout());
    ui.sbLayoutTopMargin->setValue(Common::ExpressionValue(m_worksheet->layoutTopMargin(), Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin().value<double>(), m_worksheetUnit)));
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), m_worksheetUnit));
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), m_worksheetUnit));
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), m_worksheetUnit));
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), m_worksheetUnit));
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), m_worksheetUnit));

	ui.sbLayoutRowCount->setValue(m_worksheet->layoutRowCount());
	ui.sbLayoutColumnCount->setValue(m_worksheet->layoutColumnCount());
}

void WorksheetDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
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
	KConfigGroup group = config.group("Worksheet");

	// Geometry
	ui.chScaleContent->setChecked(group.readEntry("ScaleContent", false));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width", m_worksheet->pageRect().width()), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height", m_worksheet->pageRect().height()), m_worksheetUnit));
	if (group.readEntry("UseViewSize", false))
		ui.cbSizeType->setCurrentIndex(0);
	else
		updatePaperSize();

	// Background
	backgroundWidget->loadConfig(group);

	// Layout
    ui.cbLayout->setCurrentIndex(group.readEntry("Layout", (int)m_worksheet->layout()));
    ui.sbLayoutTopMargin->setValue(Common::ExpressionValue::loadFromConfig(group, QStringLiteral("LayoutTopMargin"), m_worksheet->layoutTopMargin()));
	ui.sbLayoutBottomMargin->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry("LayoutBottomMargin", m_worksheet->layoutBottomMargin()), m_worksheetUnit));
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(group.readEntry("LayoutLeftMargin", m_worksheet->layoutLeftMargin()), m_worksheetUnit));
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(group.readEntry("LayoutRightMargin", m_worksheet->layoutRightMargin()), m_worksheetUnit));
	ui.sbLayoutHorizontalSpacing->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry("LayoutHorizontalSpacing", m_worksheet->layoutHorizontalSpacing()), m_worksheetUnit));
	ui.sbLayoutVerticalSpacing->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry("LayoutVerticalSpacing", m_worksheet->layoutVerticalSpacing()), m_worksheetUnit));

	ui.sbLayoutRowCount->setValue(group.readEntry("LayoutRowCount", m_worksheet->layoutRowCount()));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_worksheet->layoutColumnCount()));
}

void WorksheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Worksheet");

	// General
	group.writeEntry("ScaleContent", ui.chScaleContent->isChecked());
	group.writeEntry("UseViewSize", ui.cbSizeType->currentIndex() == 0);
	group.writeEntry("Width", Worksheet::convertToSceneUnits(ui.sbWidth->value().value<double>(), m_worksheetUnit));
	group.writeEntry("Height", Worksheet::convertToSceneUnits(ui.sbHeight->value().value<double>(), m_worksheetUnit));

	// Background
	backgroundWidget->saveConfig(group);

	// Layout
	group.writeEntry("Layout", ui.cbLayout->currentIndex());
	group.writeEntry("LayoutTopMargin", Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutBottomMargin", Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutLeftMargin", Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutRightMargin", Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutVerticalSpacing", Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutHorizontalSpacing", Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value().value<double>(), m_worksheetUnit));
	group.writeEntry("LayoutRowCount", ui.sbLayoutRowCount->value());
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	config.sync();
}

void WorksheetDock::loadTheme(const QString& theme) {
	for (auto* worksheet : m_worksheetList)
		worksheet->setTheme(theme);
}
