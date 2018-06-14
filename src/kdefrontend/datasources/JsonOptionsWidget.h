#ifndef JSONOPTIONSWIDGET_H
#define JSONOPTIONSWIDGET_H

#include <backend/datasources/filters/JsonFilter.h>
#include "ui_jsonoptionswidget.h"

class ImportFileWidget;
class JsonFilter;
class QJsonModel;
class QJsonTreeItem;

class JsonOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit JsonOptionsWidget(QWidget*, ImportFileWidget*);
	void applyFilterSettings(JsonFilter*) const;
	void clear();
	void loadSettings() const;
	void saveSettings();
	QJsonDocument selectedJson() const;

public slots:
	void updateContent();
private slots:
	void indexChanged();
private:
	void setTooltips();
	JsonFilter::DataContainerType getCurrentType() const;
	Ui::JsonOptionsWidget ui;
	ImportFileWidget* m_fileWidget;
	QJsonModel* m_model;
	QJsonTreeItem* m_lastItem;
};

#endif
