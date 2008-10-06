/***************************************************************************
    File                 : PatternBox.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Pattern combo box
                           
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
#ifndef PATTERNBOX_H
#define PATTERNBOX_H

#include <QComboBox>

//! Pattern combo box
class PatternBox : public QComboBox
{
  Q_OBJECT
public:
  PatternBox(bool rw, QWidget *parent);
  PatternBox(QWidget *parent);
  void setPattern(const Qt::BrushStyle& c);
  Qt::BrushStyle getSelectedPattern() const;

  static const Qt::BrushStyle patterns[];
  static int patternIndex(const Qt::BrushStyle& style);

protected:
  void init();
};

#endif
