#ifndef FUNCIONWIDGET_H
#define FUNCTIONWIDGET_H

#include "../ui_functionplotwidget.h"
#include "../plots/Plot.h"

class Set;
class LabelWidget;
class PlotStyleWidgetInterface;
class QMdiSubWindow;

/**
 * @brief Represents the widget where a function can be created/edited
 * This widget is embedded in \c FunctionPlotDialog.
 */
class FunctionPlotWidget : public QWidget{
    Q_OBJECT

public:
    FunctionPlotWidget(QWidget*);
    ~FunctionPlotWidget();

	void setSet(Set*);
	void saveSet(Set*);
	void setPlotType(const Plot::PlotType&);

private:
	Ui::FunctionPlotWidget ui;
	LabelWidget* labelWidget;
	PlotStyleWidgetInterface* plotStyleWidget;
	Plot::PlotType plotType;

	int createSetData(Set* set);

private slots:
	void insert(const QString s);
};

#endif
