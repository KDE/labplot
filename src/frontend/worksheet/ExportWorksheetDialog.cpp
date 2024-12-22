/*
	File                 : ExportWorksheetDialog.cpp
	Project              : LabPlot
	Description          : export worksheet dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExportWorksheetDialog.h"
#include "backend/core/Settings.h"
#include "frontend/GuiTools.h"
#include "frontend/worksheet/WorksheetView.h"
#include "ui_exportworksheetwidget.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <KWindowConfig>
#include <kcoreaddons_version.h>

#include <QCompleter>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QScreen>
#include <QWindow>

/*!
	\class ExportWorksheetDialog
	\brief Dialog for exporting a worksheet to a file.

	\ingroup frontend
*/

ExportWorksheetDialog::ExportWorksheetDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::ExportWorksheetWidget()) {
	ui->setupUi(this);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_showOptionsButton = new QPushButton;

	connect(btnBox, &QDialogButtonBox::clicked, this, &ExportWorksheetDialog::slotButtonClicked);

	btnBox->addButton(m_showOptionsButton, QDialogButtonBox::ActionRole);
	ui->verticalLayout->addWidget(btnBox);

	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_cancelButton = btnBox->button(QDialogButtonBox::Cancel);

	m_cancelButton->setToolTip(i18n("Close this dialog without exporting."));

	ui->leFileName->setCompleter(new QCompleter(new QFileSystemModel, this));

	ui->bOpen->setIcon(QIcon::fromTheme(QLatin1String("document-open")));

	// see Worksheet::ExportFormat
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("application-pdf")),
						  i18n("Portable Data Format (PDF)"),
						  static_cast<int>(Worksheet::ExportFormat::PDF));
#ifdef HAVE_QTSVG
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-svg+xml")),
						  i18n("Scalable Vector Graphics (SVG)"),
						  static_cast<int>(Worksheet::ExportFormat::SVG));
#endif
	ui->cbFormat->insertSeparator(3);
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-png")),
						  i18n("Portable Network Graphics (PNG)"),
						  static_cast<int>(Worksheet::ExportFormat::PNG));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-jpeg")),
						  i18n("Joint Photographic Experts Group (JPG)"),
						  static_cast<int>(Worksheet::ExportFormat::JPG));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-bmp")), i18n("Windows Bitmap (BMP)"), static_cast<int>(Worksheet::ExportFormat::BMP));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")),
						  i18n("Portable Pixmap (PPM)"),
						  static_cast<int>(Worksheet::ExportFormat::PPM));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("X11 Bitmap (XBM)"), static_cast<int>(Worksheet::ExportFormat::XBM));
	ui->cbFormat->addItem(QIcon::fromTheme(QLatin1String("image-x-generic")), i18n("X11 Bitmap (XPM)"), static_cast<int>(Worksheet::ExportFormat::XPM));

	ui->cbExportTo->addItem(i18n("File"));
	ui->cbExportTo->addItem(i18n("Clipboard"));

	ui->cbExportArea->addItem(i18n("Object's Bounding Box"), static_cast<int>(Worksheet::ExportArea::BoundingBox));
	ui->cbExportArea->addItem(i18n("Current Selection"), static_cast<int>(Worksheet::ExportArea::Selection));
	ui->cbExportArea->addItem(i18n("Complete Worksheet"), static_cast<int>(Worksheet::ExportArea::Worksheet));

	ui->cbResolution->addItem(
		i18nc("%1 is the value of DPI of the current screen", "%1 (desktop)", QString::number(GuiTools::dpi(this).first)));
	ui->cbResolution->addItem(QLatin1String("100"));
	ui->cbResolution->addItem(QLatin1String("150"));
	ui->cbResolution->addItem(QLatin1String("200"));
	ui->cbResolution->addItem(QLatin1String("300"));
	ui->cbResolution->addItem(QLatin1String("600"));
	ui->cbResolution->setValidator(new QIntValidator(ui->cbResolution));

	connect(ui->cbFormat, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &ExportWorksheetDialog::formatChanged);
	connect(ui->cbExportTo, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &ExportWorksheetDialog::exportToChanged);
	connect(ui->bOpen, &QPushButton::clicked, this, &ExportWorksheetDialog::selectFile);
	connect(ui->leFileName, &QLineEdit::textChanged, this, &ExportWorksheetDialog::fileNameChanged);
	connect(m_showOptionsButton, &QPushButton::clicked, this, &ExportWorksheetDialog::toggleOptions);
	ui->leFileName->setFocus();

	setWindowTitle(i18nc("@title:window", "Export Worksheet"));
	setWindowIcon(QIcon::fromTheme(QLatin1String("document-export-database")));

	// restore saved settings if available
	KConfigGroup conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
	ui->cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui->cbExportTo->setCurrentIndex(conf.readEntry("ExportTo", 0));
	ui->cbExportArea->setCurrentIndex(conf.readEntry("Area", 0));
	ui->chkExportBackground->setChecked(conf.readEntry("Background", true));
	ui->cbResolution->setCurrentIndex(conf.readEntry("Resolution", 0));
	m_showOptions = conf.readEntry("ShowOptions", true);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) : m_showOptionsButton->setText(i18n("Show Options"));
	ui->gbOptions->setVisible(m_showOptions);

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ExportWorksheetDialog::~ExportWorksheetDialog() {
	// save current settings
	KConfigGroup conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("ExportTo", ui->cbExportTo->currentIndex());
	conf.writeEntry("Area", ui->cbExportArea->currentIndex());
	conf.writeEntry("Background", ui->chkExportBackground->isChecked());
	conf.writeEntry("Resolution", ui->cbResolution->currentIndex());
	conf.writeEntry("ShowOptions", m_showOptions);
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	delete ui;
}

