/***************************************************************************
    File                 : ExportWorksheetDialog.cpp
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de

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

#include "ExportWorksheetDialog.h"

#include <QFileDialog>
#include <KUrlCompletion>
#include <KMessageBox>
#include <KPushButton>
#include <QStringList>
#include <QDebug>
/*!
	\class ExportWorksheetDialog
	\brief Dialog for exporting a worksheet  to a file.

	\ingroup kdefrontend
*/
 
ExportWorksheetDialog::ExportWorksheetDialog(QWidget* parent) : KDialog(parent) {
	mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	ui.gbOptions->hide();

	KUrlCompletion *comp = new KUrlCompletion();
    ui.kleFileName->setCompletionObject(comp);
	
	ui.bOpen->setIcon( KIcon("document-open") );
	
	ui.cbFormat->addItem(KIcon("application-pdf"), "Portable data format (PDF)");
	ui.cbFormat->addItem(KIcon("image-x-eps"), "Encapsulated PostScript (EPS)");
	ui.cbFormat->addItem(KIcon("image-svg+xml"), "Scalable Vector Graphics (SVG)");
	ui.cbFormat->insertSeparator(3);
	ui.cbFormat->addItem(KIcon("image-x-generic"), "Portable Network Graphics (PNG)");
	
	ui.cbExportArea->addItem("Object's bounding box");
	ui.cbExportArea->addItem("Current selection");
	ui.cbExportArea->addItem("Complete worksheet");
	
	setMainWidget( mainWidget );
	
	setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
	setButtonText(KDialog::User1,i18n("Options") + ">>");
	
	connect( ui.cbFormat, SIGNAL(currentIndexChanged(int)), SLOT(formatChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect(this,SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));

	setCaption(i18n("Export worksheet"));
	setWindowIcon(KIcon("document-export-database"));
	
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	ui.cbFormat->setCurrentIndex(conf.readEntry("Format", "").toInt());
	ui.cbExportArea->setCurrentIndex(conf.readEntry("Area", "").toInt());	
	
	resize( QSize(500,0).expandedTo(minimumSize()) );
}

void ExportWorksheetDialog::setFileName(const QString& name){
	ui.kleFileName->setText(QDir::homePath() + "/" +  name + ".pdf");
}

QString ExportWorksheetDialog::path() const{
	return ui.kleFileName->text();
}

WorksheetView::ExportFormat ExportWorksheetDialog::exportFormat() const{
	int index = ui.cbFormat->currentIndex();
	
	//we have a separator in the format combobox at the 4th position -> skip it
	if (index>3)
		index --;

	return WorksheetView::ExportFormat(index);
}

WorksheetView::ExportArea ExportWorksheetDialog::exportArea() const{
	return WorksheetView::ExportArea(ui.cbExportArea->currentIndex());
}

void ExportWorksheetDialog::slotButtonClicked(int button) {
	if (button == KDialog::Ok)
		okClicked();
	else
		KDialog::slotButtonClicked(button);
}

//SLOTS
void ExportWorksheetDialog::okClicked(){
	if ( QFile::exists(ui.kleFileName->text()) ){
		int r=KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
		if (r==KMessageBox::No) {
			return;
		}
	}

    KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
    conf.writeEntry("Format", ui.cbFormat->currentIndex());
	conf.writeEntry("Area", ui.cbExportArea->currentIndex());
	
	accept();
}

/*!
	Shows/hides the GroupBox with export options in this dialog.
*/
void ExportWorksheetDialog::toggleOptions(){
	if (ui.gbOptions->isVisible()){
		ui.gbOptions->hide();
		setButtonText(KDialog::User1,i18n("Options") + ">>");
	}else{
		ui.gbOptions->show();
		setButtonText(KDialog::User1,i18n("Options") + "<<");
	}

	//resize the dialog
	mainWidget->resize(layout()->minimumSize());
	layout()->activate();
 	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportWorksheetDialog::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, i18n("Export to file"));
    if (fileName != "")
		ui.kleFileName->setText(fileName);
}

/*!
	called when the output format was changed. Adjusts the extension for the specified file.
 */
void ExportWorksheetDialog::formatChanged(int index){
	//we have a separator in the format combobox at the 4th posiiton -> skip it
	if (index>3)
		index --;

	QStringList extensions;
	extensions<<".pdf"<<".eps"<<".svg"<<".png";
	QString path = ui.kleFileName->text();
	int i = path.indexOf(".");
	if (i==-1)
		path = path + extensions.at(index);
	else
		path=path.left(i) + extensions.at(index);

	ui.kleFileName->setText(path);
}
