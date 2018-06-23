/***************************************************************************
    File                 : FileInfoDialog.cpp
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Alexander Semke (alexander.semke@web.de)
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

#include <KLocalizedString>
#include <QFileInfo>
#include <QProcess>
#include <QDialogButtonBox>
#include <QIcon>
#include <QVBoxLayout>

#include <KWindowConfig>
#include <KSharedConfig>

/*!
\class ImportWidget
\brief Provides a dialog containing the information about the files to be imported.

\ingroup kdefrontend
*/

FileInfoDialog::FileInfoDialog(QWidget* parent) : QDialog(parent) {

	m_textEditWidget.setReadOnly(true);
	m_textEditWidget.setLineWrapMode(QTextEdit::NoWrap);

	QVBoxLayout* layout = new QVBoxLayout(this);
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

	for (const auto& fileName: files) {
		if(fileName.isEmpty())
			continue;

		if (!infoString.isEmpty())
			infoString += "<br><br><br>";

		infoString += LiveDataSource::fileInfoString(fileName);
	}

	m_textEditWidget.document()->setHtml(infoString);
}
