#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "../ui_labelwidget.h"
class Label;
class KFontRequester;
class KColorButton;

/**
 * @brief Widget for changing the properties of the Label object
 */
class LabelWidget: public QWidget{
	Q_OBJECT

public:
	LabelWidget(QWidget *parent);
	~LabelWidget();

public slots:
	void setLabel(const Label*);
	void setLabelRotationEnabled(const bool);
	void saveLabel(Label*) const;

private:
	Ui::LabelWidget ui;
	KFontRequester* fontRequester;
	KColorButton* colorButton;

private slots:
	void positionChanged(int);
	void fillingChanged(bool);
	void fillingColorClicked();

	void fontChanged(const QFont&);
	void fontColorChanged(const QColor&);

	void useTexChanged(int);
	void fontBoldToggled(bool);
	void fontItalicToggled(bool);
	void fontUnderlineToggled(bool);
	void fontSuperscriptToggled(bool);
	void fontSubscriptToggled(bool);
	void insertSymbol(const QString &);
};

#endif //LABELWIDGET_H
