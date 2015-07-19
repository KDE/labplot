/***************************************************************************
    File                 : Surface3DDock.cpp
    Project              : LabPlot
    Description          : widget for 3D surfaces properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#include "Surface3DDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/matrix/Matrix.h"

#include "backend/worksheet/plots/3d/DataHandlers.h"

#include <QDebug>

Surface3DDock::Surface3DDock(QWidget* parent)
	: QWidget(parent)
	, surface(0)
	, aspectTreeModel(0)
	, m_initializing(false) {
	ui.setupUi(this);

	this->retranslateUi();

	ui.cbDataSource->insertItem(Surface3D::DataSource_File, i18n("File"));
	ui.cbDataSource->insertItem(Surface3D::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSource->insertItem(Surface3D::DataSource_Matrix, i18n("Matrix"));
	ui.cbDataSource->insertItem(Surface3D::DataSource_Empty, i18n("Demo"));
	ui.cbDataSource->setCurrentIndex(Surface3D::DataSource_File);

	ui.cbType->insertItem(Surface3D::VisualizationType_Triangles, i18n("Triangles"));

	QList<const char*>  list;
	list << "Folder" << "Workbook" << "Spreadsheet" << "FileDataSource" << "Column";

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(TreeViewComboBox* view, treeViews) {
		view->setTopLevelClasses(list);
	}

	list.clear();
	list << "Column";

	foreach(TreeViewComboBox* view, treeViews) {
		view->setSelectableClasses(list);
		connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));
	}

	//Matrix data source
	list.clear();
	list<<"Folder"<<"Workbook"<<"Matrix";
	ui.cbMatrix->setTopLevelClasses(list);

	list.clear();
	list<<"Matrix";
	ui.cbMatrix->setSelectableClasses(list);

	//SIGNALs/SLOTs
	//General
	connect(ui.cbDataSource, SIGNAL(currentIndexChanged(int)), SLOT(onDataSourceChanged(int)));
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), SLOT(onVisualizationTypeChanged(int)));
	connect(ui.cbFileRequester, SIGNAL(urlSelected(const KUrl&)), SLOT(onFileChanged(const KUrl&)));
	connect(ui.cbMatrix, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));

	//Color filling
	connect( ui.cbColorFillingType, SIGNAL(currentIndexChanged(int)), this, SLOT(colorFillingTypeChanged(int)) );

	//Mesh

	//Projection
}

namespace {
	AbstractColumn* getColumn(const QModelIndex& index) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		return aspect ? dynamic_cast<AbstractColumn*>(aspect) : 0;
	}

	Matrix* getMatrix(const QModelIndex& index) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		return aspect ? dynamic_cast<Matrix*>(aspect) : 0;
	}
}

void Surface3DDock::setSurface(Surface3D *surface) {
	this->surface = surface;

	aspectTreeModel = new AspectTreeModel(surface->project());
	ui.cbXCoordinate->setModel(aspectTreeModel);
	ui.cbYCoordinate->setModel(aspectTreeModel);
	ui.cbZCoordinate->setModel(aspectTreeModel);
	ui.cbNode1->setModel(aspectTreeModel);
	ui.cbNode2->setModel(aspectTreeModel);
	ui.cbNode3->setModel(aspectTreeModel);
	ui.cbMatrix->setModel(aspectTreeModel);
	ui.cbColorFillingMatrix->setModel(aspectTreeModel);

	ui.cbType->setCurrentIndex(surface->visualizationType());
	ui.cbDataSource->setCurrentIndex(surface->dataSource());

	connect(surface, SIGNAL(visualizationTypeChanged(Surface3D::VisualizationType)), SLOT(visualizationTypeChanged(Surface3D::VisualizationType)));
	connect(surface, SIGNAL(sourceTypeChanged(Surface3D::DataSource)), SLOT(sourceTypeChanged(Surface3D::DataSource)));

	// DataHandlers
	connect(&surface->fileDataHandler(), SIGNAL(pathChanged(const KUrl&)), SLOT(pathChanged(const KUrl&)));
	connect(&surface->matrixDataHandler(), SIGNAL(matrixChanged(const Matrix*)), SLOT(matrixChanged(const Matrix*)));
	SpreadsheetDataHandler *sdh = &surface->spreadsheetDataHandler();
	connect(sdh, SIGNAL(xColumnChanged(const AbstractColumn*)), SLOT(xColumnChanged(const AbstractColumn*)));
	connect(sdh, SIGNAL(yColumnChanged(const AbstractColumn*)), SLOT(yColumnChanged(const AbstractColumn*)));
	connect(sdh, SIGNAL(zColumnChanged(const AbstractColumn*)), SLOT(zColumnChanged(const AbstractColumn*)));
	connect(sdh, SIGNAL(firstNodeChanged(const AbstractColumn*)), SLOT(firstNodeChanged(const AbstractColumn*)));
	connect(sdh, SIGNAL(secondNodeChanged(const AbstractColumn*)), SLOT(secondNodeChanged(const AbstractColumn*)));
	connect(sdh, SIGNAL(thirdNodeChanged(const AbstractColumn*)), SLOT(thirdNodeChanged(const AbstractColumn*)));
}

void Surface3DDock::hideDataSource(bool hide) {
	ui.labelSource->setVisible(!hide);
	ui.cbDataSource->setVisible(!hide);
}

void Surface3DDock::hideFileUrl(bool hide) {
	ui.labelFile->setVisible(!hide);
	ui.cbFileRequester->setVisible(!hide);
}

void Surface3DDock::hideTriangleInfo(bool hide) {
	const QVector<QWidget*> widgets(QVector<QWidget*>()
			<< ui.labelX << ui.labelY << ui.labelZ
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.labelNode1 << ui.labelNode2 << ui.labelNode3
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(QWidget* w, widgets){
		w->setVisible(!hide);
	}
}

namespace{
	struct Lock{
		Lock(bool& variable)
			: variable(variable){
			variable = true;
		}

		~Lock(){
			variable = false;
		}

	private:
		bool& variable;
	};
}

//*************************************************************
//****** SLOTs for changes triggered in Surface3DDock *********
//*************************************************************
void Surface3DDock::retranslateUi(){
	Lock lock(m_initializing);

	//color filling
	ui.cbColorFillingType->addItem( i18n("no filling") );
	ui.cbColorFillingType->addItem( i18n("solid color") );
	ui.cbColorFillingType->addItem( i18n("color map") );
	ui.cbColorFillingType->addItem( i18n("color map from matrix") );
}

void Surface3DDock::onTreeViewIndexChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	QObject *senderW = sender();
	if(senderW == ui.cbXCoordinate)
		surface->spreadsheetDataHandler().setXColumn(column);
	else if(senderW  == ui.cbYCoordinate)
		surface->spreadsheetDataHandler().setYColumn(column);
	else if(senderW  == ui.cbZCoordinate)
		surface->spreadsheetDataHandler().setZColumn(column);
	else if(senderW  == ui.cbNode1)
		surface->spreadsheetDataHandler().setFirstNode(column);
	else if(senderW  == ui.cbNode2)
		surface->spreadsheetDataHandler().setSecondNode(column);
	else if(senderW == ui.cbNode3)
		surface->spreadsheetDataHandler().setThirdNode(column);
	else if(senderW == ui.cbMatrix)
		surface->matrixDataHandler().setMatrix(getMatrix(index));
}

void Surface3DDock::onDataSourceChanged(int index) {
	const Surface3D::DataSource type = static_cast<Surface3D::DataSource>(index);
	hideFileUrl(type != Surface3D::DataSource_File);
	hideTriangleInfo(type != Surface3D::DataSource_Spreadsheet);

	const bool b = (type == Surface3D::DataSource_Matrix);
	ui.labelMatrix->setVisible(b);
	ui.cbMatrix->setVisible(b);

	if (!m_initializing)
		surface->setDataSource(type);
}

void Surface3DDock::onVisualizationTypeChanged(int index) {
	if(!m_initializing)
		surface->setVisualizationType(static_cast<Surface3D::VisualizationType>(index));

	if(index == Surface3D::VisualizationType_Triangles){
		hideDataSource(false);
		onDataSourceChanged(ui.cbDataSource->currentIndex());
	}else{
		hideDataSource();
		hideFileUrl();
		hideTriangleInfo();
	}
}

void Surface3DDock::onFileChanged(const KUrl& path) {
	if (m_initializing || !path.isLocalFile())
		return;

	surface->fileDataHandler().setFile(path);
}

void Surface3DDock::visualizationTypeChanged(Surface3D::VisualizationType type) {
	Lock lock(m_initializing);
	ui.cbType->setCurrentIndex(type);
}

void Surface3DDock::sourceTypeChanged(Surface3D::DataSource type) {
	Lock lock(m_initializing);
	ui.cbDataSource->setCurrentIndex(type);
}

void Surface3DDock::pathChanged(const KUrl& url) {
	Lock lock(m_initializing);
	ui.cbFileRequester->setUrl(url);
}

void Surface3DDock::matrixChanged(const Matrix* matrix) {
	Lock lock(m_initializing);
	if (matrix)
		ui.cbMatrix->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(matrix));
	else
		ui.cbMatrix->setCurrentIndex(-1);
}

void Surface3DDock::xColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbXCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbXCoordinate->setCurrentIndex(-1);
}

void Surface3DDock::yColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbYCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbYCoordinate->setCurrentIndex(-1);
}

void Surface3DDock::zColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbZCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbZCoordinate->setCurrentIndex(-1);
}

void Surface3DDock::firstNodeChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbNode1->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbNode1->setCurrentIndex(-1);
}

void Surface3DDock::secondNodeChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbNode2->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbNode2->setCurrentIndex(-1);
}

void Surface3DDock::thirdNodeChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	if (column)
		ui.cbNode3->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(column));
	else
		ui.cbNode3->setCurrentIndex(-1);
}

//Collor filling
void Surface3DDock::colorFillingTypeChanged(int index) {
	Surface3D::CollorFilling type = (Surface3D::CollorFilling)index;
	if (type == Surface3D::NoFilling) {
		ui.lColorFilling->hide();
		ui.kcbColorFilling->hide();
		ui.lColorFillingMap->hide();
		ui.cbColorFillingMap->hide();
		ui.lColorFillingMatrix->hide();
		ui.cbColorFillingMatrix->hide();
		ui.lColorFillingOpacity->hide();
		ui.sbColorFillingOpacity->hide();
	} else if (type == Surface3D::SolidColor) {
		ui.lColorFilling->show();
		ui.kcbColorFilling->show();
		ui.lColorFillingMap->hide();
		ui.cbColorFillingMap->hide();
		ui.lColorFillingMatrix->hide();
		ui.cbColorFillingMatrix->hide();
		ui.lColorFillingOpacity->show();
		ui.sbColorFillingOpacity->show();
	} else if (type == Surface3D::ColorMap) {
		ui.lColorFilling->hide();
		ui.kcbColorFilling->hide();
		ui.lColorFillingMap->show();
		ui.cbColorFillingMap->show();
		ui.lColorFillingMatrix->hide();
		ui.cbColorFillingMatrix->hide();
		ui.lColorFillingOpacity->show();
		ui.sbColorFillingOpacity->show();
	} else if (type == Surface3D::ColorMapFromMatrix) {
		ui.lColorFilling->hide();
		ui.kcbColorFilling->hide();
		ui.lColorFillingMap->hide();
		ui.cbColorFillingMap->hide();
		ui.lColorFillingMatrix->show();
		ui.cbColorFillingMatrix->show();
		ui.lColorFillingOpacity->show();
		ui.sbColorFillingOpacity->show();
	}
}

