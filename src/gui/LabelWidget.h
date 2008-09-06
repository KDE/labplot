#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "../ui_labelwidget.h"
class Label;

/**
 * @brief Widget for changing the properties of the Label object
 */
class LabelWidget: public QWidget{
	Q_OBJECT

public:
	LabelWidget(QWidget *parent);
	~LabelWidget();

signals:
	void dataChanged(bool);

public slots:
	void setLabel(const Label*);
	void setLabelRotationEnabled(const bool);
	void saveLabel(Label*) const;

private:
	Ui::LabelWidget ui;

private slots:
	void positionChanged(int);
	void fillingChanged(bool);
	void fillingColorClicked();

	void fontChanged(const QFont&);
	void textColorChanged(const QColor&);

	void useTexChanged(int);
	void fontBoldToggled(bool);
	void fontItalicToggled(bool);
	void fontUnderlineToggled(bool);
	void fontSuperscriptToggled(bool);
	void fontSubscriptToggled(bool);
	void insertSymbol(const QString &);
	void slotDataChanged();
};

#endif //LABELWIDGET_H
