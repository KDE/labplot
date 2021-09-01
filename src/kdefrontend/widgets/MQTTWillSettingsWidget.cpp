/*
File                 : MQTTWillSettingsWidget.cpp
Project              : LabPlot
Description          : widget for managing MQTT connection's will settings
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
SPDX-FileCopyrightText: 2018 Fabian Kristof <fkristofszabolcs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "MQTTWillSettingsWidget.h"

/*!
	\class MQTTWillSettingsWidget
	\brief Widget for managing MQTT connection's will settings

	\ingroup kdefrontend
 */
MQTTWillSettingsWidget::MQTTWillSettingsWidget(QWidget* parent, const MQTTClient::MQTTWill& will, const QVector<QString>& topics) : QWidget(parent), m_will(will) {
	ui.setupUi(this);
	ui.leWillUpdateInterval->setValidator(new QIntValidator(2, 1000000));

	connect(ui.cbWillMessageType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MQTTWillSettingsWidget::willMessageTypeChanged);
	connect(ui.cbWillUpdate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MQTTWillSettingsWidget::willUpdateTypeChanged);

	connect(ui.chbEnabled, &QCheckBox::stateChanged, this, &MQTTWillSettingsWidget::enableWillSettings);
	connect(ui.chbWillRetain, &QCheckBox::stateChanged, [this](int state) {
		m_will.willRetain = (state == Qt::Checked);
	});
	connect(ui.leWillOwnMessage, &QLineEdit::textChanged, [this](const QString& text) {
		m_will.willOwnMessage = text;
	});
	connect(ui.leWillUpdateInterval, &QLineEdit::textChanged, [this](const QString& text) {
		m_will.willTimeInterval = text.toInt();
	});
	connect(ui.lwWillStatistics, &QListWidget::itemChanged, [this](QListWidgetItem* item) {
		m_statisticsType = static_cast<MQTTClient::WillStatisticsType>(ui.lwWillStatistics->row(item));
	});
	connect(ui.cbWillQoS, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		m_will.willQoS = index;
	});
	connect(ui.cbWillTopic, QOverload<const QString&>::of(&QComboBox::currentTextChanged), [this](const QString& text) {
		m_will.willTopic = text;
	});
	connect(ui.bApply, &QPushButton::clicked, this, &MQTTWillSettingsWidget::applyClicked);

	loadSettings(will, topics);
}

MQTTClient::MQTTWill MQTTWillSettingsWidget::will() const {
	return m_will;
}

MQTTClient::WillStatisticsType MQTTWillSettingsWidget::statisticsType() const {
	return m_statisticsType;
}

/*!
 *\brief called when the selected will message type is changed,
 *       shows the options for the selected message type, hides the irrelevant ones
 *
 * \param type the selected will message type
 */
void MQTTWillSettingsWidget::willMessageTypeChanged(int index) {
	m_will.willMessageType = static_cast<MQTTClient::WillMessageType>(index);
	if (m_will.willMessageType == MQTTClient::WillMessageType::OwnMessage) {
		ui.leWillOwnMessage->show();
		ui.lWillOwnMessage->show();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	} else if (m_will.willMessageType == MQTTClient::WillMessageType::LastMessage) {
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
		ui.lWillStatistics->hide();
		ui.lwWillStatistics->hide();
	} else if (m_will.willMessageType == MQTTClient::WillMessageType::Statistics) {
		ui.lWillStatistics->show();
		ui.lwWillStatistics->show();
		ui.leWillOwnMessage->hide();
		ui.lWillOwnMessage->hide();
	}
}

/*!
 *\brief called when the selected update type for the will message is changed,
 *       shows the options for the selected update type, hides the irrelevant ones
 *
 * \param type the selected will update type
 */
void MQTTWillSettingsWidget::willUpdateTypeChanged(int index) {
	m_will.willUpdateType = static_cast<MQTTClient::WillUpdateType>(index);
	if (m_will.willUpdateType == MQTTClient::WillUpdateType::TimePeriod) {
		ui.leWillUpdateInterval->show();
		ui.lWillUpdateInterval->show();
	} else if (m_will.willUpdateType == MQTTClient::WillUpdateType::OnClick) {
		ui.leWillUpdateInterval->hide();
		ui.lWillUpdateInterval->hide();
	}
}

/*!
 * \brief Updates the widget based on the will settings
 */
void MQTTWillSettingsWidget::loadSettings(const MQTTClient::MQTTWill& will, const QVector<QString>& topics) {
	ui.chbEnabled->setChecked(will.enabled);
	enableWillSettings(will.enabled);

	ui.cbWillTopic->addItems(topics.toList());
	//Set back the initial value
	if (!will.willTopic.isEmpty())
		ui.cbWillTopic->setCurrentText(will.willTopic);

	int messageType = static_cast<int>(will.willMessageType);
	if (ui.cbWillMessageType->currentIndex() != messageType)
		ui.cbWillMessageType->setCurrentIndex(messageType);
	else
		willMessageTypeChanged(messageType);

	int updateType = static_cast<int>(will.willUpdateType);
	if (ui.cbWillUpdate->currentIndex() != updateType)
		ui.cbWillUpdate->setCurrentIndex(updateType);
	else
		willUpdateTypeChanged(updateType);

	ui.leWillOwnMessage->setText(will.willOwnMessage);
	ui.leWillUpdateInterval->setText(QString::number(will.willTimeInterval));
	ui.chbWillRetain->setChecked(will.willRetain);

	for (int i = 0; i < will.willStatistics.size(); ++i) {
		if (will.willStatistics[i])
			ui.lwWillStatistics->item(i)->setCheckState(Qt::Checked);
		else
			ui.lwWillStatistics->item(i)->setCheckState(Qt::Unchecked);
	}
}

void MQTTWillSettingsWidget::enableWillSettings(int state) {
	const bool enabled = m_will.enabled = (state == Qt::Checked);
	ui.lWillMessageType->setEnabled(enabled);
	ui.cbWillMessageType->setEnabled(enabled);
	ui.lWillOwnMessage->setEnabled(enabled);
	ui.leWillOwnMessage->setEnabled(enabled);
	ui.lWillStatistics->setEnabled(enabled);
	ui.lwWillStatistics->setEnabled(enabled);
	ui.lWillTopic->setEnabled(enabled);
	ui.cbWillTopic->setEnabled(enabled);
	ui.lWillQoS->setEnabled(enabled);
	ui.cbWillQoS->setEnabled(enabled);
	ui.lUseRetainMessage->setEnabled(enabled);
	ui.chbWillRetain->setEnabled(enabled);
	ui.lWillUpdateType->setEnabled(enabled);
	ui.cbWillUpdate->setEnabled(enabled);
	ui.lWillUpdateInterval->setEnabled(enabled);
	ui.leWillUpdateInterval->setEnabled(enabled);
}
