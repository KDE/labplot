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
#include "DockHelpers.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/3d/Curve3D.h"
#include "backend/core/AspectTreeModel.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>

using namespace DockHelpers;

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

	//template handler
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Curve3D);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();

	foreach(TreeViewComboBox* view, treeViews) {
		view->setSelectableClasses(list);
		connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), SLOT(onTreeViewIndexChanged(const QModelIndex&)));
	}

	connect(ui.leName, SIGNAL(returnPressed()), SLOT(onNameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), SLOT(onCommentChanged()));
	connect(ui.chkVisible, SIGNAL(toggled(bool)), SLOT(onVisibilityChanged(bool)));

	connect(ui.cbShowEdges, SIGNAL(toggled(bool)), SLOT(onShowEdgesChanged(bool)));
	connect(ui.cbClosedCurve, SIGNAL(toggled(bool)), SLOT(onIsClosedChanged(bool)));
	connect(ui.sbPointSize, SIGNAL(valueChanged(double)), SLOT(onPointRadiusChanged(double)));

	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void Curve3DDock::setCurve(Curve3D* curve) {
	this->curve = curve;

	{
	const SignalBlocker blocker(this);
	ui.leName->setText(curve->name());
	ui.leComment->setText(curve->comment());
	ui.chkVisible->setChecked(curve->isVisible());
	pointRadiusChanged(curve->pointRadius());

	aspectTreeModel = new AspectTreeModel(curve->project());
	ui.cbXCoordinate->setModel(aspectTreeModel);
	ui.cbYCoordinate->setModel(aspectTreeModel);
	ui.cbZCoordinate->setModel(aspectTreeModel);

	xColumnChanged(curve->xColumn());
	yColumnChanged(curve->yColumn());
	zColumnChanged(curve->zColumn());

	isClosedChanged(curve->isClosed());
	showEdgesChanged(curve->showEdges());
	}

	connect(curve, SIGNAL(xColumnChanged(const AbstractColumn*)), SLOT(xColumnChanged(const AbstractColumn*)));
	connect(curve, SIGNAL(yColumnChanged(const AbstractColumn*)), SLOT(yColumnChanged(const AbstractColumn*)));
	connect(curve, SIGNAL(zColumnChanged(const AbstractColumn*)), SLOT(zColumnChanged(const AbstractColumn*)));

	connect(curve, SIGNAL(pointRadiusChanged(float)), SLOT(pointRadiusChanged(float)));
	connect(curve, SIGNAL(isClosedChanged(bool)), SLOT(isClosedChanged(bool)));
	connect(curve, SIGNAL(showEdgesChanged(bool)), SLOT(showEdgesChanged(bool)));
	connect(curve, SIGNAL(visibilityChanged(bool)), ui.chkVisible, SLOT(setChecked(bool)));
}

void Curve3DDock::onNameChanged() {
	const Lock lock(m_initializing);
	curve->setName(ui.leName->text());
}

void Curve3DDock::onCommentChanged() {
	const Lock lock(m_initializing);
	curve->setComment(ui.leComment->text());
}

void Curve3DDock::onTreeViewIndexChanged(const QModelIndex& index) {
	const AbstractColumn* column = getColumn(index);

	const QObject *senderW = sender();
	const Lock lock(m_initializing);
	if(senderW == ui.cbXCoordinate)
		curve->setXColumn(column);
	else if(senderW  == ui.cbYCoordinate)
		curve->setYColumn(column);
	else
		curve->setZColumn(column);
}

void Curve3DDock::onShowEdgesChanged(bool checked) {
	ui.cbClosedCurve->setEnabled(checked);
	const Lock lock(m_initializing);
	curve->setShowEdges(checked);
}

void Curve3DDock::onVisibilityChanged(bool visible) {
	const Lock lock(m_initializing);
	curve->show(visible);
}

void Curve3DDock::onXColumnChanged(const AbstractColumn* column) {
	const Lock lock(m_initializing);
	curve->setXColumn(column);
}

void Curve3DDock::onYColumnChanged(const AbstractColumn* column) {
	const Lock lock(m_initializing);
	curve->setYColumn(column);
}

void Curve3DDock::onZColumnChanged(const AbstractColumn* column) {
	const Lock lock(m_initializing);
	curve->setZColumn(column);
}

void Curve3DDock::onPointRadiusChanged(double radius) {
	const Lock lock(m_initializing);
	curve->setPointRadius(static_cast<float>(radius));
}

void Curve3DDock::onIsClosedChanged(bool checked) {
	const Lock lock(m_initializing);
	curve->setIsClosed(checked);
}

void Curve3DDock::showEdgesChanged(bool checked) {
	const Lock lock(m_initializing);
	curve->setShowEdges(checked);
}

void Curve3DDock::isClosedChanged(bool checked) {
	if (m_initializing)
		return;
	ui.cbClosedCurve->setChecked(checked);
}

void Curve3DDock::pointRadiusChanged(float radius) {
	if (m_initializing)
		return;
	ui.sbPointSize->setValue(radius);
}

// TODO: Move to the common place
void Curve3DDock::setModelFromAspect(TreeViewComboBox* cb, const AbstractAspect* aspect) {
	if (m_initializing)
		return;
	cb->setCurrentModelIndex(modelIndexOfAspect(aspectTreeModel, aspect));
}

void Curve3DDock::xColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbXCoordinate, column);
}

void Curve3DDock::yColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbYCoordinate, column);
}

void Curve3DDock::zColumnChanged(const AbstractColumn* column) {
	setModelFromAspect(ui.cbZCoordinate, column);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void Curve3DDock::load() {
	//TODO
}

void Curve3DDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	//TODO
// 	int size = m_curvesList.size();
// 	if (size>1)
// 		m_curve->beginMacro(i18n("%1 3D-curves: template \"%2\" loaded", size, name));
// 	else
// 		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

// 	m_curve->endMacro();
}

void Curve3DDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "XYCurve" );
	//TODO

}

void Curve3DDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "XYCurve" );
	//TODO

	config.sync();
}
