/***************************************************************************
    File                 : PlotSurfaceStyleWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for surface plot style

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
#include "PlotSurfaceStyleWidget.h"
#include "../elements/Style.h"
#include "../tools/ColorMapRenderer.h"

#include <KFileDialog>
#include <KMessageBox>
#include <KDebug>

PlotSurfaceStyleWidget::PlotSurfaceStyleWidget(QWidget* parent):QWidget(parent), PlotStyleWidgetInterface(){
	ui.setupUi(this);
	ui.bOpenColorMap->setIcon( KIcon("document-open") );
	ui.bCreateColorMap->setIcon( KIcon("fill-color") );
	ui.leThreshold->setValidator( new QDoubleValidator(ui.leThreshold) );
	ui.leLevelsNumber->setValidator( new QDoubleValidator(ui.leLevelsNumber) );
	ui.kcbLevelsLineColor->setColor(Qt::black);

	this->fillPatternBox();
	ui.cbFillBrushStyle->setCurrentIndex(0);

	//Slots
	connect( ui.bOpenColorMap, SIGNAL(clicked()), this, SLOT(openColorMap()) );
	connect( ui.bCreateColorMap, SIGNAL(clicked()), this, SLOT(createColorMap()) );
	connect( ui.chbLevelsMultiColoring, SIGNAL(stateChanged(int)), this, SLOT(multiColoringChanged(int)) );
}

PlotSurfaceStyleWidget::~PlotSurfaceStyleWidget(){}

void PlotSurfaceStyleWidget::resizeEvent(QResizeEvent * event){
	this->fillPatternBox();
}

void PlotSurfaceStyleWidget::setStyle(const Style* style){
	//TODO
}

void PlotSurfaceStyleWidget::saveStyle(Style* style) const{
	//TODO
}


/*!
	fills the ComboBox for the density pattern with the 14 possible Qt::BrushStyles.
	Called on start and if the widget is resized.
*/
void PlotSurfaceStyleWidget::fillPatternBox() const{
	int index=ui.cbFillBrushStyle->currentIndex();
	ui.cbFillBrushStyle->clear();

	QPainter pa;
	int offset=5;
	int w=ui.cbFillBrushStyle->width() - 2*offset;
	int h=ui.cbFillBrushStyle->height() - 2*offset;
	QPixmap pm( w, h );
	ui.cbFillBrushStyle->setIconSize( QSize(w,h) );

	QPen pen(Qt::SolidPattern, 1);
 	pa.setPen( pen );

	//loop over 14 possible Qt::BrushStyles
	for (int i=1;i<15;i++) {
		pm.fill(Qt::transparent);
		pa.begin( &pm );
//  		pa.setRenderHint(QPainter::Antialiasing);
 		pa.setBrush( QBrush(Qt::black, (Qt::BrushStyle)i) );
		pa.drawRect( offset, offset, w-2*offset, h-2*offset);
		pa.end();
		ui.cbFillBrushStyle->addItem( QIcon::fromTheme(pm), "" );
	}

	ui.cbFillBrushStyle->setCurrentIndex(index);
}

//**********************************************************
//****************** SLOTS ********************************
//**********************************************************

/*!
	called if the "Open existing colormap"-button was clicked.
	Upon selecting a colormap file the content is rendered via ColorMapRenderer
	and is shown in the corresponding Label.
*/
void PlotSurfaceStyleWidget::openColorMap(){
// 	KUrl url=KUrl::fromPath(locate("data","LabPlot/colormaps/");//TODO
	KUrl url;
 	QString filter="*.map *.MAP|Colormap files (*.map, *.MAP)";

	KFileDialog* dialog = new KFileDialog(url, filter, this, 0);
	ColorMapPreview* colormap= new ColorMapPreview( this );
	dialog->setPreviewWidget(colormap);

	if (dialog->exec()){
		//get the pixmal and show on the button
		QString fileName=dialog->selectedFile();
		QPixmap pix=ColorMapRenderer::pixmap(fileName).scaled( ui.lColorMapPixmap->width()-5, ui.lColorMapPixmap->height()-5);
		ui.lColorMapPixmap->setPixmap( pix );
	}

}

/*!
	called if the "New colormap"-button was clicked.
	Starts the dialog for creating new gradients.
*/
void PlotSurfaceStyleWidget::createColorMap(){
	//TODO
	KMessageBox::error(this, i18n("Not yet implemented."));
}

void PlotSurfaceStyleWidget::multiColoringChanged(int state){
	if (state==Qt::Checked)
		ui.kcbLevelsLineColor->setEnabled(false);
	else
		ui.kcbLevelsLineColor->setEnabled(true);
}
