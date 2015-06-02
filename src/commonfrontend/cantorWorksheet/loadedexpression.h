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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _LOADEDEXPRESSION_H
#define _LOADEDEXPRESSION_H

#include "cantor/expression.h"

#include <KZip>
#include <QDomElement>

/** This class is used to hold expressions
    loaded from a file. they can't be evauluated
    and only show the result, they loaded from xml.
    this is used to avoid most exceptions when
    dealing with loaded Worksheets instead of newly
    created ones.
**/
class LoadedExpression : public Cantor::Expression
{
  public:
    LoadedExpression( Cantor::Session* session );
    ~LoadedExpression();

    void evaluate();
    void interrupt();

    void loadFromXml(const QDomElement& xml, const KZip& file);
};

#endif /* _LOADEDEXPRESSION_H */
