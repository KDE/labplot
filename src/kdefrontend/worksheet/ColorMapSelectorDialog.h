/***************************************************************************
    File                 : ColorMapSelectorDialog.h
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
#ifndef COLORMAPDIALOG_H
#define COLORMAPDIALOG_H

#include "backend/worksheet/plots/ColorMapManager.h"
#include "ui_colormapselectorwidget.h"
#include <KDialog>

class ColorMapSelectorDialog : public KDialog{
	Q_OBJECT

	public:
		explicit ColorMapSelectorDialog(QWidget* parent = 0, Qt::WFlags fl = 0);
		ColorMapManager::ColorMapId  colorMapId() const;
		void setColorMapId(ColorMapManager::ColorMapId);

	private:
		Ui::ColorMapSelectorWidget ui;
		ColorMapManager::ColorMapId m_colorMapId;

		bool eventFilter(QObject*, QEvent*);
		void updatePreview();

	private slots:
		void currentNameChanged(const QString&);

};

#endif
