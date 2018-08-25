/***************************************************************************
File                 : MQTTConnectionManagerDialog.cpp
Project              : LabPlot
Description          : widget for managing MQTT connections
--------------------------------------------------------------------
Copyright            : (C) 2018 Ferencz Kovacs (kferike98@gmail.com)

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

#include "MQTTConnectionManagerDialog.h"
#include "MQTTConnectionManagerWidget.h"

#ifdef HAVE_MQTT

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QTimer>

/*!
	\class MQTTConnectionManagerDialog
	\brief dialog for managing MQTT connections

	\ingroup kdefrontend
*/
MQTTConnectionManagerDialog::MQTTConnectionManagerDialog(QWidget* parent, const QString& conn, bool* changed) : QDialog(parent),
	mainWidget(new MQTTConnectionManagerWidget(this, conn)), m_changed(false),
	m_initialConnectionChanged(changed), m_initialConnection(conn) {

	setWindowIcon(QIcon::fromTheme("labplot-MQTT"));
	setWindowTitle(i18nc("@title:window", "MQTT Connections"));

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(m_buttonBox);

	connect(mainWidget, &MQTTConnectionManagerWidget::changed, this, &MQTTConnectionManagerDialog::changed);
	connect(m_buttonBox->button(QDialogButtonBox::Ok),&QPushButton::clicked, this, &MQTTConnectionManagerDialog::save);
	connect(m_buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &MQTTConnectionManagerDialog::close);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QTimer::singleShot(0, this, &MQTTConnectionManagerDialog::loadSettings);
}

/*!
 * \brief Loads the settings for the dialog
 */
void MQTTConnectionManagerDialog::loadSettings() {
	//restore saved settings
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	KConfigGroup conf(KSharedConfig::openConfig(), "MQTTConnectionManagerDialog");
	KWindowConfig::restoreWindowSize(windowHandle(), conf);
}

/*!
 * \brief returns the selected connection of the mainWidget
 */
QString MQTTConnectionManagerDialog::connection() const {
	return mainWidget->connection();
}

MQTTConnectionManagerDialog::~MQTTConnectionManagerDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "MQTTConnectionManagerDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

/*!
 * \brief Called when the user changes any setting in mainWidget
 */
void MQTTConnectionManagerDialog::changed() {
	setWindowTitle(i18nc("@title:window", "MQTT Connections  [Changed]"));

	//set true if initial connection was changed
	if(mainWidget->connection() == m_initialConnection)
		*m_initialConnectionChanged = true;

	if(mainWidget->checkConnections()) {
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		m_changed = true;
	} else
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

/*!
 * \brief Saves the settings for the mainWidget
 */
void MQTTConnectionManagerDialog::save() {
	//ok-button was clicked, save the connections if they were changed
	if (m_changed)
		mainWidget->saveConnections();
}
#endif
