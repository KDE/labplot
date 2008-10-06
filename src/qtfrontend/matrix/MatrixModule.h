/***************************************************************************
    File                 : MatrixModule.h
    Project              : SciDAVis
    Description          : Module providing the matrix Part and support classes.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email address)

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
#ifndef MATRIX_MODULE_H
#define MATRIX_MODULE_H

#include "core/interfaces.h"
#include "matrix/Matrix.h"
#include <QMenu>

//! Module providing the matrix Part and support classes.
class MatrixModule : public QObject, public PartMaker, public ActionManagerOwner, public ConfigPageMaker, 
	public XmlElementAspectMaker, public NeedsStaticInit
{
	Q_OBJECT
	Q_INTERFACES(PartMaker ActionManagerOwner ConfigPageMaker XmlElementAspectMaker NeedsStaticInit)

	public:
		virtual AbstractPart * makePart();
		virtual QAction * makeAction(QObject *parent);
		virtual ActionManager * actionManager() { return Matrix::actionManager(); }
		virtual void initActionManager();
		virtual ConfigPageWidget * makeConfigPage();
		virtual QString configPageLabel();
		virtual bool canCreate(const QString & element_name);
		virtual AbstractAspect * createAspectFromXml(XmlStreamReader * reader);
		virtual void staticInit();
};

class Ui_MatrixConfigPage;

//! Helper class for MatrixModule
class MatrixConfigPage : public ConfigPageWidget
{
	Q_OBJECT

	public:
		MatrixConfigPage();
		~MatrixConfigPage();

	public slots:
		virtual void apply();

	private:
		Ui_MatrixConfigPage *ui;
};

#endif // ifndef MATRIX_MODULE_H

