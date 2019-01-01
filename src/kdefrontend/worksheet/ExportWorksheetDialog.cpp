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
#include "ui_exportworksheetwidget.h"

#include <QCompleter>
#include <QDesktopWidget>
#include <QDirModel>
#include <QFileDialog>
#include <QTimer>

#include <QDialogButtonBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ExportWorksheetDialog
	\brief Dialog for exporting a worksheet to a file.

	\ingroup kdefrontend
*/

ExportWorksheetDialog::ExportWorksheetDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ExportWorksheetWidget()) {
	ui->setupUi(this);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_showOptionsButton = new QPushButton;

	connect(btnBox, &QDialogButtonBox::clicked, this, &ExportWorksheetDialog::slotButtonClicked);

	btnBox->addButton(m_showOptionsButton, QDialogButtonBox::ActionRole);
	ui->verticalLayout->addWidget(btnBox);

	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_cancelButton = btnBox->button(QDialogButtonBox::Cancel);
	auto* completer = new QCompleter(this);
	completer->setModel(new QDirModel);
	ui->leFileName->setCompleter(completer);

	ui->bOpen->setIcon(QIcon::fromTheme(QLatin1String("document-open")));

	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("application-pdf")), QLatin1String("Portable Data Format (PDF)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-svg+xml")), QLatin1String("Scalable Vector Graphics (SVG)"));
	ui->cbFormat->insertSeparator(3);
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), QLatin1String("Portable Network Graphics (PNG)"));

	ui->cbExportArea->addItem(i18n("Object's bounding box"));
	ui->cbExportArea->addItem(i18n("Current selection"));
	ui->cbExportArea->addItem(i18n("Complete worksheet"));

	ui->cbResolution->addItem(i18nc("%1 is the value of DPI of the current screen", "%1 (desktop)", QString::number(QApplication::desktop()->physicalDpiX())));
	ui->cbResolution->addItem(QLatin1String("100"));
	ui->cbResolution->addItem(QLatin1String("150"));
	ui->cbResolution->addItem(QLatin1String("200"));
	ui->cbResolution->addItem(QLatin1String("300"));
	ui->cbResolution->addItem(QLatin1String("600"));
	ui->cbResolution->setValidator(new QIntValidator(ui->cbResolution));

	connect(ui->cbFormat, static_cast<void (QComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ExportWorksheetDialog::formatChanged );
	connect(ui->bOpen, &QPushButton::clicked, this, &ExportWorksheetDialog::selectFile);
	connect(ui->leFileName, &QLineEdit::textChanged, this, &ExportWorksheetDialog::fileNameChanged);
	connect(m_showOptionsButton, &QPushButton::clicked, this, &ExportWorksheetDialog::toggleOptions);
	ui->leFileName->setFocus();

	setWindowTitle(i18nc("@title:window", "Export Worksheet"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("document-export-database")));

	QTimer::singleShot(0, this, &ExportWorksheetDialog::loadSettings);
}

void ExportWorksheetDialog::loadSettings() {
	//restore saved setting
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");

	KWindowConfig::restoreWindowSize(windowHandle(), conf);

	ui->cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui->cbExportArea->setCurrentIndex(conf.readEntry("Area", 0));
	ui->chkExportBackground->setChecked(conf.readEntry("Background", true));
	ui->cbResolution->setCurrentIndex(conf.readEntry("Resolution", 0));
	m_showOptions = conf.readEntry("ShowOptions", true);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) :
				m_showOptionsButton->setText(i18n("Show Options"));
	ui->gbOptions->setVisible(m_showOptions);
}

ExportWorksheetDialog::~ExportWorksheetDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("Area", ui->cbExportArea->currentIndex());
	conf.writeEntry("Background", ui->chkExportBackground->isChecked());
	conf.writeEntry("Resolution", ui->cbResolution->currentIndex());
	conf.writeEntry("ShowOptions", m_showOptions);

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void ExportWorksheetDialog::setFileName(const QString& name) {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	QString dir = conf.readEntry("LastDir", "");
	if (dir.isEmpty()) dir = QDir::homePath();
	ui->leFileName->setText(dir + QDir::separator() +  name);
	this->formatChanged(ui->cbFormat->currentIndex());
}

