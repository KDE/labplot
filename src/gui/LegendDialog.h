#ifndef LEGENDDIALOG_H
#define LEGENDDIALOG_H

#include <kdialog.h>

class MainWin;
class Legend;
class LegendWidget;

/**
 * @brief Provides a dialog for editing the legend settings.
 */
class LegendDialog: public KDialog{
	Q_OBJECT

public:
	LegendDialog(MainWin*, Legend* l=0);

private:
	LegendWidget* legendWidget;
	MainWin* mainWin;

private slots:
	void apply();
	void ok();
};

#endif //LEGENDDIALOG_H
