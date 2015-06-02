/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef WORKSHEETCURSOR_H
#define WORKSHEETCURSOR_H

#include <QTextCursor>

class WorksheetEntry;
class WorksheetTextItem;

class WorksheetCursor
{
  public:
    WorksheetCursor();
    WorksheetCursor(WorksheetEntry*, WorksheetTextItem*, const QTextCursor&);
    ~WorksheetCursor();

    WorksheetEntry* entry() const;
    WorksheetTextItem* textItem() const;
    QTextCursor textCursor() const;
    
    bool isValid() const;

  private:
    WorksheetEntry* m_entry;
    WorksheetTextItem* m_textItem;
    QTextCursor m_textCursor;
};

#endif // WORKSHEETCURSOR_H
