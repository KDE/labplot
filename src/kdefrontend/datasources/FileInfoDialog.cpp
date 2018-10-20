/***************************************************************************
    File                 : FileInfoDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2018 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2015-2016 Stefan-Gerlach (stefan.gerlach@uni.kn)

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

#include "FileInfoDialog.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/NgspiceRawAsciiFilter.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QIcon>
#include <QFileInfo>
#include <QProcess>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
\class ImportWidget
\brief Provides a dialog containing the information about the files to be imported.

\ingroup kdefrontend
*/

FileInfoDialog::FileInfoDialog(QWidget* parent) : QDialog(parent) {

	m_textEditWidget.setReadOnly(true);
	m_textEditWidget.setLineWrapMode(QTextEdit::NoWrap);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(&m_textEditWidget);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &FileInfoDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &FileInfoDialog::accept);

	layout->addWidget(buttonBox);

	setWindowIcon(QIcon::fromTheme("help-about"));
	setWindowTitle(i18nc("@title:window", "File Information"));
	setAttribute(Qt::WA_DeleteOnClose);

	setLayout(layout);

	QTimer::singleShot(0, this, &FileInfoDialog::loadSettings);
}

void FileInfoDialog::loadSettings() {
	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "FileInfoDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), conf);
}

FileInfoDialog::~FileInfoDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "FileInfoDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FileInfoDialog::setFiles(QStringList& files) {
	QString infoString;

	for (const auto& fileName : files) {
		if (fileName.isEmpty())
			continue;

		if (!infoString.isEmpty())
			infoString += "<br><br><br>";

		infoString += fileInfoString(fileName);
	}

	m_textEditWidget.document()->setHtml(infoString);
}

/*!
    returns a string containing the general information about the file \c name
    and some content specific information
    (number of columns and lines for ASCII, color-depth for images etc.).
 */
QString FileInfoDialog::fileInfoString(const QString& name) const {
	QString infoString;
	QFileInfo fileInfo;
	QString fileTypeString;
	QIODevice *file = new QFile(name);

	QString fileName;
#ifdef Q_OS_WIN
	if (name.at(1) != QLatin1Char(':'))
		fileName = QDir::homePath() + name;
	else
		fileName = name;
#else
	if (name.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + name;
	else
		fileName = name;
#endif
	if (!file)
		file = new QFile(fileName);

	if (file->open(QIODevice::ReadOnly)) {
		QStringList infoStrings;
		infoStrings << "<u><b>" + fileName + "</b></u><br>";

		// file type and type specific information about the file
#ifdef Q_OS_LINUX
		auto* proc = new QProcess();
		QStringList args;
		args<<"-b"<<fileName;
		proc->start( "file", args);

		if (proc->waitForReadyRead(1000) == false)
			infoStrings << i18n("Could not open file %1 for reading.", fileName);
		else {
			fileTypeString = proc->readLine();
			if ( fileTypeString.contains(i18n("cannot open")) )
				fileTypeString="";
			else {
				fileTypeString.remove(fileTypeString.length()-1,1);	// remove '\n'
			}
		}
		infoStrings << i18n("File type: %1", fileTypeString);
#endif

		//depending on the file type, generate additional information about the file:
		infoStrings << "<br>";
		AbstractFileFilter::FileType fileType = AbstractFileFilter::fileType(fileName);
		switch (fileType) {
		case AbstractFileFilter::Ascii:
			infoStrings << AsciiFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::Binary:
			//TODO infoStrings << BinaryFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::Image:
			//TODO infoStrings << ImageFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::HDF5:
			//TODO infoStrings << HDF5Filter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::NETCDF:
			//TODO infoStrings << NETCDFFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::FITS:
			infoStrings << FITSFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::JSON:
			//TODO infoStrings << JsonFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::ROOT:
			//TODO infoStrings << ROOTFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::NgspiceRawAscii:
		case AbstractFileFilter::NgspiceRawBinary:
			infoStrings << NgspiceRawAsciiFilter::fileInfoString(fileName);
			break;
		}

		//general information about the file
		infoStrings << "<br>";
		fileInfo.setFile(fileName);

		infoStrings << i18n("Readable: %1", fileInfo.isReadable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Writable: %1", fileInfo.isWritable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Executable: %1", fileInfo.isExecutable() ? i18n("yes") : i18n("no"));

		infoStrings << i18n("Created: %1", fileInfo.created().toString());
		infoStrings << i18n("Last modified: %1", fileInfo.lastModified().toString());
		infoStrings << i18n("Last read: %1", fileInfo.lastRead().toString());
		infoStrings << i18n("Owner: %1", fileInfo.owner());
		infoStrings << i18n("Group: %1", fileInfo.group());
		infoStrings << i18n("Size: %1", i18np("%1 cByte", "%1 cBytes", fileInfo.size()));
		infoString += infoStrings.join("<br>");
	} else
		infoString += i18n("Could not open file %1 for reading.", fileName);

	return infoString;
}
