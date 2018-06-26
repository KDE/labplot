#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#ifdef HAVE_MQTT
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/datasources/filters/AbstractFileFilter.h"

class MQTTSubscriptions;
class MQTTClient;

class MQTTTopic : public Spreadsheet{
	Q_OBJECT

public:
	MQTTTopic(AbstractScriptingEngine*, const QString& name, AbstractAspect* subscription, bool loading = false);
	~MQTTTopic() override;

	void setFilter(AbstractFileFilter*);
	AbstractFileFilter* filter() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	QString name() const;
	AbstractAspect* mqttClient() const;

	void newMessage(const QString&);
	void read();

	int readingType() const;
	int sampleRate() const;
	bool isPaused() const;
	int updateInterval() const;
	int keepNvalues() const;
	bool keepLastValues() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();

	QString m_topicName;
	AbstractAspect* m_MQTTClient;

	AbstractFileFilter* m_filter;
	QVector<QString> m_messagePuffer;

	QAction* m_reloadAction;
	QAction* m_showEditorAction;
	QAction* m_showSpreadsheetAction;
	QAction* m_plotDataAction;


public slots:

private slots:
	void plotData();
	void removeMessage();

signals:
	void readOccured();

};

#endif
#endif // MQTTTOPIC_H
