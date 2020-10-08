/***************************************************************************
    File                 : ExportSpreadsheetDialog.cpp
    Project              : LabPlot
    Description          : export spreadsheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2019 by Alexander Semke (alexander.semke@web.de)

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
#include "ui_exportspreadsheetwidget.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "kdefrontend/GuiTools.h"

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QSqlDatabase>
#include <QWindow>

#include <KMessageBox>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ExportSpreadsheetDialog
	\brief Dialog for exporting a spreadsheet to a file.

	\ingroup kdefrontend
*/
ExportSpreadsheetDialog::ExportSpreadsheetDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ExportSpreadsheetWidget()) {
	ui->setupUi(this);

	ui->gbOptions->hide();

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_showOptionsButton = new QPushButton;

	connect(btnBox, &QDialogButtonBox::clicked, this, &ExportSpreadsheetDialog::slotButtonClicked);

	btnBox->addButton(m_showOptionsButton, QDialogButtonBox::ActionRole);
	ui->verticalLayout->addWidget(btnBox);

	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_cancelButton = btnBox->button(QDialogButtonBox::Cancel);

	ui->leFileName->setCompleter(new QCompleter(new QDirModel, this));

	ui->cbFormat->addItem("ASCII", static_cast<int>(Format::ASCII));
	ui->cbFormat->addItem("Binary", static_cast<int>(Format::Binary));
	ui->cbFormat->addItem("LaTeX", static_cast<int>(Format::LaTeX));
	ui->cbFormat->addItem("FITS", static_cast<int>(Format::FITS));

	const QStringList& drivers = QSqlDatabase::drivers();
	if (drivers.contains(QLatin1String("QSQLITE")) || drivers.contains(QLatin1String("QSQLITE3")))
		ui->cbFormat->addItem("SQLite", static_cast<int>(Format::SQLite));

	QStringList separators = AsciiFilter::separatorCharacters();
	separators.takeAt(0); //remove the first entry "auto"
	ui->cbSeparator->addItems(separators);

	//TODO: use general setting for decimal separator?
	ui->cbDecimalSeparator->addItem(i18n("Point '.'"));
	ui->cbDecimalSeparator->addItem(i18n("Comma ','"));

	ui->cbLaTeXExport->addItem(i18n("Export Spreadsheet"));
	ui->cbLaTeXExport->addItem(i18n("Export Selection"));

	ui->bOpen->setIcon( QIcon::fromTheme("document-open") );

	ui->leFileName->setFocus();

	const QString textNumberFormatShort = i18n("This option determines how the convert numbers to strings.");
	ui->lDecimalSeparator->setToolTip(textNumberFormatShort);
	ui->lDecimalSeparator->setToolTip(textNumberFormatShort);

	connect(ui->bOpen, &QPushButton::clicked, this, &ExportSpreadsheetDialog::selectFile);
	connect(ui->leFileName, &QLineEdit::textChanged, this, &ExportSpreadsheetDialog::fileNameChanged );
	connect(m_showOptionsButton, &QPushButton::clicked, this, &ExportSpreadsheetDialog::toggleOptions);
	connect(ui->cbFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ExportSpreadsheetDialog::formatChanged);
	connect(ui->cbExportToFITS, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ExportSpreadsheetDialog::fitsExportToChanged);

	setWindowTitle(i18nc("@title:window", "Export Spreadsheet"));
	setWindowIcon(QIcon::fromTheme("document-export-database"));

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), conf);
	ui->cbFormat->setCurrentIndex(conf.readEntry("Format", 0));
	ui->chkExportHeader->setChecked(conf.readEntry("Header", true));
	ui->cbSeparator->setCurrentItem(conf.readEntry("Separator", "TAB"));

	//TODO: use general setting for decimal separator?
	const QChar decimalSeparator = QLocale().decimalPoint();
	int index = (decimalSeparator == '.') ? 0 : 1;
	ui->cbDecimalSeparator->setCurrentIndex(conf.readEntry("DecimalSeparator", index));

	ui->chkHeaders->setChecked(conf.readEntry("LaTeXHeaders", true));
	ui->chkGridLines->setChecked(conf.readEntry("LaTeXGridLines", true));
	ui->chkCaptions->setChecked(conf.readEntry("LaTeXCaptions", true));
	ui->chkEmptyRows->setChecked(conf.readEntry("LaTeXSkipEmpty", false));
	ui->cbLaTeXExport->setCurrentIndex(conf.readEntry("ExportOnly", 0));
	ui->chkMatrixHHeader->setChecked(conf.readEntry("MatrixHorizontalHeader", true));
	ui->chkMatrixVHeader->setChecked(conf.readEntry("MatrixVerticalHeader", true));
	ui->chkMatrixVHeader->setChecked(conf.readEntry("FITSSpreadsheetColumnsUnits", true));
	ui->cbExportToFITS->setCurrentIndex(conf.readEntry("FITSTo", 0));
	m_showOptions = conf.readEntry("ShowOptions", false);
	ui->gbOptions->setVisible(m_showOptions);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) :
			m_showOptionsButton->setText(i18n("Show Options"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ExportSpreadsheetDialog::~ExportSpreadsheetDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("Header", ui->chkExportHeader->isChecked());
	conf.writeEntry("Separator", ui->cbSeparator->currentText());
	conf.writeEntry("DecimalSeparator", ui->cbDecimalSeparator->currentIndex());
	conf.writeEntry("ShowOptions", m_showOptions);
	conf.writeEntry("LaTeXHeaders", ui->chkHeaders->isChecked());
	conf.writeEntry("LaTeXGridLines", ui->chkGridLines->isChecked());
	conf.writeEntry("LaTeXCaptions", ui->chkCaptions->isChecked());
	conf.writeEntry("LaTeXSkipEmpty", ui->chkEmptyRows->isChecked());
	conf.writeEntry("ExportOnly", ui->cbLaTeXExport->currentIndex());
	conf.writeEntry("MatrixVerticalHeader", ui->chkMatrixVHeader->isChecked());
	conf.writeEntry("MatrixHorizontalHeader", ui->chkMatrixHHeader->isChecked());
	conf.writeEntry("FITSTo", ui->cbExportToFITS->currentIndex());
	conf.writeEntry("FITSSpreadsheetColumnsUnits", ui->chkColumnsAsUnits->isChecked());

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void ExportSpreadsheetDialog::setFileName(const QString& name) {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	QString dir = conf.readEntry("LastDir", "");
	if (dir.isEmpty()) {	// use project dir as fallback
		KConfigGroup conf2(KSharedConfig::openConfig(), "MainWin");
		dir = conf2.readEntry("LastOpenDir", "");
		if (dir.isEmpty())
			dir = QDir::homePath();
	}
	ui->leFileName->setText(dir + QLatin1Char('/') + name);
	this->formatChanged(ui->cbFormat->currentIndex());
}

void ExportSpreadsheetDialog::fitsExportToChanged(int idx) {
	if (idx == 0) {
		ui->chkColumnsAsUnits->hide();
		ui->lColumnAsUnits->hide();
	} else {
		if (!m_matrixMode) {
			ui->chkColumnsAsUnits->show();
			ui->lColumnAsUnits->show();
		}
	}
}

void ExportSpreadsheetDialog::setMatrixMode(bool b) {
	if (b) {
		setWindowTitle(i18nc("@title:window", "Export Matrix"));
		ui->lExportHeader->hide();
		ui->chkExportHeader->hide();
		ui->lEmptyRows->hide();
		ui->chkEmptyRows->hide();
		if (ui->cbFormat->currentIndex() != 3) {
			ui->chkMatrixHHeader->show();
			ui->chkMatrixVHeader->show();
			ui->lMatrixHHeader->show();
			ui->lMatrixVHeader->show();
		}

		ui->lHeader->hide();
		ui->chkHeaders->hide();
		ui->cbLaTeXExport->setItemText(0,i18n("Export matrix"));
		ui->cbExportToFITS->setCurrentIndex(0);

		ui->lColumnAsUnits->hide();
		ui->chkColumnsAsUnits->hide();

		m_matrixMode = b;
	}
}

QString ExportSpreadsheetDialog::path() const {
	return ui->leFileName->text();
}

int ExportSpreadsheetDialog::exportToFits() const {
	return ui->cbExportToFITS->currentIndex();
}

bool ExportSpreadsheetDialog::exportHeader() const {
	return ui->chkExportHeader->isChecked();
}

bool ExportSpreadsheetDialog::captions() const {
	return ui->chkCaptions->isChecked();
}

bool ExportSpreadsheetDialog::exportLatexHeader() const {
	return ui->chkHeaders->isChecked();
}

bool ExportSpreadsheetDialog::gridLines() const {
	return ui->chkGridLines->isChecked();
}

bool ExportSpreadsheetDialog::skipEmptyRows() const {
	return ui->chkEmptyRows->isChecked();
}

bool ExportSpreadsheetDialog::exportSelection() const {
	return ui->cbLaTeXExport->currentIndex() == 1;
}

bool ExportSpreadsheetDialog::entireSpreadheet() const {
	return ui->cbLaTeXExport->currentIndex() == 0;
}

bool ExportSpreadsheetDialog::matrixHorizontalHeader() const {
	return ui->chkMatrixHHeader->isChecked();
}

bool ExportSpreadsheetDialog::matrixVerticalHeader() const {
	return ui->chkMatrixVHeader->isChecked();
}

bool ExportSpreadsheetDialog::commentsAsUnitsFits() const {
	return ui->chkColumnsAsUnits->isChecked();
}

QString ExportSpreadsheetDialog::separator() const {
	return ui->cbSeparator->currentText();
}

QLocale::Language ExportSpreadsheetDialog::numberFormat() const {
	if (ui->cbDecimalSeparator->currentIndex() == 0)
		return QLocale::Language::C;
	else
		return QLocale::Language::German;
}

void ExportSpreadsheetDialog::slotButtonClicked(QAbstractButton* button) {
	if (button == m_okButton)
		okClicked();
	else if (button == m_cancelButton) {
		reject();
	}
}

void ExportSpreadsheetDialog::setExportToImage(bool possible) {
	if (!possible) {
		ui->cbExportToFITS->setCurrentIndex(1);
		ui->cbExportToFITS->setItemData(0, 0, Qt::UserRole - 1);
	}
}

//SLOTS
void ExportSpreadsheetDialog::okClicked() {
	if (format() != Format::FITS)
		if ( QFile::exists(ui->leFileName->text()) ) {
			int r = KMessageBox::questionYesNo(this, i18n("The file already exists. Do you really want to overwrite it?"), i18n("Export"));
			if (r == KMessageBox::No)
				return;
		}
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	conf.writeEntry("Format", ui->cbFormat->currentIndex());
	conf.writeEntry("Header", ui->chkExportHeader->isChecked());
	conf.writeEntry("Separator", ui->cbSeparator->currentText());

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
void ExportSpreadsheetDialog::toggleOptions() {
	m_showOptions = !m_showOptions;
	ui->gbOptions->setVisible(m_showOptions);
	m_showOptions ? m_showOptionsButton->setText(i18n("Hide Options")) :
			m_showOptionsButton->setText(i18n("Show Options"));	//resize the dialog
	resize(layout()->minimumSize());
	layout()->activate();
	resize( QSize(this->width(),0).expandedTo(minimumSize()) );
}

/*!
	opens a file dialog and lets the user select the file.
*/
void ExportSpreadsheetDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExportSpreadsheetDialog");
	QString dir = conf.readEntry("LastDir", "");

	QString extensions;
	const Format format = (Format)(ui->cbFormat->itemData(ui->cbFormat->currentIndex()).toInt());
	switch (format) {
	case Format::ASCII:
		extensions = i18n("Text files (*.txt *.dat *.csv)");
		break;
	case Format::Binary:
		extensions = i18n("Binary files (*.*)");
		break;
	case Format::LaTeX:
		extensions = i18n("LaTeX files (*.tex)");
		break;
	case Format::FITS:
		extensions = i18n("FITS files (*.fits *.fit *.fts)");
		break;
	case Format::SQLite:
		extensions = i18n("SQLite databases files (*.db *.sqlite *.sdb *.db2 *.sqlite2 *.sdb2 *.db3 *.sqlite3 *.sdb3)");
		break;
	}

	const QString path = QFileDialog::getSaveFileName(this, i18n("Export to file"), dir, extensions);
	if (!path.isEmpty()) {
		ui->leFileName->setText(path);

		int pos = path.lastIndexOf(QLatin1String("/"));
		if (pos != -1) {
			QString newDir = path.left(pos);
			if (newDir != dir)
				conf.writeEntry("LastDir", newDir);
		}
	}
}

/*!
	called when the output format was changed. Adjusts the extension for the specified file.
 */
void ExportSpreadsheetDialog::formatChanged(int index) {
	QStringList extensions;
	extensions << ".txt" << ".bin" << ".tex" << ".fits" << ".db";
	QString path = ui->leFileName->text();
	int i = path.indexOf(".");
	if (index != 1) {
		if (i == -1)
			path = path + extensions.at(index);
		else
			path = path.left(i) + extensions.at(index);
	}

	const Format format = Format(ui->cbFormat->itemData(ui->cbFormat->currentIndex()).toInt());
	if (format == Format::LaTeX) {
		ui->cbSeparator->hide();
		ui->lSeparator->hide();
		ui->lDecimalSeparator->hide();
		ui->cbDecimalSeparator->hide();

		ui->chkCaptions->show();
		ui->chkGridLines->show();
		ui->lExportArea->show();
		ui->lGridLines->show();
		ui->lCaptions->show();
		ui->cbLaTeXExport->show();

		if (!m_matrixMode) {
			ui->lHeader->show();
			ui->chkHeaders->show();
			ui->lEmptyRows->show();
			ui->chkEmptyRows->show();
			ui->lMatrixHHeader->hide();
			ui->lMatrixVHeader->hide();
			ui->chkMatrixHHeader->hide();
			ui->chkMatrixVHeader->hide();
		} else {
			ui->lMatrixHHeader->show();
			ui->lMatrixVHeader->show();
			ui->chkMatrixHHeader->show();
			ui->chkMatrixVHeader->show();
		}
		ui->cbExportToFITS->hide();
		ui->lExportToFITS->hide();
		ui->lColumnAsUnits->hide();
		ui->chkColumnsAsUnits->hide();
	} else if (format == Format::FITS) {
		ui->lCaptions->hide();
		ui->lEmptyRows->hide();
		ui->lExportArea->hide();
		ui->lGridLines->hide();
		ui->lMatrixHHeader->hide();
		ui->lMatrixVHeader->hide();
		ui->lSeparator->hide();
		ui->lHeader->hide();
		ui->chkEmptyRows->hide();
		ui->chkHeaders->hide();
		ui->chkExportHeader->hide();
		ui->lExportHeader->hide();
		ui->chkGridLines->hide();
		ui->chkMatrixHHeader->hide();
		ui->chkMatrixVHeader->hide();
		ui->chkCaptions->hide();
		ui->cbLaTeXExport->hide();
		ui->cbSeparator->hide();
		ui->lDecimalSeparator->hide();
		ui->cbDecimalSeparator->hide();

		ui->cbExportToFITS->show();
		ui->lExportToFITS->show();
		if (!m_matrixMode) {
			if (ui->cbExportToFITS->currentIndex() == 1) {
				ui->lColumnAsUnits->show();
				ui->chkColumnsAsUnits->show();
			}
		}
	} else if (format == Format::SQLite) {
		ui->cbSeparator->hide();
		ui->lSeparator->hide();
		ui->lDecimalSeparator->hide();
		ui->cbDecimalSeparator->hide();

		ui->chkCaptions->hide();
		ui->chkEmptyRows->hide();
		ui->chkGridLines->hide();
		ui->lEmptyRows->hide();
		ui->lExportArea->hide();
		ui->lGridLines->hide();
		ui->lCaptions->hide();
		ui->cbLaTeXExport->hide();
		ui->cbLaTeXExport->hide();
		ui->lMatrixHHeader->hide();
		ui->lMatrixVHeader->hide();
		ui->chkMatrixHHeader->hide();
		ui->chkMatrixVHeader->hide();

		ui->lHeader->hide();
		ui->chkHeaders->hide();
		ui->chkExportHeader->hide();
		ui->lExportHeader->hide();

		ui->cbExportToFITS->hide();
		ui->lExportToFITS->hide();
		ui->lColumnAsUnits->hide();
		ui->chkColumnsAsUnits->hide();
	} else {
		ui->cbSeparator->show();
		ui->lSeparator->show();
		ui->lDecimalSeparator->show();
		ui->cbDecimalSeparator->show();

		ui->chkCaptions->hide();
		ui->chkEmptyRows->hide();
		ui->chkGridLines->hide();
		ui->lEmptyRows->hide();
		ui->lExportArea->hide();
		ui->lGridLines->hide();
		ui->lCaptions->hide();
		ui->cbLaTeXExport->hide();
		ui->lMatrixHHeader->hide();
		ui->lMatrixVHeader->hide();
		ui->chkMatrixHHeader->hide();
		ui->chkMatrixVHeader->hide();

		ui->lHeader->hide();
		ui->chkHeaders->hide();

		ui->cbExportToFITS->hide();
		ui->lExportToFITS->hide();
		ui->lColumnAsUnits->hide();
		ui->chkColumnsAsUnits->hide();
	}

	if (!m_matrixMode && !(format == Format::FITS || format == Format::SQLite)) {
		ui->chkExportHeader->show();
		ui->lExportHeader->show();
	}

	setFormat(static_cast<Format>(index));
	ui->leFileName->setText(path);
}

void ExportSpreadsheetDialog::setExportSelection(bool enable) {
	if (!enable) {
		const auto* areaToExportModel = qobject_cast<const QStandardItemModel*>(ui->cbLaTeXExport->model());
		QStandardItem* item = areaToExportModel->item(1);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
	}
}

void ExportSpreadsheetDialog::setFormat(Format format) {
	m_format = format;
}

void ExportSpreadsheetDialog::setExportTo(const QStringList &to) {
	ui->cbExportToFITS->addItems(to);
}

ExportSpreadsheetDialog::Format ExportSpreadsheetDialog::format() const {
	return m_format;
}

void ExportSpreadsheetDialog::fileNameChanged(const QString& name) {
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

	m_okButton->setEnabled(true);
}
