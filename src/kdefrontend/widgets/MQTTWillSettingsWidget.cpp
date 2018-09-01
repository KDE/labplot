/***************************************************************************
	File                 : MQTTWillSettingsWidget.cpp
	Project              : LabPlot
	Description          : widget for managing MQTT connection's will settings
	--------------------------------------------------------------------
	Copyright            : (C) 2018 by Ferencz Kovacs (kferike98@gmail.com)

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
#include "MQTTWillSettingsWidget.h"

#ifdef HAVE_MQTT
/*!
	\class MQTTWillSettingsWidget
	\brief Widget for managing MQTT connection's will settings

	\ingroup kdefrontend
 */
MQTTWillSettingsWidget::MQTTWillSettingsWidget(QWidget* parent, MQTTClient::MQTTWill will, QVector<QString> topics) :
	QWidget(parent),
	m_MQTTWill(will),
	m_topics(topics),
	m_initialising(false)
{
	ui.setupUi(this);
	ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000) );

	connect(ui.chbWill, &QCheckBox::stateChanged, this, &MQTTWillSettingsWidget::useWillMessage);
	connect(ui.cbWillMessageType, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &MQTTWillSettingsWidget::willMessageTypeChanged);
	connect(ui.cbWillUpdate, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &MQTTWillSettingsWidget::willUpdateTypeChanged);

	m_initialising = true;
	loadSettings();
	m_initialising = false;

	connect(ui.chbWillRetain, &QCheckBox::stateChanged, [this](int state) {emit retainChanged(state);});
	connect(ui.leWillOwnMessage, &QLineEdit::textChanged, [this](QString text) {emit ownMessageChanged(text);});
	connect(ui.leWillUpdateInterval, &QLineEdit::textChanged, [this](QString text) {emit intervalChanged(text.simplified().toInt());});
	connect(ui.lwWillStatistics, &QListWidget::itemChanged, [this](QListWidgetItem * item) {emit statisticsChanged(ui.lwWillStatistics->row(item));});
	connect(ui.cbWillQoS, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), [this](int index) {emit  QoSChanged(index);});
	connect(ui.cbWillTopic, static_cast<void (QComboBox::*) (const QString&)>(&QComboBox::currentTextChanged), [this](QString text) {emit topicChanged(text);});
	connect(ui.bCancel, &QPushButton::clicked, this, &MQTTWillSettingsWidget::canceled);
}

/*!
 *\brief called when use will message checkbox's state is changed,
 *       if state is checked it shows the options regarding the will message
 *
 * \param state the state of the checbox
 */
void MQTTWillSettingsWidget::useWillMessage(int state) {
	if (state == Qt::Checked) {
		ui.chbWillRetain->show();
		ui.cbWillQoS->show();
		ui.cbWillMessageType->show();
		ui.cbWillTopic->show();
		ui.cbWillUpdate->show();
		ui.lWillMessageType->show();
		ui.lWillQos->show();
		ui.lWillTopic->show();
		ui.lWillUpdate->show();

		if (ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::OwnMessage) ) {
			ui.leWillOwnMessage->show();
			ui.lWillOwnMessage->show();
		} else if (ui.cbWillMessageType->currentIndex() == static_cast<int>(MQTTClient::WillMessageType::Statistics) ) {
			ui.lWillStatistics->show();
			ui.lwWillStatistics->show();
		}

		if (ui.cbWillUpdate->currentIndex() == 0) {
			ui.leWillUpdateInterval->show();
			ui.lWillUpdateInterval->show();
		}
	} else if (state == Qt::Unchecked) {
		ui.chbWillRetain->hide();
		ui.cbWillQoS->hide();
		ui.cbWillMessageType->hide();
		ui.cbWillTopic->hide();
		ui.cbWillUpdate->hide();
		ui.leWillOwnMessage->hide();
		ui.leWillUpdateInterval->hide();
		ui.lWillMessageType->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillQos->hide();
		ui.lWillTopic->hide();
		ui.lWillUpdate->hide();
		ui.lWillUpdateInterval->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	}

	if (!m_initialising)
		emit useChanged(state);
}

/*!
 *\brief called when the selected will message type is changed,
 *       shows the options for the selected message type, hides the irrelevant ones
 *
 * \param type the selected will message type
 */
void MQTTWillSettingsWidget::willMessageTypeChanged(int type) {
	if (static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::OwnMessage) {
		ui.leWillOwnMessage->show();
		ui.lWillOwnMessage->show();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	} else if (static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::LastMessage) {
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	} else if (static_cast<MQTTClient::WillMessageType> (type) == MQTTClient::WillMessageType::Statistics) {
		ui.lWillStatistics->show();
		ui.lwWillStatistics->show();
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
	}

	if (!m_initialising)
		emit messageTypeChanged(type);
}

/*!
 *\brief called when the selected update type for the will message is changed,
 *       shows the options for the selected update type, hides the irrelevant ones
 *
 * \param type the selected will update type
 */
void MQTTWillSettingsWidget::willUpdateTypeChanged(int updateType) {
	if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::TimePeriod) {
		ui.leWillUpdateInterval->show();
		ui.lWillUpdateInterval->show();
	} else if (static_cast<MQTTClient::WillUpdateType>(updateType) == MQTTClient::WillUpdateType::OnClick) {
		ui.leWillUpdateInterval->hide();
		ui.lWillUpdateInterval->hide();
	}

	if (!m_initialising)
		emit updateTypeChanged(updateType);
}

/*!
 * \brief Updates the widget based on the will settings
 */
void MQTTWillSettingsWidget::loadSettings() {
	if (ui.chbWill->isChecked() != m_MQTTWill.MQTTUseWill)
		ui.chbWill->setChecked(m_MQTTWill.MQTTUseWill);
	else {
		if (m_MQTTWill.MQTTUseWill)
			useWillMessage(Qt::Checked);
		else
			useWillMessage(Qt::Unchecked);
	}

	for (int i = 0; i < m_topics.size(); ++i)
		ui.cbWillTopic->addItem(m_topics[i]);

	//Set back the initial value
	if (!m_MQTTWill.willTopic.isEmpty())
		ui.cbWillTopic->setCurrentText(m_MQTTWill.willTopic);

	if (ui.cbWillMessageType->currentIndex() != static_cast<int> (m_MQTTWill.willMessageType))
		ui.cbWillMessageType->setCurrentIndex(m_MQTTWill.willMessageType);
	else
		willMessageTypeChanged(m_MQTTWill.willMessageType);

	if (ui.cbWillUpdate->currentIndex() != static_cast<int> (m_MQTTWill.willUpdateType))
		ui.cbWillUpdate->setCurrentIndex(m_MQTTWill.willUpdateType);
	else
		willUpdateTypeChanged(m_MQTTWill.willUpdateType);

	ui.leWillOwnMessage->setText(m_MQTTWill.willOwnMessage);
	ui.leWillUpdateInterval->setText(QString::number(m_MQTTWill.willTimeInterval));
	ui.chbWillRetain->setChecked(m_MQTTWill.willRetain);

	for (int i = 0; i < m_MQTTWill.willStatistics.size(); ++i) {
		if (m_MQTTWill.willStatistics[i])
			ui.lwWillStatistics->item(i)->setCheckState(Qt::Checked);
		else
			ui.lwWillStatistics->item(i)->setCheckState(Qt::Unchecked);
	}
}
#endif
