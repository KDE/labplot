#ifndef PLOTSTYLEWIDGET_H
#define PLOTSTYLEWIDGET_H

// #include <QtGui>

#include "../ui_plotstylewidget.h"

/**
 * @brief Represents the widget where all the axes setting can be modified
 * This widget is embedded in \c AxisDialog.
 */
class PlotStyleWidget : public QWidget{
    Q_OBJECT

public:
    PlotStyleWidget(QWidget*);
    ~PlotStyleWidget();
/*
	void setAxesData(const QList<Axis>&, const int axisNumber=0);
	void saveAxesData(QList<Axis>*) const;
	void apply(QList<Axis>*) const;
	void setPlotType(const PlotType& );*/

private:
	Ui::PlotStyleWidget ui;

private slots:
// 	void currentAxisChanged(int);

};

#endif
