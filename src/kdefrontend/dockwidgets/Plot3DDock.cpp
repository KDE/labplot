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
#include "backend/worksheet/plots/3d/Plot3D.h"

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
	}
}

void Plot3DDock::onFileChanged(const KUrl& path){
	if (path.isLocalFile()){
		foreach(Plot3D* plot, plots){
			qDebug() << Q_FUNC_INFO << "Set";
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
	if (index == Plot3D::DataSource_Empty){
		foreach(Plot3D* plot, plots){
			qDebug() << Q_FUNC_INFO << "Set";
			plot->setDataSource(Plot3D::DataSource_Empty);
			plot->retransform();
		}
		emit needRepaint();
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

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	this->plots = plots;
}