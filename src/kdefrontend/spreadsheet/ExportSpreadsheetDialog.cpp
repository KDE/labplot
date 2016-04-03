/***************************************************************************
    File                 : ExportSpreadsheetDialog.cpp
    Project              : LabPlot
    Description          : export spreadsheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2016 by Alexander Semke (alexander.semke@web.de)

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

#include "ExportSpreadsheetDialog.h"

#include <QFileDialog>
#include <KUrlCompletion>
#include <KMessageBox>

/*!
	\class ExportSpreadsheetDialog
	\brief Dialog for exporting a spreadsheet to a file.

	\ingroup kdefrontend
*/

ExportSpreadsheetDialog::ExportSpreadsheetDialog(QWidget* parent) : KDialog(parent) {
	mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	ui.gbOptions->hide();

	KUrlCompletion *comp = new KUrlCompletion();
    ui.kleFileName->setCompletionObject(comp);

	ui.cbFormat->addItem("ASCII");
	ui.cbFormat->addItem("Binary");
	//TODO: implement later
	ui.lFormat->hide();
	ui.cbFormat->hide();

	ui.cbSeparator->addItem("TAB");
	ui.cbSeparator->addItem("SPACE");
	ui.cbSeparator->addItem(",");
	ui.cbSeparator->addItem(";");
	ui.cbSeparator->addItem(":");
	ui.cbSeparator->addItem(",TAB");
	ui.cbSeparator->addItem(";TAB");
	ui.cbSeparator->addItem(":TAB");
	ui.cbSeparator->addItem(",SPACE");
	ui.cbSeparator->addItem(";SPACE");
	ui.cbSeparator->addItem(":SPACE");

	ui.bOpen->setIcon( KIcon("document-open") );

	setMainWidget( mainWidget );

	setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );

	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.kleFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged(QString)) );
	connect(this,SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));

	setCaption(i18n("Export spreadsheet"));
	setWindowIcon(KIcon("document-export-database"));

	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	ui.cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui.chkExportHeader->setChecked(conf.readEntry("Header", true));
	ui.cbSeparator->setCurrentItem(conf.readEntry("Separator", "TAB"));
	m_showOptions = conf.readEntry("ShowOptions", false);
	ui.gbOptions->setVisible(m_showOptions);
	m_showOptions ? setButtonText(KDialog::User1,i18n("Hide Options")) : setButtonText(KDialog::User1,i18n("Show Options"));
	restoreDialogSize(conf);
}

ExportSpreadsheetDialog::~ExportSpreadsheetDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	conf.writeEntry("Format", ui.cbFormat->currentIndex());
	conf.writeEntry("Header", ui.chkExportHeader->isChecked());
	conf.writeEntry("Separator", ui.cbSeparator->currentIndex());
	conf.writeEntry("ShowOptions", m_showOptions);
	saveDialogSize(conf);
}

void ExportSpreadsheetDialog::setFileName(const QString& name){
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	QString dir = conf.readEntry("LastDir", "");
	if (dir.isEmpty()) dir = QDir::homePath();
	ui.kleFileName->setText(dir + QDir::separator() +  name);
	this->formatChanged(ui.cbFormat->currentIndex());
}

void ExportSpreadsheetDialog::setMatrixMode(bool b) {
	if (b) {
		ui.lExportHeader->hide();
		ui.chkExportHeader->hide();
	}
}

QString ExportSpreadsheetDialog::path() const{
	return ui.kleFileName->text();
}

bool ExportSpreadsheetDialog::exportHeader() const {
	return ui.chkExportHeader->isChecked();
}

QString ExportSpreadsheetDialog::separator() const {
	return ui.cbSeparator->currentText();
}

void ExportSpreadsheetDialog::slotButtonClicked(int button) {
	if (button == KDialog::Ok)
		okClicked();
	else
		KDialog::slotButtonClicked(button);
}

//SLOTS
void ExportSpreadsheetDialog::okClicked(){
	if ( QFile::exists(ui.kleFileName->text()) ){
		int r=KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
		if (r==KMessageBox::No) {
			return;
		}
	}

    KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
    conf.writeEntry("Format", ui.cbFormat->currentIndex());
	conf.writeEntry("Header", ui.chkExportHeader->isChecked());
	conf.writeEntry("Separator", ui.cbSeparator->currentText());

    QString path = ui.kleFileName->text();
    if (!path.isEmpty()) {
		QString dir = conf.readEntry("LastDir", "");
		ui.kleFileName->setText(path);
		int pos = path.lastIndexOf(QDir::separator());
		if (pos!=-1) {
			QString newDir = path.left(pos);
			if (newDir!=dir)
				conf.writeEntry("LastDir", newDir);
		}
	}

	accept();
}

/*!
	Shows/hides the GroupBox with export options in this dialog.
*/
void ExportSpreadsheetDialog::toggleOptions() {
	m_showOptions = !m_showOptions;
	ui.gbOptions->setVisible(m_showOptions);
	m_showOptions ? setButtonText(KDialog::User1, i18n("Hide Options")) : setButtonText(KDialog::User1, i18n("Show Options"));
	//resize the dialog
	mainWidget->resize(layout()->minimumSize());
	layout()->activate();
 	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportSpreadsheetDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	QString dir = conf.readEntry("LastDir", "");
    QString path = QFileDialog::getOpenFileName(this, i18n("Export to file"), dir);
    if (!path.isEmpty()) {
		ui.kleFileName->setText(path);

		int pos = path.lastIndexOf(QDir::separator());
		if (pos!=-1) {
			QString newDir = path.left(pos);
			if (newDir!=dir)
				conf.writeEntry("LastDir", newDir);
		}
	}
}

/*!
	called when the output format was changed. Adjusts the extension for the specified file.
 */
void ExportSpreadsheetDialog::formatChanged(int index){
	QStringList extensions;
	extensions<<".txt"<<".bin";
	QString path = ui.kleFileName->text();
	int i = path.indexOf(".");
	if (i==-1)
		path = path + extensions.at(index);
	else
		path=path.left(i) + extensions.at(index);

	ui.kleFileName->setText(path);
}

void ExportSpreadsheetDialog::fileNameChanged(const QString& name) {
	enableButtonOk( !name.simplified().isEmpty() );
}
