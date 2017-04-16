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
#include "backend/datasources/FileDataSource.h"

#include <KLocale>
#include <QFileInfo>
#include <QProcess>

 /*!
	\class ImportWidget
	\brief Provides a dialog containing the information about the files to be imported.

	\ingroup kdefrontend
 */

FileInfoDialog::FileInfoDialog(QWidget* parent) : KDialog(parent) {
	textEditWidget.setReadOnly(true);
	textEditWidget.setLineWrapMode(QTextEdit::NoWrap);
	setMainWidget( &textEditWidget );
	setButtons( KDialog::Ok);
	setWindowIcon(KIcon("help-about"));
	setCaption(i18n("File info"));
	setAttribute(Qt::WA_DeleteOnClose);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "FileInfoDialog");
	if (conf.exists())
		restoreDialogSize(conf);
	else
		resize( QSize(500,300).expandedTo(minimumSize()) );
}

FileInfoDialog::~FileInfoDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FileInfoDialog");
	saveDialogSize(conf);
}

void FileInfoDialog::setFiles(QStringList& files) {
	QString fileName;
	QString infoString;

	 for ( int i=0; i<files.size(); i++ ) {
		fileName = files.at(i);
		if(fileName.isEmpty())
			continue;

        if (!infoString.isEmpty())
            infoString += "<br><br><br>";

        infoString += FileDataSource::fileInfoString(fileName);
	}

	textEditWidget.document()->setHtml(infoString);
}
