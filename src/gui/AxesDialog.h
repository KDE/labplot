#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <kdialog.h>
#include "../plots/Plot.h"

class AxesWidget;
class Axis;
class MainWin;

/**
 * @brief Provides a dialog for editing the axis settings.
 */
class AxesDialog: public KDialog{
	Q_OBJECT

public:
	AxesDialog(MainWin*, const Plot::PlotType type=Plot::PLOT2D);
	~AxesDialog();

	void setAxesData(const QList<Axis> list_axes, const int axisNumber=0) const;
	void saveAxesData() const;

private:
	AxesWidget* axesWidget;
	QList<Axis>* list_axes;

private slots:
	void apply();
	void save();
};

#endif //AXESDIALOG_H
