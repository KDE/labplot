#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include <QtGui>

#include "../ui_axeswidget.h"
#include "../plottype.h"
#include "../Axis.h"

/**
 * @brief Represents the widget where all the axes setting can be modified
 * This widget is embedded in \c AxisDialog.
 */
class AxesWidget : public QWidget{
    Q_OBJECT

public:
    AxesWidget(QWidget*);
    ~AxesWidget();

	void setAxesData(const QList<Axis>&, const int axisNumber=0);
	void saveAxesData(QList<Axis>*) const;
	void apply(QList<Axis>*) const;
	void setPlotType(const PlotType& );

private:
	Ui::AxesWidget ui;

	QList<Axis> listAxes;
	PlotType plotType;
	int currentAxis;

public slots:
	void restoreDefaults();

private slots:
	void currentAxisChanged(int);

	//"General"-tab


	//"Ticks"-tab
	void ticksStyleChanged(int);
	void ticksColourClicked();

	//"Tick labels"-tab
	void labelFontClicked();
	void labelColourClicked();

	//"Grid"-tab
	void createMajorGridStyles();
	void createMinorGridStyles();
};

#endif
