#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QtGui>
#include "../ui_legendwidget.h"

class Legend;

/**
 * @brief Represents a widget where all the legend settings can be modified
 * This widget is embedded in \c LegendDialog
 */
class LegendWidget : public QWidget{
    Q_OBJECT

public:
	LegendWidget(QWidget*);
	~LegendWidget();

	void setLegend(const Legend*);
	void saveLegend(Legend*) const;

private:
	Ui::LegendWidget ui;
	Legend* legend;
	bool initializing;

signals:
	void dataChanged(bool);

private slots:
	void fillingChanged(bool);
	void slotDataChanged();
};

#endif