/*!
 * sets the current project file name. If not empty, the path of the project file
 * is determined that is then used as the default location for the exported file.
 */
void ExportWorksheetDialog::setProjectFileName(const QString& name) {
	if (name.isEmpty())
		return;

	m_projectPath = QFileInfo(name).canonicalPath();
}

void ExportWorksheetDialog::setFileName(const QString& name) {
	if (m_projectPath.isEmpty()) {
		// no project folder is available (yet), use the last used directory in this dialog
		KConfigGroup conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
		QString dir = conf.readEntry("LastDir", "");
		if (dir.isEmpty())
			dir = QDir::homePath();
		ui->leFileName->setText(dir + QLatin1String("/") + name);
	} else
		ui->leFileName->setText(m_projectPath + QLatin1String("/") + name);

	formatChanged(ui->cbFormat->currentIndex());
	exportToChanged(ui->cbExportTo->currentIndex());
}

QString ExportWorksheetDialog::path() const {
	if (ui->cbExportTo->currentIndex() == 0)
		return ui->leFileName->text();
	return {};
}

Worksheet::ExportFormat ExportWorksheetDialog::exportFormat() const {
	int index = ui->cbFormat->currentIndex();

	// we have a separator in the format combobox at the 3th position -> skip it
	if (index > 2)
		index--;

	return Worksheet::ExportFormat(index);
}

Worksheet::ExportArea ExportWorksheetDialog::exportArea() const {
	return Worksheet::ExportArea(ui->cbExportArea->currentData().toInt());
}

bool ExportWorksheetDialog::exportBackground() const {
	return ui->chkExportBackground->isChecked();
}

int ExportWorksheetDialog::exportResolution() const {
	if (ui->cbResolution->currentIndex() == 0)
		return GuiTools::dpi(this).first;
	else
		return ui->cbResolution->currentText().toInt();
}

void ExportWorksheetDialog::slotButtonClicked(QAbstractButton* button) {
	if (button == m_okButton)
		okClicked();
	else if (button == m_cancelButton)
		reject();
}

