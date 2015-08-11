/***************************************************************************
    File                 : ColorMapSelector.h
    Project              : LabPlot
    Description          : Provides a push button for the selection of a color map via ColorMapDialog.
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include "commonfrontend/widgets/ColorMapSelector.h"
#include "backend/worksheet/plots/ColorMapManager.h"
#include "kdefrontend/worksheet/ColorMapSelectorDialog.h"
#include <QDebug>

/*!
    \class ColorMapSelector
    \brief Provides a push button for the selection of a color map via ColorMapSelectorDialog.

    \ingroup commonfrontend/widgets
*/


ColorMapSelector::ColorMapSelector(QWidget* parent) : QPushButton(parent), m_colorMapId(ColorMapManager::INVALID) {
	connect(this, SIGNAL(clicked(bool)), this, SLOT(showDialog()) );
}

void ColorMapSelector::setColorMap(const ColorMapManager::ColorMapId id) {
	m_colorMapId = id;
	updatePreview();
}

void ColorMapSelector::showDialog() {
	ColorMapSelectorDialog* dlg = new ColorMapSelectorDialog(this);
	dlg->setColorMapId(m_colorMapId);
	if (dlg->exec() == QDialog::Accepted){
		m_colorMapId = dlg->colorMapId();
		updatePreview();
		emit changed(m_colorMapId);
	}

	return;
}

void ColorMapSelector::updatePreview() {
	//TODO:
	const int width = size().width()-6;
	const int height = size().height()-6;
	QPixmap pix(width, height);
	ColorMapManager::getInstance()->fillPixmap(pix, m_colorMapId);
	QPushButton::setIcon(QIcon(pix));
	QPushButton::setIconSize(QSize(width, height));
}
