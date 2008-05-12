#ifndef PLOTSURFACESTYLEWIDGET_H
#define PLOTSURFACESTYLEWIDGET_H

#include "../ui_plotsurfacestylewidget.h"
#include "PlotStyleWidget.h"

/**
 * @brief Represents the widget where all the style settings of a surface plot can be modified
 * This widget is embedded in \c FunctionWidget.
 */
class PlotSurfaceStyleWidget : public QWidget, public PlotStyleWidgetInterface{
    Q_OBJECT

public:
    PlotSurfaceStyleWidget(QWidget*);
    ~PlotSurfaceStyleWidget();
	void setStyle(const Style* );
	void saveStyle(Style*) const;

private:
	Ui::PlotSurfaceStyleWidget ui;

private slots:
	void selectColorMap();
};

#endif
