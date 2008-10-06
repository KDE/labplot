/***************************************************************************
    File                 : ImageExportDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : QFileDialog extended with options for image export

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
#ifndef IMAGE_EXPORT_DIALOG_H
#define IMAGE_EXPORT_DIALOG_H

#include "ExtensibleFileDialog.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QPrinter>

class QStackedWidget;
class QGroupBox;
class QComboBox;

//! QFileDialog extended with options for image export
class ImageExportDialog: public ExtensibleFileDialog
{
	Q_OBJECT

private:
	//! Create #m_advanced_options and everything it contains.
	void initAdvancedOptions();

	//! Container widget for all advanced options.
	QStackedWidget *m_advanced_options;
	// vector format options
	//! Container widget for all options available for vector formats.
	QGroupBox *m_vector_options;
	QSpinBox *m_resolution;
	QCheckBox *m_color;
	QCheckBox *m_keep_aspect;
	QCheckBox *m_standard_page;
	QComboBox *boxPageSize;
	// raster format options
	//! Container widget for all options available for raster formats.
	QGroupBox *m_raster_options;
	QSpinBox *m_quality;
	QCheckBox *m_transparency;

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param vector_options whether advanced options are to be provided for export to vector formats
	 * \param extended flag: show/hide the advanced options on start-up
	 * \param flags window flags
	 */
	ImageExportDialog(QWidget * parent = 0, bool vector_options = true, bool extended = true, Qt::WFlags flags = 0 );
	//! For vector formats: returns the output resolution the user selected, defaulting to the screen resolution.
	int resolution() const { return m_resolution->value(); }
	//! For vector formats: sets the output resolution the user selected.
	void setResolution(int value) { m_resolution->setValue(value); }
	//! For vector formats: returns whether colors should be enabled for ouput (default: true).
	bool colorEnabled() const { return m_color->isChecked(); }
	//! For vector formats: sets whether colors should be enabled for ouput (default: true).
	void setColorEnabled(bool value) { m_color->setChecked(value); }
	//! For vector formats: returns whether the output should preserve aspect ratio of the plot (default: true).
	bool keepAspect() const { return m_keep_aspect->isChecked(); }
	//! For vector formats: sets whether the output should preserve aspect ratio of the plot (default: true).
	void setKeepAspect(bool value) { m_keep_aspect->setChecked(value); }
	//! For vector formats: returns a standard output page size (default: QPrinter::Custom).
	QPrinter::PageSize pageSize() const;
	void setPageSize(int size);
	//! Return the quality (in percent) the user selected for export to raster formats.
	int quality() const { return m_quality->value(); }
	//! Preset the quality (in percent) for export to raster formats.
	void setQuality(int value) { m_quality->setValue(value); }
	//! Return whether the output's background should be transparent.
	bool transparency() const { return m_transparency->isChecked(); }
	//! Preset whether the output's background should be transparent.
	void setTransparency(bool value) { m_transparency->setChecked(value); }

	void selectFilter(const QString & filter);
	
protected slots:
	//! Update which options are visible and enabled based on the output format.
	void updateAdvancedOptions (const QString &filter);
};

#endif // ifndef IMAGE_EXPORT_DIALOG_H
