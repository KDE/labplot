/*
    File                 : ExamplesDialog.cpp
    Project              : LabPlot
    Description          : dialog showing the available example projects
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExamplesDialog.h"
#include "kdefrontend/examples/ExamplesWidget.h"

#include <QDialogButtonBox>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ExamplesDialog
	\brief Dialog showing the available example projects.

	\ingroup kdefrontend
 */
ExamplesDialog::ExamplesDialog(QWidget* parent) : QDialog(parent),
	m_examplesWidget(new ExamplesWidget(this)) {

	connect(m_examplesWidget, &ExamplesWidget::doubleClicked, this, &QDialog::accept);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_examplesWidget);

	//dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);
	layout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Example Projects"));
	setWindowIcon(QIcon::fromTheme("folder-documents"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ExamplesDialog::~ExamplesDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

/*!
 * return the path for the current selected example project
 */
QString ExamplesDialog::path() const {
	return m_examplesWidget->path();
}
