#ifndef LEGENDDIALOG_H
#define LEGENDDIALOG_H

#include <KDialog>

class Legend;
class LegendWidget;
class Worksheet;

/**
 * @brief Provides a dialog for editing the legend settings.
 */
class LegendDialog: public KDialog{
	Q_OBJECT

public:
	LegendDialog(QWidget*);
	void setWorksheet(Worksheet*);

private:
	LegendWidget* legendWidget;
	Worksheet* worksheet;

private slots:
	void apply();
	void save();
};

#endif //LEGENDDIALOG_H
