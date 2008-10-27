/***************************************************************************
    File                 : LabelWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : label settings widget
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "ui_labelwidget.h"
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
