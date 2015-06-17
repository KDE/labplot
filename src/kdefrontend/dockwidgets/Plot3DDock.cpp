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

Plot3DDock::Plot3DDock(QWidget* parent)
	: QWidget(parent){
	ui.setupUi(this);

	hideDataSource();
	hideFileUrl();
	hideTriangleInfo();

	ui.cbDataSourceComboBox->insertItem(Plot3D::DataSource_File, i18n("From file"));
	ui.cbDataSourceComboBox->insertItem(Plot3D::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSourceComboBox->insertItem(Plot3D::DataSource_Empty, i18n("Demo"));
	ui.cbDataSourceComboBox->setCurrentIndex(Plot3D::DataSource_File);
	connect(ui.cbDataSourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataSourceChanged(int)));

	ui.cbTypeComboBox->insertItem(Plot3D::VisualizationType_Triangles, i18n("Triangles"));
	ui.cbTypeComboBox->setCurrentIndex(Plot3D::VisualizationType_Triangles);
	onVisualizationTypeChanged(ui.cbTypeComboBox->currentIndex());

	connect(ui.cbTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onVisualizationTypeChanged(int)));

	connect(ui.cbFileRequester, SIGNAL(urlSelected(const KUrl&)), this, SLOT(onFileChanged(const KUrl&)));

	QList<const char*>  list;
	list<<"Folder"<<"Spreadsheet"<<"FileDataSource"<<"Column";

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
		onDataSourceChanged(ui.cbDataSourceComboBox->currentIndex());
	}else{
		hideDataSource();
		hideFileUrl();
		hideTriangleInfo();
	}
}

void Plot3DDock::onFileChanged(const KUrl& path){
	if (path.isLocalFile()){
		foreach(Plot3D* plot, plots){
			plot->setDataSource(Plot3D::DataSource_File);
			plot->setFile(path);
			plot->retransform();
		}
		emit needRepaint();
	}
}

void Plot3DDock::onDataSourceChanged(int index){
	qDebug() << Q_FUNC_INFO << index;
	hideFileUrl(index != Plot3D::DataSource_File);
	hideTriangleInfo(index != Plot3D::DataSource_Spreadsheet);
	if (index == Plot3D::DataSource_Empty){
		foreach(Plot3D* plot, plots){
			plot->setDataSource(Plot3D::DataSource_Empty);
			plot->retransform();
		}
		emit needRepaint();
	}else if (index == Plot3D::DataSource_Spreadsheet){
		foreach(Plot3D* plot, plots){
			plot->setDataSource(Plot3D::DataSource_Spreadsheet);
			plot->retransform();
		}
	}
}

void Plot3DDock::hideDataSource(bool hide){
	ui.labelSource->setVisible(!hide);
	ui.cbDataSourceComboBox->setVisible(!hide);
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
				<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

		foreach(TreeViewComboBox* cb, treeViews){
			cb->setModel(aspectTreeModel);
		}

		if (ui.cbTypeComboBox->currentIndex() != -1)
			onVisualizationTypeChanged(ui.cbTypeComboBox->currentIndex());
	}
}