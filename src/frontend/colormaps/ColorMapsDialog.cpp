/*
	File                 : ColorMapsDialog.cpp
	Project              : LabPlot
	Description          : dialog showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ColorMapsDialog.h"
#include "backend/core/Settings.h"
#include "frontend/colormaps/ColorMapsWidget.h"

#include <QDialogButtonBox>
#include <QWindow>

#include <KConfigGroup>
#include <KWindowConfig>

/*!
	\class ColorMapsDialog
	\brief Dialog showing the available color maps. Embeds \c ColorMapsWidget and provides the standard buttons.

	\ingroup frontend
 */
ColorMapsDialog::ColorMapsDialog(QWidget* parent)
	: QDialog(parent)
	, m_colorMapsWidget(new ColorMapsWidget(this)) {
	connect(m_colorMapsWidget, &ColorMapsWidget::doubleClicked, this, &QDialog::accept);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_colorMapsWidget);

	// dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	layout->addWidget(buttonBox);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

	setWindowTitle(i18nc("@title:window", "Color Maps Browser"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("color-management")));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ColorMapsDialog::~ColorMapsDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QPixmap ColorMapsDialog::previewPixmap() const {
	return m_colorMapsWidget->previewPixmap();
}

QString ColorMapsDialog::name() const {
	return m_colorMapsWidget->name();
}

QVector<QColor> ColorMapsDialog::colors() const {
	return m_colorMapsWidget->colors();
}
