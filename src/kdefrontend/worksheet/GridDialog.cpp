/*
    File                 : GridDialog.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2020 Alexander Semke
    Email (use @ for *)  : alexander.semke@web.de
    Description          : dialog for editing the grid properties for the worksheet view

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "GridDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpinBox>
#include <QGridLayout>
#include <QWindow>

#include <KLocalizedString>
#include <KColorButton>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

//TODO:
//1. improve the layout and move the UI-part to a ui-file
//2. restore the currently active grid settings

/**
 * @brief Provides a dialog for editing the grid properties for the worksheet view
 * \ingroup kdefrontend
 */
GridDialog::GridDialog(QWidget* parent) : QDialog(parent) {
	setWindowTitle(i18nc("@title:window", "Custom Grid"));

	QWidget* widget = new QWidget;
	auto* layout = new QGridLayout(widget);

	QLabel* label = new QLabel(i18n("Style:"), widget);
	layout->addWidget(label, 0, 0);

	cbStyle = new QComboBox(this);
	cbStyle->addItem(i18n("Lines"));
	cbStyle->addItem(i18n("Dots"));
	cbStyle->setCurrentIndex(0);
	layout->addWidget(cbStyle, 0, 1);

	label = new QLabel(i18n("Horizontal spacing:"), widget);
	layout->addWidget(label, 1, 0);

	sbHorizontalSpacing = new QSpinBox(widget);
	sbHorizontalSpacing->setRange(1,100);
	sbHorizontalSpacing->setValue(10);
	layout->addWidget(sbHorizontalSpacing, 1, 1);

	label = new QLabel(i18n("Vertical spacing:"), widget);
	layout->addWidget(label, 2, 0);

	sbVerticalSpacing = new QSpinBox(widget);
	sbVerticalSpacing->setRange(1,100);
	sbVerticalSpacing->setValue(10);
	layout->addWidget(sbVerticalSpacing, 2, 1);

	label = new QLabel(i18n("Color:"), widget);
	layout->addWidget(label, 3, 0);

	kcbColor = new KColorButton(widget);
	kcbColor->setColor(Qt::gray);
	layout->addWidget(kcbColor , 3, 1);

	label = new QLabel(i18n("Opacity:"), widget);
	layout->addWidget(label, 4, 0);

	sbOpacity = new QSpinBox(widget);
	sbOpacity->setRange(1,100);
	sbOpacity->setValue(100);
	layout->addWidget(sbOpacity, 4, 1);

	label = new QLabel(i18n(" %"), widget);
	layout->addWidget(label, 4, 2);

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	auto* vlayout = new QVBoxLayout(this);
	vlayout->addWidget(widget);
	vlayout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GridDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

GridDialog::~GridDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("GridDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void GridDialog::save(WorksheetView::GridSettings& settings) {
	if (cbStyle->currentIndex() == 0)
		settings.style  = WorksheetView::GridStyle::Line;
	else
		settings.style  = WorksheetView::GridStyle::Dot;

	settings.horizontalSpacing = sbHorizontalSpacing->value();
	settings.verticalSpacing = sbVerticalSpacing->value();
	settings.color = kcbColor->color();
	settings.opacity = (float)sbOpacity->value()/100;
}
