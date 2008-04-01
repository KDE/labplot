#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include <QtGui>

#include "../ui_axeswidget.h"
#include "../plottype.h"
#include "../elements/Axis.h"

class LabelWidget;

/**
 * @brief Represents the widget where all the axes setting can be modified
 * This widget is embedded in \c AxisDialog.
 */
class AxesWidget : public QWidget{
    Q_OBJECT

public:
    AxesWidget(QWidget*);
    ~AxesWidget();

	void setAxesList( const QList<Axis>, const int axisNumber=0);
	void saveAxesData(QList<Axis>*) const;
	void apply(QList<Axis>*) const;
	void setPlotType(const PlotType& );

private:
	Ui::AxesWidget ui;
	LabelWidget* labelWidget	;
	QList<Axis> listAxes;
	PlotType plotType;
	int currentAxis;
	bool dataChanged;

public slots:
	void restoreDefaults();

private slots:
	void currentAxisChanged(int);

	//"General"-tab


	//"Ticks"-tab
	void ticksStyleChanged(int);

	//"Tick labels"-tab
	void labelFormatChanged(const QString&);
	void majorTicksChanged(int index);
	void minorTicksChanged(int index);

	//"Grid"-tab
	void createMajorGridStyles();
	void createMinorGridStyles();
};

#endif
