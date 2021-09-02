/*
File                 : MQTTConnectionManagerDialog.cpp
Project              : LabPlot
Description          : widget for managing MQTT connections
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "MQTTConnectionManagerDialog.h"
#include "MQTTConnectionManagerWidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QWindow>

/*!
	\class MQTTConnectionManagerDialog
	\brief dialog for managing MQTT connections

	\ingroup kdefrontend
*/
MQTTConnectionManagerDialog::MQTTConnectionManagerDialog(QWidget* parent, const QString& conn, bool changed) : QDialog(parent),
	mainWidget(new MQTTConnectionManagerWidget(this, conn)),
	m_initialConnectionChanged(changed),
	m_initialConnection(conn) {

	setWindowIcon(QIcon::fromTheme("labplot-MQTT"));
	setWindowTitle(i18nc("@title:window", "MQTT Connections"));

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(m_buttonBox);

	connect(mainWidget, &MQTTConnectionManagerWidget::changed, this, &MQTTConnectionManagerDialog::changed);
	connect(m_buttonBox->button(QDialogButtonBox::Ok),&QPushButton::clicked, this, &MQTTConnectionManagerDialog::save);
	connect(m_buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &MQTTConnectionManagerDialog::close);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "MQTTConnectionManagerDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

/*!
 * \brief returns the selected connection of the mainWidget
 */
QString MQTTConnectionManagerDialog::connection() const {
	return mainWidget->connection();
}

/*!
 * \brief Returns whether the initial connection has been changed
 * \return m_initialConnectionChanged
 */
bool MQTTConnectionManagerDialog::initialConnectionChanged() const
{
	return m_initialConnectionChanged;
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
	if (mainWidget->connection() == m_initialConnection)
		m_initialConnectionChanged = true;

	if (mainWidget->checkConnections()) {
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		m_changed = true;
	} else {
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

/*!
 * \brief Saves the settings for the mainWidget
 */
void MQTTConnectionManagerDialog::save() {
	//ok-button was clicked, save the connections if they were changed
	if (m_changed)
		mainWidget->saveConnections();
}
