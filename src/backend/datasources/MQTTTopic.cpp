#ifdef HAVE_MQTT

#include "backend/datasources/MQTTTopic.h"

#include "backend/datasources/MQTTSubscriptions.h"
#include "backend/datasources/MQTTClient.h"

#include "backend/core/Project.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include "backend/datasources/filters/AsciiFilter.h"

#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>

#include <QIcon>
#include <QAction>
#include <KLocalizedString>

#include <QDebug>

MQTTTopic::MQTTTopic(AbstractScriptingEngine* engine, const QString& name, AbstractAspect* subscription, bool loading)
	: Spreadsheet(engine, name, loading),
	  m_MQTTClient(dynamic_cast<MQTTSubscriptions*> (subscription)->mqttClient()),
	  m_topicName(name),
	  m_filter(new AsciiFilter()) {
	AsciiFilter * mainFilter = dynamic_cast<AsciiFilter*>(dynamic_cast<MQTTClient*>(m_MQTTClient)->filter());
	dynamic_cast<AsciiFilter*> (m_filter)->setAutoModeEnabled(mainFilter->isAutoModeEnabled());
	if(!mainFilter->isAutoModeEnabled()) {
		dynamic_cast<AsciiFilter*> (m_filter)->setCommentCharacter(mainFilter->commentCharacter());
		dynamic_cast<AsciiFilter*> (m_filter)->setSeparatingCharacter(mainFilter->separatingCharacter());
		dynamic_cast<AsciiFilter*> (m_filter)->setDateTimeFormat(mainFilter->dateTimeFormat());
		dynamic_cast<AsciiFilter*> (m_filter)->setCreateIndexEnabled(mainFilter->createIndexEnabled());
		dynamic_cast<AsciiFilter*> (m_filter)->setSimplifyWhitespacesEnabled(mainFilter->simplifyWhitespacesEnabled());
		dynamic_cast<AsciiFilter*> (m_filter)->setNaNValueToZero(mainFilter->NaNValueToZeroEnabled());
		dynamic_cast<AsciiFilter*> (m_filter)->setRemoveQuotesEnabled(mainFilter->removeQuotesEnabled());
		dynamic_cast<AsciiFilter*> (m_filter)->setSkipEmptyParts(mainFilter->skipEmptyParts());
		dynamic_cast<AsciiFilter*> (m_filter)->setHeaderEnabled(mainFilter->isHeaderEnabled());
		QString vectorNames;
		QStringList filterVectorNames = mainFilter->vectorNames();
		for(int i = 0; i < vectorNames.size(); ++i) {
			vectorNames.append(filterVectorNames.at(i));
			if(i != vectorNames.size() - 1)
				vectorNames.append(" ");
		}
		dynamic_cast<AsciiFilter*> (m_filter)->setVectorNames(vectorNames);
		dynamic_cast<AsciiFilter*> (m_filter)->setStartRow( mainFilter->startRow());
		dynamic_cast<AsciiFilter*> (m_filter)->setEndRow( mainFilter->endRow());
		dynamic_cast<AsciiFilter*> (m_filter)->setStartColumn( mainFilter->startColumn());
		dynamic_cast<AsciiFilter*> (m_filter)->setEndColumn( mainFilter->endColumn());
	}

	connect(dynamic_cast<MQTTClient*>(m_MQTTClient), &MQTTClient::readFromTopics, this, &MQTTTopic::read);
	qDebug()<<"MqttTopic constructor:"<<m_topicName;
	initActions();
}

MQTTTopic::~MQTTTopic() {
	qDebug()<<"MqttTopic destructor:"<<m_topicName;
}

void MQTTTopic::setFilter(AbstractFileFilter* f) {
	m_filter = f;
}

AbstractFileFilter* MQTTTopic::filter() const {
	return m_filter;
}

QIcon MQTTTopic::icon() const {
	QIcon icon;
	//if (m_fileType == LiveDataSource::Ascii)
	icon = QIcon::fromTheme("text-plain");

	return icon;
}

QMenu* MQTTTopic::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
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

void MQTTTopic::initActions() {
	/*m_reloadAction = new QAction(QIcon::fromTheme("view-refresh"), i18n("Reload"), this);
	connect(m_reloadAction, &QAction::triggered, this, &LiveDataSource::read);*/

	m_plotDataAction = new QAction(QIcon::fromTheme("office-chart-line"), i18n("Plot data"), this);
	connect(m_plotDataAction, &QAction::triggered, this, &MQTTTopic::plotData);
}

