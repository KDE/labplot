/*
	File                 : ExportWorksheetDialog.cpp
	Project              : LabPlot
	Description          : export worksheet dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ExportWorksheetDialog.h"
#include "backend/core/Settings.h"
#include "frontend/GuiTools.h"
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

	ui->cbExportArea->addItem(i18n("Bounding Box"), static_cast<int>(Worksheet::ExportArea::BoundingBox));
	ui->cbExportArea->addItem(i18n("Selection"), static_cast<int>(Worksheet::ExportArea::Selection));
	ui->cbExportArea->addItem(i18n("Worksheet"), static_cast<int>(Worksheet::ExportArea::Worksheet));

	QString info = i18n(
		"Specifies the area to export:"
		"<ul>"
		"<li>Bounding Box - the bounding box of all objects on the worksheet is exported</li>"
		"<li>Selection - the area of the currently selected objects is exported</li>"
		"<li>Worksheet - the complete worksheet area is exported</li>"
		"</ul>"
	);
	ui->cbExportArea->setToolTip(info);

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
	ui->cbExportArea->setCurrentIndex(conf.readEntry("Area", 1));
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
	return static_cast<Worksheet::ExportFormat>(ui->cbFormat->currentData().toInt());
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

//////////////////////
QString ExportWorksheetDialog::formatExtension(Worksheet::ExportFormat format) {
	switch (format) {
	case Worksheet::ExportFormat::PDF:
		return QStringLiteral(".pdf");
	case Worksheet::ExportFormat::SVG:
		return QStringLiteral(".svg");
	case Worksheet::ExportFormat::PNG:
		return QStringLiteral(".png");
	case Worksheet::ExportFormat::JPG:
		return QStringLiteral(".jpg");
	case Worksheet::ExportFormat::BMP:
		return QStringLiteral(".bmp");
	case Worksheet::ExportFormat::PPM:
		return QStringLiteral(".ppm");
	case Worksheet::ExportFormat::XBM:
		return QStringLiteral(".xbm");
	case Worksheet::ExportFormat::XPM:
		return QStringLiteral(".xpm");
	}

	return {};
}

QString ExportWorksheetDialog::formatCaption(Worksheet::ExportFormat format) {
	switch (format) {
	case Worksheet::ExportFormat::PDF:
		return i18n("Portable Data Format (*.pdf *.PDF)");
	case Worksheet::ExportFormat::SVG:
		return i18n("Scalable Vector Graphics (*.svg *.SVG)");
	case Worksheet::ExportFormat::PNG:
		return i18n("Portable Network Graphics (*.png *.PNG)");
	case Worksheet::ExportFormat::JPG:
		return i18n("Joint Photographic Experts Group (*.jpg *.jpeg *.JPG *.JPEG)");
	case Worksheet::ExportFormat::BMP:
		return i18n("Windows Bitmap (*.bmp *.BMP)");
	case Worksheet::ExportFormat::PPM:
		return i18n("Portable Pixmap (*.ppm *.PPM)");
	case Worksheet::ExportFormat::XBM:
		return i18n("X11 Bitmap (*.xbm *.XBM)");
	case Worksheet::ExportFormat::XPM:
		return i18n("X11 Bitmap (*.xpm *.XPM)");
	}

	return {};
}

// SLOTS
void ExportWorksheetDialog::okClicked() {
	const bool exportToFile = (ui->cbExportTo->currentIndex() == 0);

	if (exportToFile) {
		QString filename = ui->leFileName->text();

		if (m_askOverwrite && QFile::exists(filename)) {
			int status = KMessageBox::questionTwoActions(this,
						 i18n("The file already exists. Do you really want to overwrite it?"),
						 i18n("Export"),
						 KStandardGuiItem::overwrite(),
						 KStandardGuiItem::cancel());
			if (status == KMessageBox::SecondaryAction)
				return;
		}

		if (!filename.isEmpty()) {
			auto conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
			QString dir = conf.readEntry("LastDir", "");
			int pos = filename.lastIndexOf(QLatin1String("/"));
			if (pos != -1) {
				QString newDir = filename.left(pos);
				if (newDir != dir)
					conf.writeEntry("LastDir", newDir);
			}
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
	auto conf = Settings::group(QStringLiteral("ExportWorksheetDialog"));
	const QString dir = conf.readEntry("LastDir", "");

	const auto format = Worksheet::ExportFormat(ui->cbFormat->currentData().toInt());
	QString path = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Export to File"), dir, formatCaption(format));
	if (!path.isEmpty()) {
		// if the file already exists, the user was already asked
		// in QFileDialog whether to overwrite or not.
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
	const auto& path = ui->leFileName->text();
	if (!path.isEmpty())
		ui->leFileName->setText(GuiTools::replaceExtension(path, formatExtension(format)));
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
	QString filename = ui->leFileName->text();
	int pos = filename.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString dir = filename.left(pos);
		bool invalid = !QDir(dir).exists();
		GuiTools::highlight(ui->leFileName, invalid);
		if (invalid) {
			m_okButton->setEnabled(false);
			return;
		}
	}

	// append file extension if not present
	bool changed = false;
	const auto format = Worksheet::ExportFormat(ui->cbFormat->currentData().toInt());
	if (format == Worksheet::ExportFormat::JPG) { // can be .jpg (default) or .jpeg
		if (! (filename.endsWith(formatExtension(format), Qt::CaseInsensitive) || filename.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive))) {
			filename.append(formatExtension(format));
			changed = true;
		}
	} else { // all other formats have a single ending
		if (! filename.endsWith(formatExtension(format), Qt::CaseInsensitive)) {
			filename.append(formatExtension(format));
			changed = true;
		}
	}
	if (changed)
		ui->leFileName->setText(filename);

	m_askOverwrite = true;
	m_okButton->setEnabled(true);
}
