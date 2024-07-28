/*
	File                 : bar3DPlotAreaDock.cpp
	Project              : LabPlot
	Description          : widget for bar3DPlotArea properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Bar3DPlotAreaDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/matrix/Matrix.h"
#include <backend/core/AspectTreeModel.h>
// #include <kdefrontend/TemplateHandler.h>

Bar3DPlotAreaDock::Bar3DPlotAreaDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	// data-columns
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	m_gridLayout = new QGridLayout(ui.frameDataColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameDataColumns->setLayout(m_gridLayout);

	// SIGNALs/SLOTs
	// General
	connect(m_buttonNew, &QPushButton::clicked, this, &Bar3DPlotDock::addDataColumn);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &bar3DPlotAreaDock::themeChanged);
	connect(ui.cbShadowQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &bar3DPlotAreaDock::shadowQualityChanged);
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Bar3DPlotAreaDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Bar3DPlotAreaDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Bar3DPlotAreaDock::zoomLevelChanged);
	// connect(ui.kcbColor)
}

void Bar3DPlotAreaDock::setBars(const QList<Bar3DPlotArea*>& bars) {
	CONDITIONAL_LOCK_RETURN;
	m_bars = bars;
	m_bar = m_bars.first();
	setAspects(bars);
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});

	// show the properties of the first bar
	// tab "General"

	barThemeChanged(m_bar->theme());
	barShadowQualityChanged(m_bar->shadowQuality());
	barZoomLevelChanged(m_bar->zoomLevel());
	barXRotationChanged(m_bar->xRotation());
	barYRotationChanged(m_bar->yRotation());

	connect(m_bar, &Bar3DPlotArea::themeChanged, this, &Bar3DPlotAreaDock::barThemeChanged);
	connect(m_bar, &Bar3DPlotArea::shadowQualityChanged, this, &Bar3DPlotAreaDock::barShadowQualityChanged);
	connect(m_bar, &Bar3DPlotArea::zoomLevelChanged, this, &Bar3DPlotAreaDock::barZoomLevelChanged);
	connect(m_bar, &Bar3DPlotArea::xRotationChanged, this, &Bar3DPlotAreaDock::barXRotationChanged);
	connect(m_bar, &Bar3DPlotArea::yRotationChanged, this, &Bar3DPlotAreaDock::barYRotationChanged);
}

void Bar3DPlotAreaDock::retranslateUi() {
	// This function should contain translation code if needed
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Bar3DPlotArea::SoftHigh, i18n("Soft High"));

	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::Qt, i18n("Qt"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::PrimaryColors, i18n("Primary Colors"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::StoneMoss, i18n("Stone Moss"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::ArmyBlue, i18n("Army Blue"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::Retro, i18n("Retro"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::Ebony, i18n("Ebony"));
	ui.cbTheme->insertItem(Bar3DPlotArea::Theme::Isabelle, i18n("Isabelle"));
}
void Bar3DPlotAreaDock::setDataColumns() {
	int newCount = m_dataComboBoxes.count();
	int oldCount = m_bar->columns().count();

	if (newCount > oldCount) {
		ui.cbNumber->addItem(QString::number(newCount));
	} else {
		if (newCount != 0) {
			ui.cbNumber->removeItem(ui.cbNumber->count() - 1);
		}
	}

	QVector<AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_bar->setColumns(columns);
}

void Bar3DPlotAreaDock::addDataColumn() {
	auto* cb = new TreeViewComboBox(this);

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
	cb->setModel(aspectModel());
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
		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(button, &QPushButton::clicked, this, &BarPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void Bar3DPlotAreaDock::removeDataColumn() {
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

//*************************************************************
//**** SLOTs for changes triggered in bar3DPlotAreaDock ***
//*************************************************************
// Tab "General"

void Bar3DPlotAreaDock::columnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;
}

//*************************************************************
//***** SLOTs for changes triggered in bar3DPlotArea ******
//*************************************************************
// Tab "General"

void Bar3DPlotAreaDock::barXRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(xRot);
}

void Bar3DPlotAreaDock::barYRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(yRot);
}
void Bar3DPlotAreaDock::barZoomLevelChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(val);
}
void Bar3DPlotAreaDock::barShadowQualityChanged(Bar3DPlotArea::ShadowQuality shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(shadowQuality);
}

void Bar3DPlotAreaDock::barThemeChanged(Bar3DPlotArea::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(theme);
}
void Bar3DPlotAreaDock::xRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setXRotation(xRot);
}

void Bar3DPlotAreaDock::yRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setYRotation(yRot);
}
void Bar3DPlotAreaDock::zoomLevelChanged(int zoomLevel) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setZoomLevel(zoomLevel);
}
void Bar3DPlotAreaDock::shadowQualityChanged(int shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	for (Bar3DPlotArea* bar : m_bars)
		bar->setShadowQuality(static_cast<Bar3DPlotArea::ShadowQuality>(shadowQuality));
}

void Bar3DPlotAreaDock::themeChanged(int theme) {
	CONDITIONAL_LOCK_RETURN;
	for (Bar3DPlotArea* bar : m_bars)
		bar->setTheme(static_cast<Bar3DPlotArea::Theme>(theme));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Bar3DPlotAreaDock::load() {
	// TODO
}

void Bar3DPlotAreaDock::loadConfig(KConfig& config) {
	// TODO
}
