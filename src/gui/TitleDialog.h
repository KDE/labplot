#ifndef TITLEDIALOG_H
#define TITLEDIALOG_H

#include <KDialog>

class MainWin;
class LabelWidget;
class Label;

class TitleDialog: public KDialog{
	Q_OBJECT

public:
	TitleDialog(MainWin *mw, Label *title=0);

private:
	MainWin* mainWin;
	LabelWidget* labelWidget;
	Label *title;

private slots:
	void apply();
	void save();
};

#endif //TITLEDIALOG_H
