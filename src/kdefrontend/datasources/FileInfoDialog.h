/***************************************************************************
    File                 : FileInfoDialog.h
    Project              : LabPlot
    Description          : file info dialog
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

#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include <QDialog>
#include <QTextEdit>

class FileInfoDialog : public QDialog {
Q_OBJECT

public:
	explicit FileInfoDialog(QWidget*);
	~FileInfoDialog();
	void setFiles(QStringList&);

private:
	QTextEdit m_textEditWidget;
	QString fileInfoString(const QString&) const;

private slots:
	void loadSettings();
};

#endif //IMPORTFILEDIALOG_H
