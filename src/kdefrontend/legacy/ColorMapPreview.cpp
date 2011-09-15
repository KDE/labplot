/***************************************************************************
    File                 : ColorMapPreview.cc
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

#include "ColorMapPreview.h"
#include "../tools/ColorMapRenderer.h"
#include <KDebug>

ColorMapPreview::ColorMapPreview( QWidget *parent ): KPreviewWidgetBase( parent ){
	label = new QLabel(this);
	label->setFrameShape( QFrame::StyledPanel );
	label->setFrameShadow( QFrame::Sunken );
	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setAlignment( Qt::AlignHCenter );
	layout->addWidget( label );
 	this->setMaximumSize ( QSize(100, 16777215) );
}

ColorMapPreview::~ColorMapPreview(){
}

void ColorMapPreview::showPreview( const KUrl &u ){
	QString path = u.path();
    QFileInfo fi( path );
	if (fi.suffix() != "map" && fi.suffix() != "MAP"){
    	 label->setText( "No color map" );
	}else{
		QPixmap pix=ColorMapRenderer::pixmap(path).scaled(this->width()-10, this->height()-10);
		label->setPixmap( pix );
	}
}

void ColorMapPreview::clearPreview(){
	label->setPixmap(0);
	label->setText( "No color map" );
}
