#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include "ui_statisticswidget.h"
#include <KDialog>

class Column;

class StatisticsDialog : public KDialog {
	Q_OBJECT
public:
	explicit StatisticsDialog(const QString&, QWidget *parent = 0);
	void setColumns(const QList<Column*>& columns);

private:
	const QString isNanValue(const double value);
	Ui::StatisticsDialog ui;
	QString m_htmlText;
	QList<Column*> m_columns;
	QSize sizeHint() const;

private slots:
	void currentTabChanged(int index);
};

#endif
