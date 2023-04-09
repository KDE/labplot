/*
	File		: MQTTTopic.cpp
	Project		: LabPlot
	Description	: Represents a topic of a MQTTSubscription
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/MQTTTopic.h"

#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"

#include <KLocalizedString>
#include <QAction>
#include <QIcon>
#include <QMenu>

/*!
  \class MQTTTopic
  \brief  Represents a topic of a subscription made in MQTTClient.

  \ingroup datasources
*/
MQTTTopic::MQTTTopic(const QString& name, MQTTSubscription* subscription, bool loading)
	: Spreadsheet(name, loading, AspectType::MQTTTopic)
	, m_topicName(name)
	, m_MQTTClient(subscription->mqttClient())
	, m_filter(new AsciiFilter) {
	auto mainFilter = m_MQTTClient->filter();

	m_filter->setAutoModeEnabled(mainFilter->isAutoModeEnabled());
	if (!mainFilter->isAutoModeEnabled()) {
		m_filter->setCommentCharacter(mainFilter->commentCharacter());
		m_filter->setSeparatingCharacter(mainFilter->separatingCharacter());
		m_filter->setDateTimeFormat(mainFilter->dateTimeFormat());
		m_filter->setCreateIndexEnabled(mainFilter->createIndexEnabled());
		m_filter->setCreateTimestampEnabled(mainFilter->createTimestampEnabled());
		m_filter->setSimplifyWhitespacesEnabled(mainFilter->simplifyWhitespacesEnabled());
		m_filter->setNaNValueToZero(mainFilter->NaNValueToZeroEnabled());
		m_filter->setRemoveQuotesEnabled(mainFilter->removeQuotesEnabled());
		m_filter->setSkipEmptyParts(mainFilter->skipEmptyParts());
		m_filter->setHeaderEnabled(mainFilter->isHeaderEnabled());
		QString vectorNames;
		const QStringList& filterVectorNames = mainFilter->vectorNames();
		for (int i = 0; i < filterVectorNames.size(); ++i) {
			vectorNames.append(filterVectorNames.at(i));
			if (i != vectorNames.size() - 1)
				vectorNames.append(QLatin1String(" "));
		}

		m_filter->setVectorNames(vectorNames);
		m_filter->setStartRow(mainFilter->startRow());
		m_filter->setEndRow(mainFilter->endRow());
		m_filter->setStartColumn(mainFilter->startColumn());
		m_filter->setEndColumn(mainFilter->endColumn());
	}

	connect(m_MQTTClient, &MQTTClient::readFromTopics, this, &MQTTTopic::read);
	qDebug() << "New MqttTopic: " << m_topicName;
	initActions();
}

MQTTTopic::~MQTTTopic() {
	qDebug() << "MqttTopic destructor:" << m_topicName;
	delete m_filter;
}

/*!
 *\brief Sets the MQTTTopic's filter
 * The ownership of the filter is passed to MQTTTopic.
 *
 * \param filter
 */
void MQTTTopic::setFilter(AsciiFilter* f) {
	delete m_filter;
	m_filter = f;
}

/*!
 *\brief Returns the MQTTTopic's filter
 */
AsciiFilter* MQTTTopic::filter() const {
	return m_filter;
}

/*!
 *\brief Returns the MQTTTopic's icon
 */
QIcon MQTTTopic::icon() const {
	return QIcon::fromTheme(QStringLiteral("text-plain"));
}

/*!
 *\brief Adds an action to the MQTTTopic's context menu in the project explorer
 */
QMenu* MQTTTopic::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	// there're already actions available there. Skip the first title-action
	// and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_plotDataAction);
	menu->insertSeparator(firstAction);

	return menu;
}

QWidget* MQTTTopic::view() const {
	if (!m_partView)
		m_partView = new SpreadsheetView(const_cast<MQTTTopic*>(this), true);
	return m_partView;
}

/*!
 *\brief Adds a message received by the topic to the message puffer
 */
void MQTTTopic::newMessage(const QString& message) {
	m_messagePuffer.push_back(message);
}

/*!
 *\brief Returns the name of the MQTTTopic
 */
QString MQTTTopic::topicName() const {
	return m_topicName;
}

/*!
 *\brief Initializes the actions of MQTTTopic
 */
void MQTTTopic::initActions() {
	m_plotDataAction = new QAction(QIcon::fromTheme(QStringLiteral("office-chart-line")), i18n("Plot data"), this);
	connect(m_plotDataAction, &QAction::triggered, this, &MQTTTopic::plotData);
}

/*!
 *\brief Returns the MQTTClient the topic belongs to
 */
MQTTClient* MQTTTopic::mqttClient() const {
	return m_MQTTClient;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

/*!
 *\brief Plots the data stored in MQTTTopic
 */
void MQTTTopic::plotData() {
	auto* dlg = new PlotDataDialog(this);
	dlg->exec();
}

/*!
 *\brief Reads every message from the message puffer
 */
void MQTTTopic::read() {
	while (!m_messagePuffer.isEmpty()) {
		qDebug() << "Reading from topic " << m_topicName;
		const QString tempMessage = m_messagePuffer.takeFirst();
		m_filter->readMQTTTopic(tempMessage, this);
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/*!
  Saves as XML.
 */
void MQTTTopic::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("MQTTTopic"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("topicName"), m_topicName);
	writer->writeAttribute(QStringLiteral("filterPrepared"), QString::number(m_filter->isPrepared()));
	writer->writeAttribute(QStringLiteral("filterSeparator"), m_filter->separator());
	writer->writeAttribute(QStringLiteral("messagePufferSize"), QString::number(m_messagePuffer.size()));
	for (int i = 0; i < m_messagePuffer.count(); ++i)
		writer->writeAttribute(QStringLiteral("message") + QString::number(i), m_messagePuffer[i]);
	writer->writeEndElement();

	// filter
	m_filter->save(writer);

	// Columns
	for (auto* col : children<Column>(AbstractAspect::ChildIndexFlag::IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); // MQTTTopic
}

/*!
  Loads from XML.
*/
bool MQTTTopic::load(XmlStreamReader* reader, bool preview) {
	removeColumns(0, columnCount());
	if (!readBasicAttributes(reader))
		return false;

	bool isFilterPrepared = false;
	QString separator;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("MQTTTopic"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("topicName")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'topicName'")));
			else {
				m_topicName = str;
				setName(str);
			}

			str = attribs.value(QStringLiteral("filterPrepared")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'filterPrepared'")));
			else {
				isFilterPrepared = str.toInt();
			}

			str = attribs.value(QStringLiteral("filterSeparator")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'filterSeparator'")));
			else {
				separator = str;
			}

			int pufferSize = 0;
			str = attribs.value(QStringLiteral("messagePufferSize")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg(QStringLiteral("'messagePufferSize'")));
			else
				pufferSize = str.toInt();
			for (int i = 0; i < pufferSize; ++i) {
				str = attribs.value(QStringLiteral("message") + QString::number(i)).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.arg(QStringLiteral("'message") + QString::number(i) + QLatin1Char('\'')));
				else
					m_messagePuffer.push_back(str);
			}
		} else if (reader->name() == QLatin1String("asciiFilter")) {
			if (!m_filter->load(reader))
				return false;
		} else if (reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Text);
			if (!column->load(reader, preview)) {
				delete column;
				setColumnCount(0);
				return false;
			}
			addChild(column);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// prepare filter for reading
	m_filter->setPreparedForMQTT(isFilterPrepared, this, separator);

	return !reader->hasError();
}
