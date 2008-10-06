/***************************************************************************
    File                 : ColorMapPreview.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : color map preview
                           
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
#ifndef COLORMAPPREVIEW_H
#define COLORMAPPREVIEW_H

#include <QtGui>
#include <kpreviewwidgetbase.h>
#include <kurl.h>

#ifdef HAVE_GL
#include "qwt3d_types.h"
#endif

class ColorMapPreview : public KPreviewWidgetBase{
 	Q_OBJECT

public:
	ColorMapPreview( QWidget *parent );
// 	~ColorMapPreview();

private:
#ifdef HAVE_GL
	Qwt3D::ColorVector cv;
#endif
	QLabel* label;
	QPixmap pixmap;
	bool open(QString);

public slots:
	void showPreview( const KUrl& u );
	void clearPreview();
};

#endif
