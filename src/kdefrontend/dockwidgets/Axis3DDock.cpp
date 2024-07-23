/*
	File                 : Axis3DDock.cpp
	Project              : LabPlot
	Description          : widget for Axis3D properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Axis3DDock.h"
#include "TreeViewComboBox.h"
#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/3d/Surface3DPlotArea.h"
#include <backend/core/AspectTreeModel.h>

Axis3DDock::Axis3DDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	ui.dsbMinRange->setMinimum(-std::numeric_limits<double>::max());
	ui.dsbMinRange->setMinimum(-std::numeric_limits<double>::max());

	// SIGNALs/SLOTs
	connect(ui.leXLabel, &QLineEdit::textChanged, this, &Axis3DDock::titleChanged);
	connect(ui.dsbMinRange, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Axis3DDock::minRangeChanged);
	connect(ui.dsbMaxRange, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Axis3DDock::maxRangeChanged);
	connect(ui.sbSegmentCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &Axis3DDock::segmentCountChanged);
	connect(ui.sbSubSegmentCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &Axis3DDock::subSegmentCountChanged);
	connect(ui.cbXLabelFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Axis3DDock::formatChanged);
}

void Axis3DDock::setAxes(const QList<Axis3D*>& axes) {
	CONDITIONAL_LOCK_RETURN;
	m_axes = axes;
	m_axis = m_axes.first();
	// Load properties of the first axis
	load();

	for (auto* axis : m_axes) {
		connect(axis, &Axis3D::minRangeChanged, this, &Axis3DDock::axisMinRangeChanged);
		connect(axis, &Axis3D::maxRangeChanged, this, &Axis3DDock::axisMaxRangeChanged);
		connect(axis, &Axis3D::segmentCountChanged, this, &Axis3DDock::axisSegmentCountChanged);
		connect(axis, &Axis3D::subSegmentCountChanged, this, &Axis3DDock::axisSubSegmentCountChanged);
		connect(axis, &Axis3D::titleChanged, this, &Axis3DDock::axisTitleChanged);
		connect(axis, &Axis3D::axisFormatChanged, this, &Axis3DDock::axisFormatChanged);
	}
}

void Axis3DDock::updateUiVisibility() {
	// Implement UI visibility update logic here
}

void Axis3DDock::load() {
	ui.leXLabel->setText(m_axis->title());
	ui.dsbMinRange->setValue(m_axis->minRange());
	ui.dsbMaxRange->setValue(m_axis->maxRange());
	ui.sbSegmentCount->setValue(m_axis->segmentCount());
	ui.sbSubSegmentCount->setValue(m_axis->subSegmentCount());
	ui.cbXLabelFormat->setCurrentIndex(static_cast<int>(m_axis->axisFormat()));
}

void Axis3DDock::loadConfig(KConfig& config) {
	// Implement loading from config logic here
}

void Axis3DDock::retranslateUi() {
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_Decimal, i18n("Numeric"));
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_Scientific, i18n("Scientific"));
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_MultiplierOfPi, i18n("Multiple of Pi"));
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_PowerOf10, i18n("Power of 10"));
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_PowerOf2, i18n("Power of 2"));
	ui.cbXLabelFormat->insertItem(Axis3D::Format::Format_PowerOfE, i18n("Power of e"));
}

// SLOTs for changes triggered in Axis3DDock
void Axis3DDock::titleChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axes) {
		axis->setTitle(text);
	}
}

void Axis3DDock::minRangeChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axes) {
		axis->setMinRange(value);
	}
}
void Axis3DDock::maxRangeChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axes) {
		axis->setMaxRange(value);
	}
}

void Axis3DDock::segmentCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axes) {
		axis->setSegmentCount(count);
	}
}

void Axis3DDock::subSegmentCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* axis : m_axes) {
		axis->setSubSegmentCount(count);
	}
}

void Axis3DDock::formatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;
	Axis3D::Format newFormat = static_cast<Axis3D::Format>(index);
	for (auto* axis : m_axes) {
		axis->setAxisFormat(newFormat);
	}
}

// SLOTs for changes triggered in Axis3D
void Axis3DDock::axisMinRangeChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.dsbMinRange->setValue(value);
	ui.dsbMaxRange->setMinimum(value);
}

void Axis3DDock::axisMaxRangeChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.dsbMaxRange->setValue(value);
	ui.dsbMinRange->setMaximum(value);
}

void Axis3DDock::axisSegmentCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSegmentCount->setValue(count);
}

void Axis3DDock::axisSubSegmentCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbSubSegmentCount->setValue(count);
}

void Axis3DDock::axisTitleChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;
	ui.leXLabel->setText(text);
}

void Axis3DDock::axisFormatChanged(Axis3D::Format format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbXLabelFormat->setCurrentIndex(static_cast<int>(format));
}
