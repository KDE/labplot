/***************************************************************************
	File                 : ColorMapsDialog.h
	Project              : LabPlot
	Description          : dialog showing the available color maps
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

#include "ColorMapsDialog.h"
#include "kdefrontend/colormaps/ColorMapsWidget.h"

#include <QDialogButtonBox>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ColorMapsDialog
	\brief Dialog for importing data from a dataset. Embeds \c ColorMapsWidget and provides the standard buttons.

	\ingroup kdefrontend
 */
ColorMapsDialog::ColorMapsDialog(QWidget* parent) : QDialog(parent),
	m_colorMapsWidget(new ColorMapsWidget(this)){

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_colorMapsWidget);

	//dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);
	layout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Color Maps Browser"));
	setWindowIcon(QIcon::fromTheme("color-management"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf(KSharedConfig::openConfig(), "ColorMapsDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ColorMapsDialog::~ColorMapsDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ColorMapsDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QPixmap ColorMapsDialog::previewPixmap() const {
	return m_colorMapsWidget->previewPixmap();
}
