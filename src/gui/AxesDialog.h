#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <KDialog>
#include "../plots/Plot.h"

class AxesWidget;
class Axis;
class MainWin;
class Worksheet;

/**
 * @brief Provides a dialog for editing the axis settings.
 */
class AxesDialog: public KDialog{
	Q_OBJECT

public:
	AxesDialog(MainWin*, const Plot::PlotType type=Plot::PLOT2D);
	~AxesDialog();

	void setWorksheet(Worksheet*);
	void setAxes(QList<Axis>* list_axes, const int axisNumber=0);
	void saveAxes() const;

private:
	AxesWidget* axesWidget;
	QList<Axis>* list_axes;
	Worksheet* worksheet;

private slots:
	void apply();
	void save();
};

#endif //AXESDIALOG_H
