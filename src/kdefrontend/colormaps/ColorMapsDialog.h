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
#ifndef COLORMAPSDIALOG_H
#define COLORMAPSDIALOG_H

#include <QDialog>

class ColorMapsWidget;

class ColorMapsDialog : public QDialog {
    Q_OBJECT

public:
	explicit ColorMapsDialog(QWidget*);
	~ColorMapsDialog() override;

	QPixmap previewPixmap() const;
	QString name() const;
	QVector<QColor> colors() const;

private:
	ColorMapsWidget* m_colorMapsWidget;

};

#endif // COLORMAPSDIALOG_H
