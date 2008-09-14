#ifndef TITLEDIALOG_H
#define TITLEDIALOG_H

#include <KDialog>

class LabelWidget;
class Label;
class Worksheet;

class TitleDialog: public KDialog{
	Q_OBJECT

public:
	TitleDialog(QWidget*);
	void setWorksheet(Worksheet*);

private:
	LabelWidget* labelWidget;
	Worksheet* worksheet;

private slots:
	void apply();
	void save();
};

#endif //TITLEDIALOG_H
