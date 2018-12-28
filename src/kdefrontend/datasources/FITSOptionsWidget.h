/***************************************************************************
File                 : FITSOptionsWidget.cpp
Project              : LabPlot
Description          : Widget providing options for the import of FITS data
--------------------------------------------------------------------
Copyright            : (C) 2016 Fabian Kristof (fkristofszabolcs@gmail.com)
Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#ifndef FITSOPTIONSWIDGET_H
#define FITSOPTIONSWIDGET_H

#include "ui_fitsoptionswidget.h"

class FITSFilter;
class ImportFileWidget;

class FITSOptionsWidget : public QWidget {
	Q_OBJECT

public:
    explicit FITSOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	QString currentExtensionName();
	void updateContent(FITSFilter*, const QString& fileName);
	const QStringList selectedExtensions() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }
	const QString extensionName(bool* ok);

private:
    Ui::FITSOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void fitsTreeWidgetSelectionChanged();
};

#endif // FITSOPTIONSWIDGET_H