void MQTTTopic::plotData() {
	PlotDataDialog* dlg = new PlotDataDialog(this);
	dlg->exec();
}

int MQTTTopic::readingType() const {
	return static_cast<int> (dynamic_cast<MQTTClient*>(m_MQTTClient)->readingType());
}

int MQTTTopic::sampleRate() const {
	return dynamic_cast<MQTTClient*>(m_MQTTClient)->sampleRate();
}

bool  MQTTTopic::isPaused() const {
	return dynamic_cast<MQTTClient*>(m_MQTTClient)->isPaused();
}

int MQTTTopic::updateInterval() const {
	return dynamic_cast<MQTTClient*>(m_MQTTClient)->updateInterval();
}

int MQTTTopic::keepNvalues() const {
	return dynamic_cast<MQTTClient*>(m_MQTTClient)->keepNvalues();
}

void MQTTTopic::newMessage(const QString& message) {
	m_messagePuffer.push_back(message);
}

void MQTTTopic::read() {
	while(!m_messagePuffer.isEmpty()) {
		qDebug()<< "reading from topic " + m_topicName;
		QString tempMessage = m_messagePuffer.takeFirst();
		dynamic_cast<AsciiFilter*>(m_filter)->readMQTTTopic(tempMessage, m_topicName, this);
	}
}

QString MQTTTopic::topicName() const{
	return m_topicName;
}

void MQTTTopic::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("MQTTTopic");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("topicName", m_topicName);
	writer->writeAttribute("filterPrepared", QString::number(dynamic_cast<AsciiFilter*>(m_filter)->isPrepared()));
	writer->writeAttribute("filterSeparator", dynamic_cast<AsciiFilter*>(m_filter)->separator());
	writer->writeAttribute("messagePufferSize", QString::number(m_messagePuffer.size()));
	for(int i = 0; i < m_messagePuffer.count(); ++i)
		writer->writeAttribute("message"+QString::number(i), m_messagePuffer[i]);
	writer->writeEndElement();

	//filter
	m_filter->save(writer);

	//Columns
	for (auto* col : children<Column>(IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); //MQTTTopic
}

bool MQTTTopic::load(XmlStreamReader* reader, bool preview) {
	qDebug()<<"Start loading MQTTTopic";
	removeColumns(0, columnCount());
	if (!readBasicAttributes(reader))
		return false;

	bool isFilterPrepared = false;
	QString separator = "";

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "MQTTTopic")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "general") {
			qDebug()<<"MQTTTopic general";
			attribs = reader->attributes();

			str = attribs.value("topicName").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'topicName'"));
			else {
				m_topicName =  str;
				setName(str);
			}

			str = attribs.value("filterPrepared").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'filterPrepared'"));
			else {
				isFilterPrepared =  str.toInt();
			}

			str = attribs.value("filterSeparator").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'filterSeparator'"));
			else {
				separator =  str;
			}

			int pufferSize = 0;
			str = attribs.value("messagePufferSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'messagePufferSize'"));
			else
				pufferSize = str.toInt();
			for(int i = 0; i < pufferSize; ++i)
			{
				str = attribs.value("message"+QString::number(i)).toString();
				if(str.isEmpty())
					reader->raiseWarning(attributeWarning.arg("'message"+QString::number(i)+"'"));
				else
					m_messagePuffer.push_back(str);
			}
		} else if (reader->name() == "asciiFilter") {
			qDebug()<<"MQTTTopic filter load";
			if (!m_filter->load(reader))
				return false;
		} else if(reader->name() == "column") {
			qDebug()<<"MQTTTopic column load";
			Column* column = new Column("", AbstractColumn::Text);
			if (!column->load(reader, preview)) {
				delete column;
				setColumnCount(0);
				return false;
			}
			addChild(column);
		} else {// unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	dynamic_cast<AsciiFilter*>(m_filter)->setPreparedForMQTT(isFilterPrepared, this, separator);
	qDebug()<<"End loading MQTTTopic";
	return !reader->hasError();
}

AbstractAspect* MQTTTopic::mqttClient() const{
	return m_MQTTClient;
}

void MQTTTopic::removeMessage() {
	m_messagePuffer.removeFirst();
}

#endif
