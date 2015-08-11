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

#ifndef COLORMAPSELECTOR_H
#define COLORMAPSELECTOR_H

#include "backend/worksheet/plots/ColorMapManager.h"
#include <QPushButton>

class ColorMapSelector : public QPushButton {
	Q_OBJECT

	public:
		explicit ColorMapSelector(QWidget* parent = 0);
		void setColorMap(const ColorMapManager::ColorMapId);

	private:
		ColorMapManager::ColorMapId m_colorMapId;
		void updatePreview();

	private slots:
		void showDialog();

	signals:
		void changed(const ColorMapManager::ColorMapId) const;
};

#endif
