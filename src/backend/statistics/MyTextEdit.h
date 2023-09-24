/***************************************************************************
    File                 : MyTextEdit.h
    Project              : LabPlot
    Description          : Derived class of QTextEdit to add ToolTip Functionality
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>

class MyTextEdit : public QTextEdit {
    Q_OBJECT

public:
    typedef QTextEdit inherited;
    explicit MyTextEdit(QWidget* parent = nullptr);
    void setHtml(QString text);

    void extractToolTips(QString& text, bool insert = true);

protected Q_SLOTS:
    bool event(QEvent *e);

private:
	QMap<std::pair<double, double>, QString> m_tooltips;
};

#endif // MYTEXTEDIT_H
