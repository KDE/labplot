/***************************************************************************
File                 : HDF5OptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of HDF5 data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)

**************************************************************************/

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
#ifndef HDF5OPTIONSWIDGET_H
#define HDF5OPTIONSWIDGET_H

#include "ui_hdf5optionswidget.h"

class HDF5Filter;
class ImportFileWidget;

class HDF5OptionsWidget : public QWidget {
    Q_OBJECT

public:
	explicit HDF5OptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(HDF5Filter*, const QString &fileName);
	const QStringList selectedHDF5Names() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::HDF5OptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void hdf5TreeWidgetSelectionChanged();
};

#endif
