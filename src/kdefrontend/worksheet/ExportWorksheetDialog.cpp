/***************************************************************************
    File                 : ExportWorksheetDialog.cpp
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2016 by Alexander Semke (alexander.semke@web.de)

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
#include <QDesktopWidget>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDebug>

/*!
	\class ExportWorksheetDialog
	\brief Dialog for exporting a worksheet to a file.

	\ingroup kdefrontend
*/

ExportWorksheetDialog::ExportWorksheetDialog(QWidget* parent) : KDialog(parent) {
	mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleFileName->setCompletionObject(comp);

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );

	ui.cbFormat->addItem(QIcon::fromTheme("application-pdf"), "Portable data format (PDF)");
	ui.cbFormat->addItem(QIcon::fromTheme("image-svg+xml"), "Scalable Vector Graphics (SVG)");
	ui.cbFormat->insertSeparator(3);
	ui.cbFormat->addItem(QIcon::fromTheme("image-x-generic"), "Portable Network Graphics (PNG)");

	ui.cbExportArea->addItem(i18n("Object's bounding box"));
	ui.cbExportArea->addItem(i18n("Current selection"));
	ui.cbExportArea->addItem(i18n("Complete worksheet"));

	ui.cbResolution->addItem(QString::number(QApplication::desktop()->physicalDpiX()) + " (" + i18n("desktop") + ')');
	ui.cbResolution->addItem("100");
	ui.cbResolution->addItem("150");
	ui.cbResolution->addItem("200");
	ui.cbResolution->addItem("300");
	ui.cbResolution->addItem("600");
	ui.cbResolution->setValidator(new QIntValidator(ui.cbResolution));

	setMainWidget(mainWidget);

	setButtons(KDialog::Ok | KDialog::User1 | KDialog::Cancel);

	connect( ui.cbFormat, SIGNAL(currentIndexChanged(int)), SLOT(formatChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()) );
	connect( ui.kleFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged(QString)) );
	connect(this,SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));

	setCaption(i18n("Export worksheet"));
	setWindowIcon(QIcon::fromTheme("document-export-database"));

	//restore saved setting
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	ui.cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui.cbExportArea->setCurrentIndex(conf.readEntry("Area", 0));
	ui.chkExportBackground->setChecked(conf.readEntry("Background", true));
	ui.cbResolution->setCurrentIndex(conf.readEntry("Resolution", 0));
	m_showOptions = conf.readEntry("ShowOptions", false);
	ui.gbOptions->setVisible(m_showOptions);
	m_showOptions ? setButtonText(KDialog::User1,i18n("Hide Options")) : setButtonText(KDialog::User1,i18n("Show Options"));
	restoreDialogSize(conf);
}

ExportWorksheetDialog::~ExportWorksheetDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	conf.writeEntry("Format", ui.cbFormat->currentIndex());
	conf.writeEntry("Area", ui.cbExportArea->currentIndex());
	conf.writeEntry("Background", ui.chkExportBackground->isChecked());
	conf.writeEntry("Resolution", ui.cbResolution->currentIndex());
	conf.writeEntry("ShowOptions", m_showOptions);
	saveDialogSize(conf);
}

void ExportWorksheetDialog::setFileName(const QString& name) {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	QString dir = conf.readEntry("LastDir", "");
	if (dir.isEmpty()) dir = QDir::homePath();
	ui.kleFileName->setText(dir + QDir::separator() +  name);
	this->formatChanged(ui.cbFormat->currentIndex());
}

QString ExportWorksheetDialog::path() const{
	return ui.kleFileName->text();
}

WorksheetView::ExportFormat ExportWorksheetDialog::exportFormat() const {
	int index = ui.cbFormat->currentIndex();

	//we have a separator in the format combobox at the 3th position -> skip it
	if (index>2)
		index --;

	return WorksheetView::ExportFormat(index);
}

WorksheetView::ExportArea ExportWorksheetDialog::exportArea() const {
	return WorksheetView::ExportArea(ui.cbExportArea->currentIndex());
}

bool ExportWorksheetDialog::exportBackground() const {
	return ui.chkExportBackground->isChecked();
}

int ExportWorksheetDialog::exportResolution() const {
	if (ui.cbResolution->currentIndex() == 0)
		return QApplication::desktop()->physicalDpiX();
	else
		return ui.cbResolution->currentText().toInt();

}

void ExportWorksheetDialog::slotButtonClicked(int button) {
	if (button == KDialog::Ok)
		okClicked();
	else
		KDialog::slotButtonClicked(button);
}

//SLOTS
void ExportWorksheetDialog::okClicked() {
	if ( QFile::exists(ui.kleFileName->text()) ) {
		int r=KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
		if (r==KMessageBox::No)
			return;
	}

	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	conf.writeEntry("Format", ui.cbFormat->currentIndex());
	conf.writeEntry("Area", ui.cbExportArea->currentIndex());
	conf.writeEntry("Resolution", ui.cbResolution->currentIndex());

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
void ExportWorksheetDialog::toggleOptions() {
	m_showOptions = !m_showOptions;
	ui.gbOptions->setVisible(m_showOptions);
	m_showOptions ? setButtonText(KDialog::User1,i18n("Hide Options")) : setButtonText(KDialog::User1,i18n("Show Options"));

	//resize the dialog
	mainWidget->resize(layout()->minimumSize());
	layout()->activate();
 	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportWorksheetDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
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
void ExportWorksheetDialog::formatChanged(int index) {
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

	// show resolution option for png format
	ui.lResolution->setVisible(index==3);
	ui.cbResolution->setVisible(index==3);
}

void ExportWorksheetDialog::fileNameChanged(const QString& name) {
	enableButtonOk( !name.simplified().isEmpty() );
}
