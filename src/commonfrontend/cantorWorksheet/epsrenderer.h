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

#ifndef EPSRENDERER_H
#define EPSRENDERER_H

#include <QTextDocument>
#include <QTextImageFormat>
#include <QPixmap>
#include <QSizeF>
#include <QUrl>
#include "latexrenderer.h"

class EpsRenderer
{
  public:
    EpsRenderer();
    ~EpsRenderer();

    enum FormulaProperties {CantorFormula = 1, ImagePath = 2, Code = 3,
                            Delimiter = 4};
    enum FormulaType {LatexFormula = Cantor::LatexRenderer::LatexMethod,
                      MmlFormula = Cantor::LatexRenderer::MmlMethod};

    QTextImageFormat render(QTextDocument *document, const QUrl& url);
    QTextImageFormat render(QTextDocument *document,
                            const Cantor::LatexRenderer* latex);

    void setScale(qreal scale);
    qreal scale();

    void useHighResolution(bool b);

    QSizeF renderToResource(QTextDocument *document, const QUrl& url);
    QImage renderToImage(const QUrl& url, QSizeF* size = 0);

  private:
    double m_scale;
    bool m_useHighRes;
};

#endif //EPSRENDERER_H
