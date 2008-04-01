#ifndef FUNCIONWIDGET_H
#define FUNCTIONWIDGET_H

#include "../ui_functionwidget.h"
#include "../plottype.h"

class Function;
class LabelWidget;
class PlotStyleWidget;

/**
 * @brief Represents the widget where a function can be created/edited
 * This widget is embedded in \c FunctionDialog.
 */
class FunctionWidget : public QWidget{
    Q_OBJECT

public:
    FunctionWidget(QWidget*);
    ~FunctionWidget();

	void setFunction(const Function&);
	void saveFunction(Function*) const;
	void setPlotType(const PlotType& );

private:
	Ui::FunctionWidget ui;
	LabelWidget* labelWidget;
	PlotStyleWidget* plotStyleWidget;

	PlotType plotType;

private slots:
	void insert(const QString s);
};

#endif
