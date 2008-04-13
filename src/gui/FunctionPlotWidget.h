#ifndef FUNCIONWIDGET_H
#define FUNCTIONWIDGET_H

#include "../ui_functionplotwidget.h"
#include "../plottype.h"

class Set;
class LabelWidget;
class PlotStyleWidget;
class PlotSurfaceStyleWidget;
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
	void setPlotType(const PlotType&);

private:
	Ui::FunctionPlotWidget ui;
	LabelWidget* labelWidget;
	PlotStyleWidget* plotStyleWidget;
	PlotSurfaceStyleWidget* plotSurfaceStyleWidget;
	PlotType plotType;

private slots:
	void insert(const QString s);
};

#endif
