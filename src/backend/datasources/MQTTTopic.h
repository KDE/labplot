#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#ifdef HAVE_MQTT
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/datasources/filters/AbstractFileFilter.h"

class MQTTSubscription;
class MQTTClient;

class MQTTTopic : public Spreadsheet{
	Q_OBJECT

public:
	MQTTTopic(const QString& name, MQTTSubscription* subscription, bool loading = false);
	~MQTTTopic() override;

	void setFilter(AbstractFileFilter*);
	AbstractFileFilter* filter() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	QString topicName() const;
	MQTTClient* mqttClient() const;
	void newMessage(const QString&);
	int readingType() const;
	int sampleSize() const;
	bool isPaused() const;
	int updateInterval() const;
	int keepNValues() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();

	QString m_topicName;
	MQTTClient* m_MQTTClient;

	AbstractFileFilter* m_filter;
	QVector<QString> m_messagePuffer;

	QAction* m_reloadAction;
	QAction* m_showEditorAction;
	QAction* m_showSpreadsheetAction;
	QAction* m_plotDataAction;

public slots:	
	void read();

private slots:
	void plotData();

signals:
	void readOccured();

};

#endif // HAVE_MQTT
#endif // MQTTTOPIC_H
