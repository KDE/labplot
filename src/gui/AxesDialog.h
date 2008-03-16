/*!
	provides a dialog for the AxisWidget for editing the axis settings.
*/

#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <QObject>
#include "kdialog.h"
#include "../plottype.h"

class AxesWidget;
class Axis;
class MainWin;

class AxesDialog: public KDialog{
	Q_OBJECT

public:
	AxesDialog(MainWin*, const PlotType type=PLOT2D);
	~AxesDialog();

	void setAxesData(const QList<Axis> list_axes, const int axisNumber=0) const;
	void saveAxesData() const;

private:
	AxesWidget* axesWidget;
	QList<Axis>* list_axes;
};

#endif //AXESDIALOG_H
