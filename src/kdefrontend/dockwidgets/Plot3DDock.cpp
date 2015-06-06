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
	hideCoordinates();

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
	ui.cbXCoordinate->setTopLevelClasses(list);
	ui.cbYCoordinate->setTopLevelClasses(list);
	ui.cbZCoordinate->setTopLevelClasses(list);

	list.clear();
	list<<"Column";
	ui.cbXCoordinate->setSelectableClasses(list);
	ui.cbYCoordinate->setSelectableClasses(list);
	ui.cbZCoordinate->setSelectableClasses(list);

	connect(ui.cbXCoordinate, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(onXCoordinateSourceChanged(const QModelIndex&)));
	connect(ui.cbYCoordinate, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(onYCoordinateSourceChanged(const QModelIndex&)));
	connect(ui.cbZCoordinate, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(onZCoordinatedSourceChanged(const QModelIndex&)));
}

AbstractColumn* Plot3DDock::getColumn(const QModelIndex& index) const{
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	return aspect ? dynamic_cast<AbstractColumn*>(aspect) : 0;
}

void Plot3DDock::onXCoordinateSourceChanged(const QModelIndex& index){
	qDebug() << Q_FUNC_INFO;
	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	foreach(Plot3D* plot, plots){
		plot->setDataSource(Plot3D::DataSource_Spreadsheet);
		plot->setXColumn(column);
		plot->retransform();
	}
}

void Plot3DDock::onYCoordinateSourceChanged(const QModelIndex& index){
	qDebug() << Q_FUNC_INFO;
	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	foreach(Plot3D* plot, plots){
		plot->setDataSource(Plot3D::DataSource_Spreadsheet);
		plot->setYColumn(column);
		plot->retransform();
	}
}

void Plot3DDock::onZCoordinatedSourceChanged(const QModelIndex& index){
	qDebug() << Q_FUNC_INFO;
	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	foreach(Plot3D* plot, plots){
		plot->setDataSource(Plot3D::DataSource_Spreadsheet);
		plot->setZColumn(column);
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
		hideCoordinates();
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
	hideCoordinates(index != Plot3D::DataSource_Spreadsheet);
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

void Plot3DDock::hideCoordinates(bool hide){
	ui.labelX->setVisible(!hide);
	ui.labelY->setVisible(!hide);
	ui.labelZ->setVisible(!hide);

	ui.cbXCoordinate->setVisible(!hide);
	ui.cbYCoordinate->setVisible(!hide);
	ui.cbZCoordinate->setVisible(!hide);
}

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	this->plots = plots;
	if (!plots.empty()){
		QAbstractItemModel *aspectTreeModel = new AspectTreeModel(plots.first()->project());
		ui.cbXCoordinate->setModel(aspectTreeModel);
		ui.cbYCoordinate->setModel(aspectTreeModel);
		ui.cbZCoordinate->setModel(aspectTreeModel);
	}
}