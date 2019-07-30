/***************************************************************************
    File                 : FileInfoDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2015-2018 Stefan-Gerlach (stefan.gerlach@uni.kn)

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
#include "backend/datasources/filters/filters.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QIcon>
#include <QFileInfo>
#include <QProcess>
#include <QVBoxLayout>
#include <QWindow>

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
	m_textEditWidget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_textEditWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "FileInfoDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 200).expandedTo(minimumSize()));
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

	//resize to fit the content
	QSize size = m_textEditWidget.document()->size().toSize();
	m_textEditWidget.setMinimumSize(size.width() + m_textEditWidget.contentsMargins().left() + m_textEditWidget.contentsMargins().right(),
									size.height() + m_textEditWidget.contentsMargins().top() + m_textEditWidget.contentsMargins().bottom());
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

		// File type given by "file"
#ifdef Q_OS_LINUX
		auto* proc = new QProcess();
		QStringList args;
		args << "-b" << fileName;
		proc->start( "file", args);

		if (proc->waitForReadyRead(1000) == false)
			infoStrings << i18n("Reading from file %1 failed.", fileName);
		else {
			fileTypeString = proc->readLine();
			if (fileTypeString.contains(i18n("cannot open")))
				fileTypeString.clear();
			else {
				fileTypeString.remove(fileTypeString.length() - 1, 1);	// remove '\n'
			}
		}
		infoStrings << i18n("<b>File type:</b> %1", fileTypeString);
#endif

		// General:
		fileInfo.setFile(fileName);
		infoStrings << "<b>" << i18n("General:") << "</b>";

		infoStrings << i18n("Readable: %1", fileInfo.isReadable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Writable: %1", fileInfo.isWritable() ? i18n("yes") : i18n("no"));
		infoStrings << i18n("Executable: %1", fileInfo.isExecutable() ? i18n("yes") : i18n("no"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
		infoStrings << i18n("Birth time: %1", fileInfo.birthTime().toString());
		infoStrings << i18n("Last metadata changed: %1", fileInfo.metadataChangeTime().toString());
#else
		infoStrings << i18n("Created: %1", fileInfo.created().toString());
#endif
		infoStrings << i18n("Last modified: %1", fileInfo.lastModified().toString());
		infoStrings << i18n("Last read: %1", fileInfo.lastRead().toString());
		infoStrings << i18n("Owner: %1", fileInfo.owner());
		infoStrings << i18n("Group: %1", fileInfo.group());
		infoStrings << i18n("Size: %1", i18np("%1 cByte", "%1 cBytes", fileInfo.size()));

		// Summary:
		infoStrings << "<b>" << i18n("Summary:") << "</b>";
		//depending on the file type, generate summary and content information about the file
		//TODO: content information (in BNF) for more types
		switch (AbstractFileFilter::fileType(fileName)) {
		case AbstractFileFilter::Ascii:
			infoStrings << AsciiFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::Binary:
			infoStrings << BinaryFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::Image:
			infoStrings << ImageFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::HDF5:
			infoStrings << HDF5Filter::fileInfoString(fileName);
			infoStrings << "<b>" << i18n("Content:") << "</b>";
			infoStrings << HDF5Filter::fileDDLString(fileName);
			break;
		case AbstractFileFilter::NETCDF:
			infoStrings << NetCDFFilter::fileInfoString(fileName);
			infoStrings << "<b>" << i18n("Content:") << "</b>";
			infoStrings << NetCDFFilter::fileCDLString(fileName);
			break;
		case AbstractFileFilter::FITS:
			infoStrings << FITSFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::JSON:
			infoStrings << JsonFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::ROOT:
			infoStrings << ROOTFilter::fileInfoString(fileName);
			break;
		case AbstractFileFilter::NgspiceRawAscii:
		case AbstractFileFilter::NgspiceRawBinary:
			infoStrings << NgspiceRawAsciiFilter::fileInfoString(fileName);
			break;
		}


		infoString += infoStrings.join("<br>");

	} else
		infoString += i18n("Could not open file %1 for reading.", fileName);

	return infoString;
}
