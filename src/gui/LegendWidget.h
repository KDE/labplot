#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QtGui>
#include "../ui_legendwidget.h"

class Legend;

/**
 * @brief Represents the widget where all the legend settings can be modified
 * This widget is embedded in \c LegendDialog and in \c ProjectManagerWidget
 */
class LegendWidget : public QWidget{
    Q_OBJECT

public:
	LegendWidget(QWidget*);
	~LegendWidget();

	void setLegend(Legend*);
	void save();

private:
	Ui::LegendWidget ui;
	Legend* legend;

private slots:
	void fillingChanged(bool);
};

#endif
