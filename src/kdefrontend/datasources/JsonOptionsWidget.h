#ifndef JSONOPTIONSWIDGET_H
#define JSONOPTIONSWIDGET_H

#include "ui_jsonoptionswidget.h"

class JsonFilter;

class JsonOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit JsonOptionsWidget(QWidget*);
	void applyFilterSettings(JsonFilter*) const;
	void loadSettings() const;
	void saveSettings();

public slots:

private:
	void setTooltips();
	Ui::JsonOptionsWidget ui;
};

#endif
