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

#include "worksheetcursor.h"

WorksheetCursor::WorksheetCursor()
{
    m_entry = 0;
    m_textItem = 0;
    m_textCursor = QTextCursor();
}

WorksheetCursor::WorksheetCursor(WorksheetEntry* entry, WorksheetTextItem* item,
                                 const QTextCursor& cursor)
{
    m_entry = entry;
    m_textItem = item;
    m_textCursor = cursor;
}

WorksheetCursor::~WorksheetCursor()
{
}

WorksheetEntry* WorksheetCursor::entry() const
{
    return m_entry;
}

WorksheetTextItem* WorksheetCursor::textItem() const
{
    return m_textItem;
}

QTextCursor WorksheetCursor::textCursor() const
{
    return m_textCursor;
}

bool WorksheetCursor::isValid() const
{
    return m_entry && m_textItem;
}

