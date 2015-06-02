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
    Copyright (C) 2011 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "latexrenderer.h"
using namespace Cantor;

#include <KProcess>
#include <QDebug>
#include <QFileInfo>
#include <QEventLoop>
#include <QTemporaryFile>
#include <QStandardPaths>

// #include <cantor/config-cantorlib.h>
// #include "settings.h"

class Cantor::LatexRendererPrivate
{
  public:
    QString latexCode;
    QString header;
    LatexRenderer::Method method;
    bool isEquationOnly;
    LatexRenderer::EquationType equationType;
    QString errorMessage;
    bool success;
    QString latexFilename;
};

static const QLatin1String tex("\\documentclass[12pt,fleqn]{article}          \n "\
                         "\\usepackage{latexsym,amsfonts,amssymb,ulem}  \n "\
                         "\\usepackage[dvips]{graphicx}                 \n "\
                         "\\usepackage[utf8]{inputenc}                  \n "\
                         "\\setlength\\textwidth{5in}                   \n "\
                         "\\setlength{\\parindent}{0pt}                 \n "\
                         "%1                                            \n "\
                         "\\pagestyle{empty}                            \n "\
                         "\\begin{document}                             \n "\
                         "%2                                            \n "\
                         "\\end{document}\n");

static const QLatin1String eqnHeader("\\begin{eqnarray*}%1\\end{eqnarray*}    \n ");
static const QLatin1String inlineEqnHeader("$%1$ \n");

LatexRenderer::LatexRenderer(QObject* parent) : QObject(parent),
                                                d(new LatexRendererPrivate)
{
    d->method=LatexMethod;
    d->isEquationOnly=false;
    d->equationType=InlineEquation;
    d->success=false;
}

LatexRenderer::~LatexRenderer()
{
    delete d;
}

QString LatexRenderer::latexCode() const
{
    return d->latexCode;
}

void LatexRenderer::setLatexCode(const QString& src)
{
    d->latexCode=src;
}

QString LatexRenderer::header() const
{
    return d->header;
}

void LatexRenderer::addHeader(const QString& header)
{
    d->header.append(header);
}

void LatexRenderer::setHeader(const QString& header)
{
    d->header=header;
}

LatexRenderer::Method LatexRenderer::method() const
{
    return d->method;
}

void LatexRenderer::setMethod(LatexRenderer::Method method)
{
    d->method=method;
}

void LatexRenderer::setEquationType(LatexRenderer::EquationType type)
{
    d->equationType=type;
}

LatexRenderer::EquationType LatexRenderer::equationType() const
{
    return d->equationType;
}


void LatexRenderer::setErrorMessage(const QString& msg)
{
    d->errorMessage=msg;
}

QString LatexRenderer::errorMessage() const
{
    return d->errorMessage;
}

bool LatexRenderer::renderingSuccessful() const
{
    return d->success;
}

void LatexRenderer::setEquationOnly(bool isEquationOnly)
{
    d->isEquationOnly=isEquationOnly;
}

bool LatexRenderer::isEquationOnly() const
{
    return d->isEquationOnly;
}


QString LatexRenderer::imagePath() const
{
    return d->latexFilename;
}

void LatexRenderer::render()
{
    switch(d->method)
    {
        case LatexRenderer::LatexMethod:  renderWithLatex(); break;
        case LatexRenderer::MmlMethod:    renderWithMml(); break;
    };
}

void LatexRenderer::renderBlocking()
{
    QEventLoop event;
    connect(this, SIGNAL(done()), &event, SLOT(quit()));
    connect(this, SIGNAL(error()), &event, SLOT(quit()));

    render();
    event.exec();
}

void LatexRenderer::renderWithLatex()
{
    qDebug()<<"rendering using latex method";
    QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    //Check if the cantor subdir exists, if not, create it
    QTemporaryFile *texFile=new QTemporaryFile(dir + QLatin1String("/cantor_tex-XXXXXX.tex"));
    texFile->open();

    QString expressionTex=tex;
    expressionTex=expressionTex.arg(d->header);
    if(isEquationOnly())
    {
        switch(equationType())
        {
            case FullEquation: expressionTex=expressionTex.arg(eqnHeader); break;
            case InlineEquation: expressionTex=expressionTex.arg(inlineEqnHeader); break;
        }
    }
    expressionTex=expressionTex.arg(d->latexCode);

    qDebug()<<"full tex: "<<expressionTex;

    texFile->write(expressionTex.toUtf8());
    texFile->flush();

    QString fileName = texFile->fileName();
    qDebug()<<"fileName: "<<fileName;
    d->latexFilename=fileName;
    d->latexFilename.replace(QLatin1String(".tex"), QLatin1String(".eps"));
    KProcess *p=new KProcess( this );
    p->setWorkingDirectory(dir);

//     (*p)<<Settings::self()->latexCommand()<<QLatin1String("-interaction=batchmode")<<QLatin1String("-halt-on-error")<<fileName;
    (*p)<<QLatin1String("-interaction=batchmode")<<QLatin1String("-halt-on-error")<<fileName;
    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( convertToPs() ) );
    p->start();
}

void LatexRenderer::convertToPs()
{
    qDebug()<<"converting to ps";
    QString dviFile=d->latexFilename;
    dviFile.replace(QLatin1String(".eps"), QLatin1String(".dvi"));
    KProcess *p=new KProcess( this );
//     qDebug()<<"running: "<<Settings::self()->dvipsCommand()<<"-E"<<"-o"<<d->latexFilename<<dviFile;
//     (*p)<<Settings::self()->dvipsCommand()<<QLatin1String("-E")<<QLatin1String("-o")<<d->latexFilename<<dviFile;
    (*p)<<QLatin1String("-E")<<QLatin1String("-o")<<d->latexFilename<<dviFile;

    connect(p, SIGNAL( finished(int,  QProcess::ExitStatus) ), this, SLOT( convertingDone() ) );
    p->start();
}

void LatexRenderer::convertingDone()
{
    qDebug()<<"rendered file "<<d->latexFilename;
    //cleanup the temp directory a bit...
    QString dir=QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/") + QLatin1String("cantor");
    QStringList unneededExtensions;
    unneededExtensions<<QLatin1String(".log")<<QLatin1String(".aux")<<QLatin1String(".tex")<<QLatin1String(".dvi");
    foreach(const QString& ext, unneededExtensions)
    {
        QString s=d->latexFilename;
        s.replace(QLatin1String(".eps"), ext);
        QFile f(s);
        //f.remove();
    }

    if(QFileInfo(d->latexFilename).exists())
    {
        d->success=true;
        emit done();
    }else
    {
        d->success=false;
        setErrorMessage(QLatin1String("something is wrong"));
        emit error();
    }
}

void LatexRenderer::renderWithMml()
{
    qDebug()<<"WARNING: MML rendering not implemented yet!";
    emit done();
}


