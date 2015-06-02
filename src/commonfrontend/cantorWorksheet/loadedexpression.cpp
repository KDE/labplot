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

#include "loadedexpression.h"

#include "cantor/imageresult.h"
#include "cantor/epsresult.h"
#include "cantor/textresult.h"
#include "cantor/latexresult.h"
#include "cantor/animationresult.h"

#include <KGlobal>
#include <QDir>

LoadedExpression::LoadedExpression( Cantor::Session* session ) : Cantor::Expression( session )
{

}

LoadedExpression::~LoadedExpression()
{

}

void LoadedExpression::interrupt()
{
    //Do nothing
}

void LoadedExpression::evaluate()
{
    //Do nothing
}

void LoadedExpression::loadFromXml(const QDomElement& xml, const KZip& file)
{
    setCommand(xml.firstChildElement(QLatin1String("Command")).text());

    QDomElement resultElement=xml.firstChildElement(QLatin1String("Result"));
    Cantor::Result* result=0;
    const QString& type=resultElement.attribute(QLatin1String("type"));
    if ( type == QLatin1String("text"))
    {
        result=new Cantor::TextResult(resultElement.text());
    }
    else if (type == QLatin1String("image") || type == QLatin1String("latex") || type == QLatin1String("animation"))
    {
        const KArchiveEntry* imageEntry=file.directory()->entry(resultElement.attribute(QLatin1String("filename")));
        if (imageEntry&&imageEntry->isFile())
        {
            const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
            QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/") + QLatin1String("cantor");
            imageFile->copyTo(dir);
            QUrl imageUrl = QUrl::fromLocalFile(QDir(dir).absoluteFilePath(imageFile->name()));
            if(type==QLatin1String("latex"))
            {
                result=new Cantor::LatexResult(resultElement.text(), imageUrl);
            }else if(type==QLatin1String("animation"))
            {
                result=new Cantor::AnimationResult(imageUrl);
            }else if(imageFile->name().endsWith(QLatin1String(".eps")))
            {
                result=new Cantor::EpsResult(imageUrl);
            }else
            {
                result=new Cantor::ImageResult(imageUrl);
            }
        }
    }

    setResult(result);
}
