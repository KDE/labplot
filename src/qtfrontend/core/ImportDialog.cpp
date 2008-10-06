/***************************************************************************
    File                 : ImportDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Select file(s) to import and import filter to use.

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

#include "core/ImportDialog.h"
#include "core/AbstractImportFilter.h"
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSignalMapper>
#include <QMetaProperty>
#include <QVariant>
#include <QLabel>
#include <QComboBox>

ImportDialog::ImportDialog(QMap<QString, AbstractImportFilter*> filter_map, QWidget *parent)
	: ExtensibleFileDialog(parent), m_filter_map(filter_map), m_options_ui(0)
{
	setWindowTitle(tr("Import File(s)"));
	setFileMode( QFileDialog::ExistingFiles );

	QWidget * extension_widget = new QWidget();
	QGridLayout * extension_layout = new QGridLayout();
	extension_widget->setLayout(extension_layout);
	extension_layout->addWidget(new QLabel(tr("Destination:")), 0, 0);
	m_destination = new QComboBox(extension_widget);
	// IMPORTANT: keep this in sync with enum Destination
	m_destination->addItem(tr("Current project"));
	m_destination->addItem(tr("New project(s)"));
	extension_layout->addWidget(m_destination, 0, 1);
	setExtensionWidget(extension_widget);

	setFilters(m_filter_map.keys());
	filterChanged(selectedFilter());

	connect(this, SIGNAL(filterSelected(const QString&)),
			this, SLOT(filterChanged(const QString&)));
}

void ImportDialog::filterChanged(const QString& filter_name) {
	m_filter = m_filter_map[filter_name];
	if (!m_filter) {
		if (m_options_ui) delete m_options_ui;
		m_options_ui = 0;
		return;
	}
	if (!QMetaObject::invokeMethod(m_filter, "makeOptionsGui", Qt::DirectConnection,
			Q_RETURN_ARG(QWidget*, m_options_ui)))
		m_options_ui = generateOptionsGUI();
	static_cast<QGridLayout*>(extensionWidget()->layout())->addWidget(m_options_ui, 1, 0, 1, 2);
};

QWidget * ImportDialog::generateOptionsGUI() {
	qDeleteAll(m_settings_map);
	m_settings_map.clear();

	QWidget * settings = new QWidget();
	QGridLayout * layout = new QGridLayout();
	settings->setLayout(layout);
	int offset = m_filter->metaObject()->propertyOffset();
	// don't include properties of QObject
	int prop_count = m_filter->metaObject()->propertyCount() - offset;
	for (int i=0; i<prop_count; i++) {
		QMetaProperty * prop = new QMetaProperty(m_filter->metaObject()->property(offset+i));
		layout->addWidget(new QLabel(QString(prop->name()).replace("_", " "), settings), i/2, 2*(i%2));
		switch(prop->type()) {
			case QVariant::Int:
				{
					QSpinBox * box = new QSpinBox(settings);
					box->setValue(prop->read(m_filter).toInt());
					connect(box, SIGNAL(valueChanged(int)),
							this, SLOT(intValueChanged(int)));
					m_settings_map[static_cast<QObject*>(box)] = prop;
					layout->addWidget(box, i/2, 2*(i%2)+1);
					break;
				}
			case QVariant::Bool:
				{
					QCheckBox * box = new QCheckBox(settings);
					box->setCheckState(prop->read(m_filter).toBool() ? Qt::Checked : Qt::Unchecked);
					connect(box, SIGNAL(stateChanged(int)),
							this, SLOT(boolValueChanged(int)));
					m_settings_map[static_cast<QObject*>(box)] = prop;
					layout->addWidget(box, i/2, 2*(i%2)+1);
					break;
				}
			case QVariant::String:
				{
					QLineEdit * edit = new QLineEdit(settings);
					edit->setText(prop->read(m_filter).toString());
					connect(edit, SIGNAL(textChanged(const QString&)),
							this, SLOT(stringValueChanged(const QString&)));
					m_settings_map[static_cast<QObject*>(edit)] = prop;
					layout->addWidget(edit, i/2, 2*(i%2)+1);
					break;
				}
				// TODO: support more types
			default:
				break;
		}
	}
	return settings;
}

ImportDialog::Destination ImportDialog::destination() const
{
	return (Destination) m_destination->currentIndex();
}

void ImportDialog::intValueChanged(int value)
{
	m_settings_map[sender()]->write(m_filter, QVariant(value));
}

void ImportDialog::boolValueChanged(int state)
{
	switch (state) {
		case Qt::Checked:
			m_settings_map[sender()]->write(m_filter, QVariant(true));
			break;
		case Qt::Unchecked:
			m_settings_map[sender()]->write(m_filter, QVariant(false));
			break;
	}
}

void ImportDialog::stringValueChanged(const QString &value)
{
	m_settings_map[sender()]->write(m_filter, QVariant(value));
}
