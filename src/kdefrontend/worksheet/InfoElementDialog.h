#ifndef INFOELEMENTDIALOG_H
#define INFOELEMENTDIALOG_H

#include <QDialog>

class CartesianPlot;
class XYCurve;
class QListWidgetItem;

namespace Ui {
class InfoElementDialog;
}

class InfoElementDialog : public QDialog
{
	Q_OBJECT

public:
	explicit InfoElementDialog(QWidget *parent = nullptr);
	~InfoElementDialog();
	void setPlot(CartesianPlot* plot);
	void setActiveCurve(const XYCurve* curve, double pos);
	void updateSettings();
private:
	void createElement();
	void updateSelectedCurveLabel(QListWidgetItem *item);
	void removePlot();
	void validateSettings();

private:
	Ui::InfoElementDialog *ui;
	CartesianPlot* m_plot{nullptr};


};

#endif // INFOELEMENTDIALOG_H
