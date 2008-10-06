/***************************************************************************
    File                 : MatrixModule.cpp
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
#include "matrix/MatrixModule.h"
#include "matrix/Matrix.h"
#include "core/Project.h"
#include "core/ProjectWindow.h"
#include "lib/ActionManager.h"
#include <QAction>
#include <QPixmap>
#include <QSettings>
#include "ui_MatrixConfigPage.h"

MatrixConfigPage::MatrixConfigPage() 
{
	ui = new Ui_MatrixConfigPage();
	ui->setupUi(this);
}

MatrixConfigPage::~MatrixConfigPage() 
{
	delete ui;
}

void MatrixConfigPage::apply()
{
	// TODO: read settings from ui and change them in Matrix
}

AbstractPart * MatrixModule::makePart()
{
	return new Matrix(0, 32, 32, tr("Matrix %1").arg(1));
}

QAction * MatrixModule::makeAction(QObject *parent)
{
	QAction *new_matrix = new QAction(tr("New &Matrix"), parent);
	new_matrix->setShortcut(tr("Ctrl+M", "new matrix shortcut"));
	new_matrix->setIcon(QIcon(QPixmap(":/new_matrix.xpm")));
	Matrix::actionManager()->addAction(new_matrix, "new_matrix");
	return new_matrix;
}

void MatrixModule::initActionManager()
{
	Matrix::initActionManager();
}

ConfigPageWidget * MatrixModule::makeConfigPage()
{
	return new MatrixConfigPage();
}
		
QString MatrixModule::configPageLabel()
{
	return QObject::tr("Matrix");
}

bool MatrixModule::canCreate(const QString & element_name)
{	
	return element_name == "matrix";
}

AbstractAspect * MatrixModule::createAspectFromXml(XmlStreamReader * reader)
{
	Matrix * matrix = new Matrix(0, 0, 0, tr("Matrix %1").arg(1));
	if (!(matrix->load(reader)))
	{
		delete matrix;
		return NULL;
	}
	else
		return matrix;
}

void MatrixModule::staticInit()
{
	Matrix::setGlobalDefault("images_path", QApplication::applicationDirPath()); 
}

Q_EXPORT_PLUGIN2(scidavis_matrix, MatrixModule)

