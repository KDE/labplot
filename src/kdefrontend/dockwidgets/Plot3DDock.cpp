/***************************************************************************
    File                 : Plot3DDock.h
    Project              : LabPlot
    Description          : widget for 3D plot properties
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

#include "Plot3DDock.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/3d/Plot3D.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>

#include <KComboBox>
#include <KUrl>
#include <KUrlRequester>
#include <KLocalizedString>

Plot3DDock::Plot3DDock(QWidget* parent) : QWidget(parent){
	ui.setupUi(this);

	//TODO: remove this later - the initialization of the dock widget will be done in setPlots() later
	hideDataSource();
	hideFileUrl();
	hideTriangleInfo();
	//######

	ui.cbDataSource->insertItem(Plot3D::DataSource_File, i18n("File"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Matrix, i18n("Matrix"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Empty, i18n("Demo"));
	ui.cbDataSource->setCurrentIndex(Plot3D::DataSource_File);

	ui.cbType->insertItem(Plot3D::VisualizationType_Triangles, i18n("Triangles"));
	ui.cbType->setCurrentIndex(Plot3D::VisualizationType_Triangles);
	onVisualizationTypeChanged(ui.cbType->currentIndex());


	//Spreadsheet data source
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column";

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(TreeViewComboBox* view, treeViews){
		view->setTopLevelClasses(list);
	}

	list.clear();
	list<<"Column";

	foreach(TreeViewComboBox* view, treeViews){
		view->setSelectableClasses(list);
		connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(onTreeViewIndexChanged(const QModelIndex&)));
	}

	//Matrix data source
	list.clear();
	list<<"Folder"<<"Workbook"<<"Matrix";
	ui.cbMatrix->setTopLevelClasses(list);

	list.clear();
	list<<"Matrix";
	ui.cbMatrix->setSelectableClasses(list);

	//SIGNALs/SLOTs
	connect(ui.cbDataSource, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataSourceChanged(int)));
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(onVisualizationTypeChanged(int)));
	connect(ui.cbFileRequester, SIGNAL(urlSelected(const KUrl&)), this, SLOT(onFileChanged(const KUrl&)));
}

AbstractColumn* Plot3DDock::getColumn(const QModelIndex& index) const{
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	return aspect ? dynamic_cast<AbstractColumn*>(aspect) : 0;
}

void Plot3DDock::onTreeViewIndexChanged(const QModelIndex& index){
	qDebug() << Q_FUNC_INFO;
	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	foreach(Plot3D* plot, plots){
		plot->setDataSource(Plot3D::DataSource_Spreadsheet);
		QObject *senderW = sender();
		if(senderW == ui.cbXCoordinate)
			plot->setXColumn(column);
		else if(senderW  == ui.cbYCoordinate)
			plot->setYColumn(column);
		else if(senderW  == ui.cbZCoordinate)
			plot->setZColumn(column);
		else if(senderW  == ui.cbNode1)
			plot->setNodeColumn(0, column);
		else if(senderW  == ui.cbNode2)
			plot->setNodeColumn(1, column);
		else if(senderW == ui.cbNode3)
			plot->setNodeColumn(2, column);
		plot->retransform();
	}
}

void Plot3DDock::onVisualizationTypeChanged(int index){
	qDebug() << Q_FUNC_INFO << index;
	foreach(Plot3D* plot, plots){
		plot->setVisualizationType(static_cast<Plot3D::VisualizationType>(index));
		plot->retransform();
	}

	if (index == Plot3D::VisualizationType_Triangles){
		hideDataSource(false);
		onDataSourceChanged(ui.cbDataSource->currentIndex());
	}else{
		hideDataSource();
		hideFileUrl();
		hideTriangleInfo();
	}
}

void Plot3DDock::onFileChanged(const KUrl& path){
	if (!path.isLocalFile())
		return;

	foreach(Plot3D* plot, plots)
		plot->setFile(path);
}

void Plot3DDock::onDataSourceChanged(int index){
	qDebug() << Q_FUNC_INFO << index;
	Plot3D::DataSource type = (Plot3D::DataSource)index;
	hideFileUrl(index != Plot3D::DataSource_File);
	hideTriangleInfo(index != Plot3D::DataSource_Spreadsheet);
	
	bool b = (type==Plot3D::DataSource_Matrix);
	ui.labelMatrix->setVisible(b);
	ui.cbMatrix->setVisible(b);

	foreach(Plot3D* plot, plots)
		plot->setDataSource(type);
}

void Plot3DDock::hideDataSource(bool hide){
	ui.labelSource->setVisible(!hide);
	ui.cbDataSource->setVisible(!hide);
}

void Plot3DDock::hideFileUrl(bool hide){
	ui.labelFile->setVisible(!hide);
	ui.cbFileRequester->setVisible(!hide);
}

void Plot3DDock::hideTriangleInfo(bool hide){
	const QVector<QWidget*> widgets(QVector<QWidget*>()
			<< ui.labelX << ui.labelY << ui.labelZ
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.labelNode1 << ui.labelNode2 << ui.labelNode3
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(QWidget* w, widgets){
		w->setVisible(!hide);
	}
}

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	this->plots = plots;
	if (!plots.empty()){
		QAbstractItemModel *aspectTreeModel = new AspectTreeModel(plots.first()->project());

		const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
				<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
				<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3 << ui.cbMatrix);

		foreach(TreeViewComboBox* cb, treeViews){
			cb->setModel(aspectTreeModel);
		}

		if (ui.cbType->currentIndex() != -1)
			onVisualizationTypeChanged(ui.cbType->currentIndex());
	}
}