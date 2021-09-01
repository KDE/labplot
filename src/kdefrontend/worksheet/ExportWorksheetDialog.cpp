/*
    File                 : ExportWorksheetDialog.cpp
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2019 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ExportWorksheetDialog.h"
#include "ui_exportworksheetwidget.h"
#include "kdefrontend/GuiTools.h"
#include "commonfrontend/worksheet/WorksheetView.h"

#include <QCompleter>
#include <QDesktopWidget>
#include <QDirModel>
#include <QFileDialog>
#include <QWindow>

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

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_showOptionsButton = new QPushButton;

	connect(btnBox, &QDialogButtonBox::clicked, this, &ExportWorksheetDialog::slotButtonClicked);

	btnBox->addButton(m_showOptionsButton, QDialogButtonBox::ActionRole);
	ui->verticalLayout->addWidget(btnBox);

	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_cancelButton = btnBox->button(QDialogButtonBox::Cancel);

	m_cancelButton->setToolTip(i18n("Close this dialog without exporting."));

	ui->leFileName->setCompleter(new QCompleter(new QDirModel, this));

	ui->bOpen->setIcon(QIcon::fromTheme(QLatin1String("document-open")));

	// see WorksheetView::ExportFormat
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("application-pdf")), i18n("Portable Data Format (PDF)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-svg+xml")), i18n("Scalable Vector Graphics (SVG)"));
	ui->cbFormat->insertSeparator(3);
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-png")), i18n("Portable Network Graphics (PNG)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-jpeg")), i18n("Joint Photographic Experts Group (JPG)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-bmp")), i18n("Windows Bitmap (BMP)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("Portable Pixmap (PPM)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("X11 Bitmap (XBM)"));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("X11 Bitmap (XPM)"));

	ui->cbExportTo->addItem(i18n("File"));
	ui->cbExportTo->addItem(i18n("Clipboard"));

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

	connect(ui->cbFormat, static_cast<void (QComboBox::*)(int)>(&KComboBox::currentIndexChanged),
			this, &ExportWorksheetDialog::formatChanged);
	connect(ui->cbExportTo, static_cast<void (QComboBox::*)(int)>(&KComboBox::currentIndexChanged),
			this, &ExportWorksheetDialog::exportToChanged);
	connect(ui->bOpen, &QPushButton::clicked, this, &ExportWorksheetDialog::selectFile);
	connect(ui->leFileName, &QLineEdit::textChanged, this, &ExportWorksheetDialog::fileNameChanged);
	connect(m_showOptionsButton, &QPushButton::clicked, this, &ExportWorksheetDialog::toggleOptions);
	ui->leFileName->setFocus();

	setWindowTitle(i18nc("@title:window", "Export Worksheet"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("document-export-database")));

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	ui->cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui->cbExportTo->setCurrentIndex(conf.readEntry("ExportTo", 0));
	ui->cbExportArea->setCurrentIndex(conf.readEntry("Area", 0));
	ui->chkExportBackground->setChecked(conf.readEntry("Background", true));
	ui->cbResolution->setCurrentIndex(conf.readEntry("Resolution", 0));
	m_showOptions = conf.readEntry("ShowOptions", true);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) :
				m_showOptionsButton->setText(i18n("Show Options"));
	ui->gbOptions->setVisible(m_showOptions);

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ExportWorksheetDialog::~ExportWorksheetDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("ExportTo", ui->cbExportTo->currentIndex());
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
	ui->leFileName->setText(dir + QLatin1String("/") +  name);

	formatChanged(ui->cbFormat->currentIndex());
	exportToChanged(ui->cbExportTo->currentIndex());
}

QString ExportWorksheetDialog::path() const {
	if (ui->cbExportTo->currentIndex() == 0)
		return ui->leFileName->text();
	else
		return QString();
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
	else if (button == m_cancelButton)
	    reject();
}

//SLOTS
void ExportWorksheetDialog::okClicked() {
	if ( m_askOverwrite && QFile::exists(ui->leFileName->text()) ) {
		int r = KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
		if (r == KMessageBox::No)
			return;
	}

	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	QString path = ui->leFileName->text();
	if (!path.isEmpty()) {
		QString dir = conf.readEntry("LastDir", "");
		int pos = path.lastIndexOf(QLatin1String("/"));
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
	layout()->activate();
	resize( QSize(this->width(), 0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportWorksheetDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportWorksheetDialog");
	const QString dir = conf.readEntry("LastDir", "");

	DEBUG(Q_FUNC_INFO << ", format" << ui->cbFormat->currentIndex())
	QString format;
	int index = ui->cbFormat->currentIndex();
	if (index > 2)	// consider separator
		index--;

	switch ((WorksheetView::ExportFormat)index) {
	case WorksheetView::ExportFormat::PDF:
		format = i18n("Portable Data Format (*.pdf *.PDF)");
		break;
	case WorksheetView::ExportFormat::SVG:
		format = i18n("Scalable Vector Graphics (*.svg *.SVG)");
		break;
	case WorksheetView::ExportFormat::PNG:
		format = i18n("Portable Network Graphics (*.png *.PNG)");
		break;
	case WorksheetView::ExportFormat::JPG:
		format = i18n("Joint Photographic Experts Group (*.jpg *.jpeg *.JPG *.JPEG)");
		break;
	case WorksheetView::ExportFormat::BMP:
		format = i18n("Windows Bitmap (*.bmp *.BMP)");
		break;
	case WorksheetView::ExportFormat::PPM:
		format = i18n("Portable Pixmap (*.ppm *.PPM)");
		break;
	case WorksheetView::ExportFormat::XBM:
		format = i18n("X11 Bitmap (*.xbm *.XBM)");
		break;
	case WorksheetView::ExportFormat::XPM:
		format = i18n("X11 Bitmap (*.xpm *.XPM)");
		break;
	}

	const QString path = QFileDialog::getSaveFileName(this, i18n("Export to file"), dir, format);
	if (!path.isEmpty()) {
		//if the file is already existing, the user was already asked
		//in QFileDialog whether to overwrite or not.
		//Don't ask again when the user click on Ok-button.
		m_askOverwrite = false;

		m_initializing = true;
		ui->leFileName->setText(path);
		m_initializing = false;

		int pos = path.lastIndexOf(QLatin1String("/"));
		if (pos != -1) {
			const QString newDir = path.left(pos);
			if (newDir != dir && QDir(newDir).exists())
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
	// see WorksheetView::ExportFormat
	extensions << QLatin1String(".pdf") << QLatin1String(".svg") << QLatin1String(".png") << QLatin1String(".jpg") << QLatin1String(".bmp")
		<< QLatin1String(".ppm") << QLatin1String(".xbm") << QLatin1String(".xpm");
	QString path = ui->leFileName->text();
	int i = path.indexOf(QLatin1Char('.'));
	if (i == -1)
		path = path + extensions.at(index);
	else
		path = path.left(i) + extensions.at(index);

	ui->leFileName->setText(path);

	// show resolution option for png format
	bool visible = (index == 2);
	ui->lResolution->setVisible(visible);
	ui->cbResolution->setVisible(visible);
}

/*!
	called when the target destination (file or clipboard) format was changed.
 */
void ExportWorksheetDialog::exportToChanged(int index) {
	bool toFile = (index == 0);
	ui->lFileName->setVisible(toFile);
	ui->leFileName->setVisible(toFile);
	ui->bOpen->setVisible(toFile);

	if (toFile) {
		m_okButton->setToolTip(i18n("Export to file and close the dialog."));
		fileNameChanged(ui->leFileName->text()); //call this to check whether a valid file name was provided
	} else {
		m_okButton->setToolTip(i18n("Export to clipboard and close the dialog."));
		m_okButton->setEnabled(true);
	}
}

void ExportWorksheetDialog::fileNameChanged(const QString& name) {
	if (m_initializing)
		return;

	if (name.simplified().isEmpty()) {
		m_okButton->setEnabled(false);
		return;
	}
	QString path = ui->leFileName->text();
	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString dir = path.left(pos);
		bool invalid = !QDir(dir).exists();
		GuiTools::highlight(ui->leFileName, invalid);
		if (invalid) {
			m_okButton->setEnabled(false);
			return;
		}
	}

	m_askOverwrite = true;
	m_okButton->setEnabled(true);
}
