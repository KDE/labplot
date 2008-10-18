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

// #include <fstream>
//using namespace std;
#include "ColorMapPreview.h"

ColorMapPreview::ColorMapPreview( QWidget *parent ): KPreviewWidgetBase( parent ){
	label = new QLabel(this);
// 	setFrameShape( QFrame::StyledPanel );
// 	setFrameShadow( QFrame::Sunken );
	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->addWidget( label );
}

ColorMapPreview::~ColorMapPreview(){
}

void ColorMapPreview::showPreview( const KUrl &u ){
	QString path = u.path();
    QFileInfo fi( path );
	if (fi.suffix() != "map" && fi.suffix() != "MAP"){
      label->setText( "No color map" );
	}else{
			if ( open(path) )
				label->setPixmap( pixmap );
	}
}

void ColorMapPreview::clearPreview(){
	label->setPixmap(0);
	label->setText( "No color map" );
}

bool ColorMapPreview::open(QString fname) {
#ifdef HAVE_GL
	ifstream file((const char*)fname.local8Bit());

	Qwt3D::RGBA rgb;
	cv.clear();

	while ( file ) {
		file >> rgb.r >> rgb.g >> rgb.b;
		file.ignore(10000,'\n');
		if (!file.good())
			break;
		else
		{
			rgb.a = 1;
			rgb.r /= 255;
			rgb.g /= 255;
			rgb.b /= 255;
			cv.push_back(rgb);
		}
	}

	pixmap.resize(80, cv.size());
	QPainter p( &pixmap );
  p.translate( 0, cv.size()-1 );
	for (unsigned i=0; i!=cv.size(); ++i)
	{
		rgb = cv[i];
		p.setPen(Qwt3D::GL2Qt(rgb.r,rgb.g,rgb.b));
		p.drawLine(QPoint(0,0),QPoint(pixmap.width(),0));
    p.translate( 0, -1 );
	}
  p.end();

#endif
	return true;
}