QString ExportWorksheetDialog::path() const {
	return ui->leFileName->text();
}

WorksheetView::ExportFormat ExportWorksheetDialog::exportFormat() const {
	int index = ui->cbFormat->currentIndex();

	//we have a separator in the format combobox at the 3th position -> skip it
	if (index > 2)
		index--;

	return WorksheetView::ExportFormat(index);
}

WorksheetView::ExportArea ExportWorksheetDialog::exportArea() const {
	return WorksheetView::ExportArea(ui->cbExportArea->currentIndex());
}

bool ExportWorksheetDialog::exportBackground() const {
	return ui->chkExportBackground->isChecked();
}

int ExportWorksheetDialog::exportResolution() const {
	if (ui->cbResolution->currentIndex() == 0)
		return QApplication::desktop()->physicalDpiX();
	else
		return ui->cbResolution->currentText().toInt();
}

void ExportWorksheetDialog::slotButtonClicked(QAbstractButton* button) {
	if (button == m_okButton)
		okClicked();
	else if (button == m_cancelButton) {
	    reject();
	}
}

//SLOTS
void ExportWorksheetDialog::okClicked() {
	if ( QFile::exists(ui->leFileName->text()) ) {
		int r = KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
		if (r == KMessageBox::No)
			return;
	}

	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("Area", ui->cbExportArea->currentIndex());
	conf.writeEntry("Resolution", ui->cbResolution->currentIndex());

	QString path = ui->leFileName->text();
	if (!path.isEmpty()) {
		QString dir = conf.readEntry("LastDir", "");
		ui->leFileName->setText(path);
		int pos = path.lastIndexOf(QDir::separator());
		if (pos != -1) {
			QString newDir = path.left(pos);
			if (newDir != dir)
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
	ui->gbOptions->setVisible(m_showOptions);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) :
			m_showOptionsButton->setText(i18n("Show Options"));
	//resize the dialog
	resize(layout()->minimumSize());
	layout()->activate();
	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportWorksheetDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	const QString dir = conf.readEntry("LastDir", "");

	QString format;
	if (ui->cbFormat->currentIndex() == 0)
		format = i18n("Portable Data Format (PDF) (*.pdf *.PDF)");
	else if (ui->cbFormat->currentIndex() == 1)
		format = i18n("Scalable Vector Graphics (SVG) (*.svg *.SVG)");
	else
		format = i18n("Portable Network Graphics (PNG) (*.png *.PNG)");

	const QString path = QFileDialog::getSaveFileName(this, i18n("Export to file"), dir, format);
	if (!path.isEmpty()) {
		ui->leFileName->setText(path);

		int pos = path.lastIndexOf(QDir::separator());
		if (pos != -1) {
			const QString newDir = path.left(pos);
			if (newDir != dir)
				conf.writeEntry("LastDir", newDir);
		}
	}
}

/*!
	called when the output format was changed. Adjusts the extension for the specified file.
 */
void ExportWorksheetDialog::formatChanged(int index) {
	//we have a separator in the format combobox at the 3rd position -> skip it
	if (index > 2)
		index --;

	QStringList extensions;
	extensions << QLatin1String(".pdf") << QLatin1String(".svg") << QLatin1String(".png");
	QString path = ui->leFileName->text();
	int i = path.indexOf(QLatin1Char('.'));
	if (i == -1)
		path = path + extensions.at(index);
	else
		path = path.left(i) + extensions.at(index);

	ui->leFileName->setText(path);

	// show resolution option for png format
	ui->lResolution->setVisible(index == 3);
	ui->cbResolution->setVisible(index == 3);
}

void ExportWorksheetDialog::fileNameChanged(const QString& name) {
	m_okButton->setEnabled(!name.simplified().isEmpty());
}
