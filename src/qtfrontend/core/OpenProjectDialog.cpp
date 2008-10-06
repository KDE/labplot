/***************************************************************************
    File                 : OpenProjectDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Ion Vasilief
    Email (use @ for *)  : knut.franke*gmx.de, ion_vasilief*yahoo.fr
    Description          : Dialog for opening project files.

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

#include "OpenProjectDialog.h"
#include "ApplicationWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>

OpenProjectDialog::OpenProjectDialog(QWidget *parent, bool extended, Qt::WFlags flags)
	: ExtensibleFileDialog(parent, extended, flags)
{
	setCaption(tr("Open Project"));
	setFileMode(ExistingFile);
	QStringList filters;
	filters << tr("SciDAVis project") + " (*.sciprj)"
		<< tr("Compressed SciDAVis project") + " (*.sciprj.gz)"
		<< tr("QtiPlot project") + " (*.qti)"
		<< tr("Compressed QtiPlot project") + " (*.qti.gz)"
		<< tr("Origin project") + " (*.opj *.OPJ)"
		<< tr("Origin matrix") + " (*.ogm *.OGM)"
		<< tr("Origin worksheet") + " (*.ogw *.OGW)"
		<< tr("Origin graph") + " (*.ogg *.OGG)"
		<< tr("Backup files") + " (*.sciprj~)"
		//<< tr("Python Source") + " (*.py *.PY)"
		<< tr("All files") + " (*)";
	setFilters(filters);

	QWidget *advanced_options = new QWidget();
	QHBoxLayout *advanced_layout = new QHBoxLayout();
	advanced_options->setLayout(advanced_layout);
	advanced_layout->addWidget(new QLabel(tr("Open As")));
	m_open_mode = new QComboBox();
	// Important: Keep this is sync with enum OpenMode.
	m_open_mode->addItem(tr("New Project Window"));
	m_open_mode->addItem(tr("New Folder"));
	advanced_layout->addWidget(m_open_mode);
	setExtensionWidget(advanced_options);

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

void OpenProjectDialog::updateAdvancedOptions (const QString & filter)
{
	if (filter.contains("*.ogm") || filter.contains("*.ogw")) {
		m_extension_toggle->setChecked(false);
		m_extension_toggle->setEnabled(false);
		return;
	}
	m_extension_toggle->setEnabled(true);
}

void OpenProjectDialog::closeEvent(QCloseEvent* e)
{
	if (isExtendable()){
		ApplicationWindow *app = (ApplicationWindow *)this->parent();
		if (app)
			app->m_extended_open_dialog = this->isExtended();
	}

	e->accept();
}
