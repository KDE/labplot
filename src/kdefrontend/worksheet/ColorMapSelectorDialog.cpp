/***************************************************************************
    File                 : ColorMapSelectorDialog.cpp
    Project              : LabPlot
    Description          : Dialog allowing the selection of a color map
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
#include "ColorMapSelectorDialog.h"

/*!
	\class ColorMapSelectorDialog
	\brief Dialog allowing the selection of a color map

	\ingroup kdefrontend
 */

ColorMapSelectorDialog::ColorMapSelectorDialog(QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl), m_colorMapId(ColorMapManager::INVALID) {

	setWindowTitle(i18n("Color Map Selector"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	const QStringList& names = ColorMapManager::getInstance()->colorMapNames();
	ui.lwColorMaps->addItems(names);

	ui.lPreview->installEventFilter(this);

	setButtons( KDialog::Ok | KDialog::Cancel );
	setButtonToolTip(KDialog::Ok, i18n("Select current color map"));

	resize( QSize(400,400).expandedTo(minimumSize()) );

	connect(ui.lwColorMaps, SIGNAL(currentTextChanged(QString)), this, SLOT(currentNameChanged(QString)));
}

void ColorMapSelectorDialog::setColorMapId(ColorMapManager::ColorMapId id) {
	m_colorMapId = id;
	updatePreview();
}

ColorMapManager::ColorMapId ColorMapSelectorDialog::colorMapId() const {
	return ColorMapManager::getInstance()->colorMapId(ui.lwColorMaps->currentItem()->text());
}

void ColorMapSelectorDialog::currentNameChanged(const QString& name) {
	m_colorMapId = ColorMapManager::getInstance()->colorMapId(name);
	updatePreview();
}

void ColorMapSelectorDialog::updatePreview() {
	QPixmap pix(ui.lPreview->size());
	ColorMapManager::getInstance()->fillPixmap(pix, m_colorMapId, Qt::Vertical);
	ui.lPreview->setPixmap(pix);
}

bool ColorMapSelectorDialog::eventFilter(QObject* watched, QEvent* event) {
	if (watched == ui.lPreview && event->type() == QEvent::Resize){
		updatePreview();
		return true;
	} else {
		return QWidget::eventFilter(watched, event);
	}
}
