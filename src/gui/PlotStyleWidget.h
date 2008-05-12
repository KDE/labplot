#ifndef PLOTSTYLEWIDGET_H
#define PLOTSTYLEWIDGET_H

#include "../ui_plotstylewidget.h"

class Style;

class PlotStyleWidgetInterface{
public:
	virtual void setStyle(const Style* )=0;
	virtual void saveStyle(Style*) const=0;
	virtual ~PlotStyleWidgetInterface(){}
};


/**
 * @brief Represents the widget where all the style setting of a plot can be modified.
 * This widget is embedded in \c FunctionWidget.
 */
class PlotStyleWidget : public QWidget, public PlotStyleWidgetInterface{
    Q_OBJECT

public:
    PlotStyleWidget(QWidget*);
    ~PlotStyleWidget();
	void setStyle(const Style* );
	void saveStyle(Style*) const;

private:
	Ui::PlotStyleWidget ui;

private slots:
	void symbolTypeChanged(int);
	void symbolFillingColorChanged();

	void fillLineStyleBox();
	void fillAreaFillingPatternBox();
	void fillSymbolTypeBox();
	void fillSymbolFillingBox();
	void fillSymbolFillingPatternBox();

	void boxWidthStateChanged(int);
};

#endif
