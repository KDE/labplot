/*
	File                 : Bar3DPlotDock.cpp
	Project              : LabPlot
	Description          : widget for Bar3DPlotArea properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Bar3DPlotDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include <backend/core/AspectTreeModel.h>

Bar3DPlotDock::Bar3DPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	// data-columns
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	m_gridLayout = new QGridLayout(ui.frameColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameColumns->setLayout(m_gridLayout);

	// SIGNALs/SLOTs
	// General
	connect(m_buttonNew, &QPushButton::clicked, this, &Bar3DPlotDock::addDataColumn);
	connect(ui.cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Bar3DPlotDock::themeChanged);
	connect(ui.cbShadowQuality, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Bar3DPlotDock::shadowQualityChanged);
	connect(ui.slXRot, &QSlider::sliderMoved, this, &Bar3DPlotDock::xRotationChanged);
	connect(ui.slYRot, &QSlider::sliderMoved, this, &Bar3DPlotDock::yRotationChanged);
	connect(ui.slZoom, &QSlider::sliderMoved, this, &Bar3DPlotDock::zoomLevelChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &Bar3DPlotDock::colorChanged);
}

void Bar3DPlotDock::setBars(const QList<Bar3DScene*>& bars) {
	CONDITIONAL_LOCK_RETURN;
	m_bars = bars;
	m_bar = m_bars.first();
	setAspects(bars);

	// show the properties of the first bar
	// tab "General"
	loadDataColumns();

	ui.cbTheme->setCurrentIndex(m_bar->theme());
	ui.cbShadowQuality->setCurrentIndex(m_bar->shadowQuality());
	ui.slZoom->setValue(m_bar->zoomLevel());
	ui.slXRot->setRange(0, 90);
	ui.slYRot->setRange(0, 90);
	ui.slZoom->setRange(100, 400);
	ui.slXRot->setValue(m_bar->xRotation());
	ui.slYRot->setValue(m_bar->yRotation());
	ui.kcbColor->setColor(m_bar->color());

	connect(m_bar, &Bar3DScene::themeChanged, this, &Bar3DPlotDock::barThemeChanged);
	connect(m_bar, &Bar3DScene::shadowQualityChanged, this, &Bar3DPlotDock::barShadowQualityChanged);
	connect(m_bar, &Bar3DScene::zoomLevelChanged, this, &Bar3DPlotDock::barZoomLevelChanged);
	connect(m_bar, &Bar3DScene::xRotationChanged, this, &Bar3DPlotDock::barXRotationChanged);
	connect(m_bar, &Bar3DScene::yRotationChanged, this, &Bar3DPlotDock::barYRotationChanged);
}

void Bar3DPlotDock::retranslateUi() {
	// This function should contain translation code if needed
	ui.cbShadowQuality->insertItem(Bar3DScene::None, i18n("None"));
	ui.cbShadowQuality->insertItem(Bar3DScene::Low, i18n("Low"));
	ui.cbShadowQuality->insertItem(Bar3DScene::Medium, i18n("Medium"));
	ui.cbShadowQuality->insertItem(Bar3DScene::High, i18n("High"));
	ui.cbShadowQuality->insertItem(Bar3DScene::SoftLow, i18n("Soft Low"));
	ui.cbShadowQuality->insertItem(Bar3DScene::SoftMedium, i18n("Soft Medium"));
	ui.cbShadowQuality->insertItem(Bar3DScene::SoftHigh, i18n("Soft High"));

	ui.cbTheme->insertItem(Bar3DScene::Theme::Qt, i18n("Qt"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::PrimaryColors, i18n("Primary Colors"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::StoneMoss, i18n("Stone Moss"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::ArmyBlue, i18n("Army Blue"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::Retro, i18n("Retro"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::Ebony, i18n("Ebony"));
	ui.cbTheme->insertItem(Bar3DScene::Theme::Isabelle, i18n("Isabelle"));
}
void Bar3DPlotDock::setDataColumns() {
	int newCount = m_dataComboBoxes.count();
	int oldCount = m_bar->dataColumns().count();

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

	m_bar->setDataColumns(columns);
}

void Bar3DPlotDock::addDataColumn() {
	auto* cb = new TreeViewComboBox(this);
	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	auto* model = aspectModel();

	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cb->setModel(model);
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &Bar3DPlotDock::columnChanged);

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
		connect(button, &QPushButton::clicked, this, &Bar3DPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lColumn->setText(i18n("Columns:"));
}

void Bar3DPlotDock::removeDataColumn() {
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

	if (!m_removeButtons.isEmpty()) {
		ui.lColumn->setText(i18n("Columns:"));
	} else {
		ui.lColumn->setText(i18n("Column:"));
	}

	if (!m_initializing)
		setDataColumns();
}

//*************************************************************
//**** SLOTs for changes triggered in bar3DPlotAreaDock ***
//*************************************************************
// Tab "General"

void Bar3DPlotDock::columnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;
	setDataColumns();
}

//*************************************************************
//***** SLOTs for changes triggered in bar3DPlotArea ******
//*************************************************************
// Tab "General"

void Bar3DPlotDock::barXRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slXRot->setValue(xRot);
}

void Bar3DPlotDock::barYRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	ui.slYRot->setValue(yRot);
}
void Bar3DPlotDock::barZoomLevelChanged(int val) {
	CONDITIONAL_LOCK_RETURN;
	ui.slZoom->setValue(val);
}
void Bar3DPlotDock::barShadowQualityChanged(Base3DPlot::ShadowQuality shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbShadowQuality->setCurrentIndex(shadowQuality);
}

void Bar3DPlotDock::barThemeChanged(Base3DPlot::Theme theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentIndex(theme);
}

void Bar3DPlotDock::barColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}
void Bar3DPlotDock::xRotationChanged(int xRot) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setXRotation(xRot);
}

void Bar3DPlotDock::yRotationChanged(int yRot) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setYRotation(yRot);
}
void Bar3DPlotDock::zoomLevelChanged(int zoomLevel) {
	CONDITIONAL_LOCK_RETURN;
	m_bar->setZoomLevel(zoomLevel);
}
void Bar3DPlotDock::shadowQualityChanged(int shadowQuality) {
	CONDITIONAL_LOCK_RETURN;
	for (Bar3DScene* bar : m_bars)
		bar->setShadowQuality(static_cast<Base3DPlot::ShadowQuality>(shadowQuality));
}

void Bar3DPlotDock::themeChanged(int theme) {
	CONDITIONAL_LOCK_RETURN;
	for (Bar3DScene* bar : m_bars)
		bar->setTheme(static_cast<Base3DPlot::Theme>(theme));
}

void Bar3DPlotDock::colorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	for (Bar3DScene* bar : m_bars)
		bar->setColor(color);
}
void Bar3DPlotDock::loadDataColumns() {
	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_bar->dataColumns().count();
	ui.cbNumber->clear();

	auto* model = aspectModel();
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
		for (int i = 0; i < count; ++i) {
			m_dataComboBoxes.at(i)->setModel(model); // the model might have changed in-between, reset the current model
			m_dataComboBoxes.at(i)->setAspect(m_bar->dataColumns().at(i));
		}

		// show columns names in the combobox for the selection of the bar to be modified
		for (int i = 0; i < count; ++i)
			if (m_bar->dataColumns().at(i)) {
				const auto& name = m_bar->dataColumns().at(i)->name();
				ui.cbNumber->addItem(name);
			}
	} else {
		// no data columns set in the box plot yet, we show the first combo box only and reset its model
		m_dataComboBoxes.first()->setModel(model);
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 0; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	// disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_bars.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);

	// select the first column after all of them were added to the combobox
	ui.cbNumber->setCurrentIndex(0);
}

void Bar3DPlotDock::barColumnsChanged(const QVector<const AbstractColumn*>&) {
	CONDITIONAL_LOCK_RETURN;
	loadDataColumns();
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Bar3DPlotDock::load() {
	// TODO
}

void Bar3DPlotDock::loadConfig(KConfig& config) {
	// TODO
}