// SLOTS
void ExportWorksheetDialog::okClicked() {
	if (ui->cbExportTo->currentIndex() == 0 /*export to file*/
		&& m_askOverwrite && QFile::exists(ui->leFileName->text())) {
		int status = KMessageBox::questionTwoActions(this,
													 i18n("The file already exists. Do you really want to overwrite it?"),
													 i18n("Export"),
													 KStandardGuiItem::overwrite(),
													 KStandardGuiItem::cancel());
		if (status == KMessageBox::SecondaryAction)
			return;
	}

	KConfigGroup conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
	const auto& path = ui->leFileName->text();
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
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) : m_showOptionsButton->setText(i18n("Show Options"));

	// resize the dialog
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportWorksheetDialog::selectFile() {
	KConfigGroup conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
	const QString dir = conf.readEntry("LastDir", "");

	const auto format = Worksheet::ExportFormat(ui->cbFormat->currentData().toInt());
	QString caption;
	switch (format) {
	case Worksheet::ExportFormat::PDF:
		caption = i18n("Portable Data Format (*.pdf *.PDF)");
		break;
	case Worksheet::ExportFormat::SVG:
		caption = i18n("Scalable Vector Graphics (*.svg *.SVG)");
		break;
	case Worksheet::ExportFormat::PNG:
		caption = i18n("Portable Network Graphics (*.png *.PNG)");
		break;
	case Worksheet::ExportFormat::JPG:
		caption = i18n("Joint Photographic Experts Group (*.jpg *.jpeg *.JPG *.JPEG)");
		break;
	case Worksheet::ExportFormat::BMP:
		caption = i18n("Windows Bitmap (*.bmp *.BMP)");
		break;
	case Worksheet::ExportFormat::PPM:
		caption = i18n("Portable Pixmap (*.ppm *.PPM)");
		break;
	case Worksheet::ExportFormat::XBM:
		caption = i18n("X11 Bitmap (*.xbm *.XBM)");
		break;
	case Worksheet::ExportFormat::XPM:
		caption = i18n("X11 Bitmap (*.xpm *.XPM)");
		break;
	}

	const QString path = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Export to File"), dir, caption);
	if (!path.isEmpty()) {
		// if the file is already existing, the user was already asked
		// in QFileDialog whether to overwrite or not.
		// Don't ask again when the user click on Ok-button.
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
void ExportWorksheetDialog::formatChanged(int) {
	const auto format = Worksheet::ExportFormat(ui->cbFormat->currentData().toInt());

	// show resolution option for png format
	const bool visible = (format == Worksheet::ExportFormat::PNG);
	ui->lResolution->setVisible(visible);
	ui->cbResolution->setVisible(visible);

	// add/replace the file extension for the current file format
	QString extension;
	switch (format) {
	case Worksheet::ExportFormat::PDF:
		extension = QStringLiteral(".pdf");
		break;
	case Worksheet::ExportFormat::SVG:
		extension = QStringLiteral(".svg");
		break;
	case Worksheet::ExportFormat::PNG:
		extension = QStringLiteral(".png");
		break;
	case Worksheet::ExportFormat::JPG:
		extension = QStringLiteral(".jpg");
		break;
	case Worksheet::ExportFormat::BMP:
		extension = QStringLiteral(".bmp");
		break;
	case Worksheet::ExportFormat::PPM:
		extension = QStringLiteral(".ppm");
		break;
	case Worksheet::ExportFormat::XBM:
		extension = QStringLiteral(".xbm");
		break;
	case Worksheet::ExportFormat::XPM:
		extension = QStringLiteral(".xpm");
		break;
	}

	const auto& path = ui->leFileName->text();
	if (!path.isEmpty())
		ui->leFileName->setText(GuiTools::replaceExtension(path, extension));
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
		fileNameChanged(ui->leFileName->text()); // call this to check whether a valid file name was provided
	} else {
		m_okButton->setToolTip(i18n("Export to clipboard and close the dialog."));
		m_okButton->setEnabled(true);
	}
}

void ExportWorksheetDialog::fileNameChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;

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
