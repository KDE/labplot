#ifdef HAVE_MQTT

#include "backend/datasources/MQTTTopic.h"

#include "backend/datasources/MQTTSubscriptions.h"
#include "backend/datasources/MQTTClient.h"

#include "backend/core/Project.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"

#include "backend/datasources/filters/AsciiFilter.h"

#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTimer>
#include <QMessageBox>

#include <QIcon>
#include <QAction>
#include <KLocalizedString>

#include <QDebug>

MQTTTopic::MQTTTopic(AbstractScriptingEngine* engine, const QString& name, AbstractAspect* subscription, bool loading)
	: Spreadsheet(engine, name, loading),
	  m_client(dynamic_cast<MQTTSubscriptions*> (subscription)->mqttClient()),
	  m_topicName(name),
	  m_filter(new AsciiFilter()) {
	AsciiFilter * mainFilter = dynamic_cast<AsciiFilter*>(dynamic_cast<MQTTClient*>(m_client)->filter());
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

	connect(dynamic_cast<MQTTClient*>(m_client), &MQTTClient::readFromTopics, this, &MQTTTopic::read);
	initActions();
}

MQTTTopic::~MQTTTopic() {

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
	return static_cast<int> (dynamic_cast<MQTTClient*>(m_client)->readingType());
}

int MQTTTopic::sampleRate() const {
	return dynamic_cast<MQTTClient*>(m_client)->sampleRate();
}

bool  MQTTTopic::isPaused() const {
	return dynamic_cast<MQTTClient*>(m_client)->isPaused();
}

int MQTTTopic::updateInterval() const {
	return dynamic_cast<MQTTClient*>(m_client)->updateInterval();
}

int MQTTTopic::keepNvalues() const {
	return dynamic_cast<MQTTClient*>(m_client)->keepNvalues();
}

bool MQTTTopic::keepLastValues() const {
	return dynamic_cast<MQTTClient*>(m_client)->keepLastValues();
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

QString MQTTTopic::name() const{
	return m_topicName;
}

void MQTTTopic::save(QXmlStreamWriter*) const {

}

bool MQTTTopic::load(XmlStreamReader*, bool preview) {
	return true;
}

AbstractAspect* MQTTTopic::mqttClient() const{
	return m_client;
}

void MQTTTopic::removeMessage() {
	m_messagePuffer.removeFirst();
}

#endif
