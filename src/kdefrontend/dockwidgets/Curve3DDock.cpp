/***************************************************************************
    File                 : Curve3DDock.cpp
    Project              : LabPlot
    Description          : widget for 3D curves properties
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

#include "Curve3DDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/3d/Curve3D.h"
#include "backend/core/AspectTreeModel.h"

Curve3DDock::Curve3DDock(QWidget* parent)
	: QWidget(parent)
	, curve(0)
	, aspectTreeModel(0)
	, m_initializing(false) {
	ui.setupUi(this);

	QList<const char*>  list;
	list << "Folder" << "Workbook" << "Spreadsheet" << "FileDataSource" << "Column";

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate);

	foreach(TreeViewComboBox* view, treeViews) {
		view->setTopLevelClasses(list);
	}

	list.clear();
	list << "Column";

	foreach(TreeViewComboBox* view, treeViews) {
		view->setSelectableClasses(list);
		connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));
	}

	connect(ui.leName, SIGNAL(returnPressed()), SLOT(nameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), SLOT(commentChanged()));
	connect(ui.chkVisible, SIGNAL(toggled(bool)), SLOT(onVisibilityChanged(bool)));

	connect(ui.cbShowVertices, SIGNAL(toggled(bool)), SLOT(onShowVerticesChanged(bool)));
	connect(ui.cbClosedCurve, SIGNAL(toggled(bool)), SLOT(onClosedCurveChanged(bool)));
}

namespace {
	AbstractColumn* getColumn(const QModelIndex& index) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		return aspect ? dynamic_cast<AbstractColumn*>(aspect) : 0;
	}

	// TODO: Move to the common place
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


void Curve3DDock::setCurve(Curve3D* curve) {
	Lock lock(m_initializing);
	if (this->curve)
		this->curve->disconnect(this);

	this->curve = curve;

	ui.leName->setText(curve->name());
	ui.leComment->setText(curve->comment());
	ui.chkVisible->setChecked(curve->isVisible());

	aspectTreeModel = new AspectTreeModel(curve->project());
	ui.cbXCoordinate->setModel(aspectTreeModel);
	ui.cbYCoordinate->setModel(aspectTreeModel);
	ui.cbZCoordinate->setModel(aspectTreeModel);

	if (curve->xColumn())
		ui.cbXCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(curve->xColumn()));
	if (curve->yColumn())
		ui.cbYCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(curve->yColumn()));
	if (curve->zColumn())
		ui.cbZCoordinate->setCurrentModelIndex(aspectTreeModel->modelIndexOfAspect(curve->zColumn()));

	ui.cbClosedCurve->setChecked(curve->isClosed());
	ui.cbShowVertices->setChecked(curve->showVertices());

	connect(curve, SIGNAL(xColumnChanged(const AbstractColumn*)), SLOT(xColumnChanged(const AbstractColumn*)));
	connect(curve, SIGNAL(yColumnChanged(const AbstractColumn*)), SLOT(yColumnChanged(const AbstractColumn*)));
	connect(curve, SIGNAL(zColumnChanged(const AbstractColumn*)), SLOT(zColumnChanged(const AbstractColumn*)));

	connect(curve, SIGNAL(pointRadiusChanged(float)), SLOT(pointRadiusChanged(float)));
	connect(curve, SIGNAL(isClosedChanged(bool)), SLOT(isClosedChanged(bool)));
	connect(curve, SIGNAL(showVerticesChanged(bool)), SLOT(showVerticesChanged(bool)));
}

void Curve3DDock::nameChanged() {
	Lock lock(m_initializing);
	curve->setName(ui.leName->text());
}

void Curve3DDock::commentChanged() {
	Lock lock(m_initializing);
	curve->setComment(ui.leComment->text());
}

void Curve3DDock::onTreeViewIndexChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	QObject *senderW = sender();
	if(senderW == ui.cbXCoordinate)
		curve->setXColumn(column);
	else if(senderW  == ui.cbYCoordinate)
		curve->setYColumn(column);
	else
		curve->setZColumn(column);
}

void Curve3DDock::onShowVerticesChanged(bool checked) {
	if (m_initializing)
		return;

	curve->setShowVertices(checked);
}

void Curve3DDock::onVisibilityChanged(bool visible) {
	if (!m_initializing)
		curve->show(visible);
}

void Curve3DDock::onClosedCurveChanged(bool checked) {
	if (m_initializing)
		return;

	curve->setIsClosed(checked);
}

void Curve3DDock::xColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	curve->setXColumn(column);
}

void Curve3DDock::yColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	curve->setYColumn(column);
}

void Curve3DDock::zColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	curve->setZColumn(column);
}

void Curve3DDock::pointRadiusChanged(float radius) {
	Lock lock(m_initializing);
	curve->setPointRadius(radius);
}

void Curve3DDock::isClosedChanged(bool checked) {
	Lock lock(m_initializing);
	curve->setIsClosed(checked);
}

void Curve3DDock::showVerticesChanged(bool checked) {
	Lock lock(m_initializing);
	curve->setShowVertices(checked);
}