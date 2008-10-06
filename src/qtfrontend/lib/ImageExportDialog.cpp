/***************************************************************************
    File                 : ImageExportDialog.cpp
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
#include "ImageExportDialog.h"

#include <QStackedWidget>
#include <QImageWriter>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QPrinter>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QCloseEvent>

ImageExportDialog::ImageExportDialog(QWidget * parent, bool vector_options, bool extended, Qt::WFlags flags)
	: ExtensibleFileDialog( parent, extended, flags )
{
	setWindowTitle( tr( "Choose a filename to save under" ) );
	setAcceptMode(QFileDialog::AcceptSave);

	QList<QByteArray> list = QImageWriter::supportedImageFormats();
	list<<"EPS";
	list<<"PS";
	list<<"PDF";
	#if QT_VERSION >= 0x040300
		list<<"SVG";
	#endif

	QStringList filters;
	for(int i=0 ; i<list.count() ; i++)
		filters << "*."+list[i].toLower();

	filters.sort();
	setFilters(filters);
	setFileMode( QFileDialog::AnyFile );

	initAdvancedOptions();
	m_vector_options->setEnabled(vector_options);
	setExtensionWidget(m_advanced_options);

#if QT_VERSION >= 0x040300
	connect(this, SIGNAL(filterSelected ( const QString & )),
			this, SLOT(updateAdvancedOptions ( const QString & )));
#else
	QList<QComboBox*> combo_boxes = findChildren<QComboBox*>();
	if (combo_boxes.size() >= 2)
		connect(combo_boxes[1], SIGNAL(currentIndexChanged ( const QString & )),
				this, SLOT(updateAdvancedOptions ( const QString & )));
#endif
	updateAdvancedOptions(selectedFilter());
}

void ImageExportDialog::initAdvancedOptions()
{
	m_advanced_options = new QStackedWidget();

	m_vector_options = new QGroupBox();
	QGridLayout *vector_layout = new QGridLayout(m_vector_options);
	m_advanced_options->addWidget(m_vector_options);

	vector_layout->addWidget(new QLabel(tr("Resolution (DPI)")), 1, 0);
	m_resolution = new QSpinBox();
	m_resolution->setRange(0, 1000);
	vector_layout->addWidget(m_resolution, 1, 1);

	m_color = new QCheckBox();
	m_color->setText(tr("Export in &color"));
	vector_layout->addWidget(m_color, 2, 0, 1, 2);

    m_standard_page = new QCheckBox();
	m_standard_page->setText(tr("Export to &standard page size"));
	vector_layout->addWidget(m_standard_page, 3, 0, 1, 2);

	boxPageSize = new QComboBox();
	boxPageSize->addItem("A0 - 841 x 1189 mm");
	boxPageSize->addItem("A1 - 594 x 841 mm");
	boxPageSize->addItem("A2 - 420 x 594 mm");
	boxPageSize->addItem("A3 - 297 x 420 mm");
	boxPageSize->addItem("A4 - 210 x 297 mm");
	boxPageSize->addItem("A5 - 148 x 210 mm");
	boxPageSize->addItem("A6 - 105 x 148 mm");
	boxPageSize->addItem("A7 - 74 x 105 mm");
	boxPageSize->addItem("A8 - 52 x 74 mm");
	boxPageSize->addItem("A9 - 37 x 52 mm");
	boxPageSize->addItem("B0 - 1030 x 1456 mm");
	boxPageSize->addItem("B1 - 728 x 1030 mm");
	boxPageSize->addItem("B2 - 515 x 728 mm");
	boxPageSize->addItem("B3 - 364 x 515 mm");
	boxPageSize->addItem("B4 - 257 x 364 mm");
	boxPageSize->addItem("B5 - 182 x 257 mm");
	boxPageSize->addItem("B6 - 128 x 182 mm");
	boxPageSize->addItem("B7 - 91 x 128 mm");
	boxPageSize->addItem("B8 - 64 x 91 mm");
	boxPageSize->addItem("B9 - 45 x 64 mm");

	vector_layout->addWidget(boxPageSize, 3, 1, 1, 2);

    connect(m_standard_page, SIGNAL(toggled(bool)), boxPageSize, SLOT(setEnabled(bool)));

	m_keep_aspect = new QCheckBox();
	m_keep_aspect->setText(tr("&Keep aspect ratio"));
	vector_layout->addWidget(m_keep_aspect, 4, 0, 1, 2);

	m_raster_options = new QGroupBox();
	QGridLayout *raster_layout = new QGridLayout(m_raster_options);
	m_advanced_options->addWidget(m_raster_options);

	raster_layout->addWidget(new QLabel(tr("Image quality")), 1, 0);
	m_quality = new QSpinBox();
	m_quality->setRange(1, 100);
	raster_layout->addWidget(m_quality, 1, 1);

	m_transparency = new QCheckBox();
	m_transparency->setText(tr("Save transparency"));
	raster_layout->addWidget(m_transparency, 2, 0, 1, 2);
}

void ImageExportDialog::updateAdvancedOptions (const QString & filter)
{
	if (filter.contains("*.svg")) {
		m_extension_toggle->setChecked(false);
		m_extension_toggle->setEnabled(false);
		return;
	}
	m_extension_toggle->setEnabled(true);
	if (filter.contains("*.eps") || filter.contains("*.ps") || filter.contains("*.pdf"))
		m_advanced_options->setCurrentIndex(0);
	else {
		m_advanced_options->setCurrentIndex(1);
		m_transparency->setEnabled(filter.contains("*.tif") || filter.contains("*.tiff") || filter.contains("*.png") || filter.contains("*.xpm"));
	}
}

QPrinter::PageSize ImageExportDialog::pageSize() const
{
if (!m_standard_page->isChecked())
    return QPrinter::Custom;

QPrinter::PageSize size;
switch (boxPageSize->currentIndex())
	{
	case 0:
		size = QPrinter::A0;
	break;

	case 1:
		size = QPrinter::A1;
	break;

	case 2:
		size = QPrinter::A2;
	break;

	case 3:
		size = QPrinter::A3;
	break;

	case 4:
		size = QPrinter::A4;
	break;

	case 5:
		size = QPrinter::A5;
	break;

	case 6:
		size = QPrinter::A6;
	break;

	case 7:
		size = QPrinter::A7;
	break;

	case 8:
		size = QPrinter::A8;
	break;

	case 9:
		size = QPrinter::A9;
	break;

	case 10:
		size = QPrinter::B0;
	break;

	case 11:
		size = QPrinter::B1;
	break;

	case 12:
		size = QPrinter::B2;
	break;

	case 13:
		size = QPrinter::B3;
	break;

	case 14:
		size = QPrinter::B4;
	break;

	case 15:
		size = QPrinter::B5;
	break;

	case 16:
		size = QPrinter::B6;
	break;

	case 17:
		size = QPrinter::B7;
	break;

	case 18:
		size = QPrinter::B8;
	break;

	case 19:
		size = QPrinter::B9;
	break;
	}
return size;
}

void ImageExportDialog::setPageSize(int size)
{
	m_standard_page->setChecked(size != QPrinter::Custom);
	boxPageSize->setEnabled(size != QPrinter::Custom);
if (size == QPrinter::Custom)
    return;
if (!size)
    boxPageSize->setCurrentIndex(4);
else if (size == 1)
    boxPageSize->setCurrentIndex(15);
else if (size >= 5 && size <= 8)
    boxPageSize->setCurrentIndex(size - 5);
else if (size > 8 && size <= 23)
    boxPageSize->setCurrentIndex(size - 4);
}

void ImageExportDialog::selectFilter(const QString & filter)
{
	QFileDialog::selectFilter(filter);
	updateAdvancedOptions(filter);
}
