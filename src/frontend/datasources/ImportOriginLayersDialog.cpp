/*
	File                 : ImportOriginLayersDialog.cpp
	Project              : LabPlot
	Description          : Dialog providing the option to select how to import multiple layers in the Origin file.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportOriginLayersDialog.h"
#include "backend/core/Settings.h"
#include "ui_importoriginlayersdialog.h"

#include <KWindowConfig>
#include <QWindow>

ImportOriginLayersDialog::ImportOriginLayersDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::ImportOriginLayersDialog) {
	ui->setupUi(this);

	setWindowTitle(i18nc("@title:window", "Import Origin Multi-Layer Graphs"));
	setWindowIcon(QIcon::fromTheme(QStringLiteral("document-import")));

	QString info = i18n(
		"Multiple layers are used in Origin to either implement multiple plots or multiple axes on the same plot "
		"(see <a href=\"https://www.originlab.com/doc/Origin-Help/MultiLayer-Graph\">Origin's Documentation</a> for more details).<br>"
		"LabPlot can process only one type at the same time."
		"<br><br>"
		"Specify how to import multi-layered graphs in the selected project:"
		"<ul>"
		"<li>As Plot Area - a new plot area is created for every layer.</li>"
		"<li>As Coordinate System - a new coordinate system (data range) on the same plot area is created for every layer.</li>"
		"</ul>");
	ui->lInfo->setText(info);

	// add options to control how to read Origin's graph layers - as new plot areas or as new coordinate systems
	ui->cbGraphLayer->addItem(i18n("As Plot Area"));
	ui->cbGraphLayer->addItem(i18n("As Coordinate System"));

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("ImportOriginLayersDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	ui->cbGraphLayer->setCurrentIndex(conf.readEntry(QStringLiteral("GraphLayer"), 0));
}

ImportOriginLayersDialog::~ImportOriginLayersDialog() {
	// save current settings
	KConfigGroup conf = Settings::group(QStringLiteral("ImportProjectDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
	conf.writeEntry(QStringLiteral("GraphLayer"), ui->cbGraphLayer->currentIndex());

	delete ui;
}

bool ImportOriginLayersDialog::graphLayersAsPlotArea() const {
	return (ui->cbGraphLayer->currentIndex() == 0);
}
