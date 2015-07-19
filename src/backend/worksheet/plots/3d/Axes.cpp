/***************************************************************************
    File                 : Axes.cpp
    Project              : LabPlot
    Description          : 3D plot axes
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#include "Axes.h"
#include "AxesPrivate.h"
#include "Plot3D.h"

#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <QDebug>

#include <KLocale>
#include <KMenu>
#include <KIcon>

#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkAxesActor.h>
#include <vtkProperty.h>
#include <vtkBoundingBox.h>
#include <vtkTextActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkCamera.h>

Axes::Axes(vtkRenderer& renderer)
	: AbstractAspect(i18n("Axes"))
	, d_ptr(new AxesPrivate(renderer, this)) {
}

Axes::~Axes() {
}

QIcon Axes::icon() const {
	return KIcon("axis-horizontal");
}

QMenu* Axes::createContextMenu() {
	// Reimplements createContextMenu to hide a delete button
	KMenu* menu = new KMenu();
	menu->addTitle(this->name());
	menu->addAction(KIcon("edit-rename"), i18n("Rename"), this, SIGNAL(renameRequested()));
	return menu;
}

void Axes::updateBounds() {
	Q_D(Axes);
	if (!d->showAxes || !d->vtkAxes)
		return;

	if (d->type == AxesType_Cube) {
		vtkActorCollection* actors = d->renderer.GetActors();
		actors->InitTraversal();

		vtkBoundingBox bb;

		double bounds[6];
		if (actors->GetNumberOfItems() != 0) {
			vtkActor* actor = 0;
			while ((actor = actors->GetNextActor()) != 0) {
				if (operator!=(actor))
					bb.AddBounds(actor->GetBounds());
			}

			vtkCubeAxesActor *axes = dynamic_cast<vtkCubeAxesActor*>(d->vtkAxes.GetPointer());
			bb.GetBounds(bounds);
			axes->SetBounds(bounds);
		} else {
			bounds[0] = bounds[2] = bounds[4] = -100;
			bounds[1] = bounds[3] = bounds[5] = 100;
		}
	}
}

bool Axes::operator==(vtkProp* prop) const {
	Q_D(const Axes);
	return d->vtkAxes == prop;
}

bool Axes::operator!=(vtkProp* prop) const {
	return !operator==(prop);
}

bool Axes::isVisible() const {
	Q_D(const Axes);
	return d->showAxes;
}

void Axes::show(bool pred) {
	Q_D(Axes);
	d->showAxes = pred;
	d->update();
	setHidden(!pred);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Axes, Axes::AxesType, type, type)
BASIC_SHARED_D_READER_IMPL(Axes, int, fontSize, fontSize)
BASIC_SHARED_D_READER_IMPL(Axes, double, width, width)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, xLabelColor, xLabelColor)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, yLabelColor, yLabelColor)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, zLabelColor, zLabelColor)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Axes, SetType, Axes::AxesType, type, update)
void Axes::setType(AxesType type) {
	Q_D(Axes);
	if (type != d->type)
		exec(new AxesSetTypeCmd(d, type, i18n("%1: axes type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetFontSize, int, fontSize, update)
void Axes::setFontSize(int fontSize) {
	Q_D(Axes);
	if (fontSize != d->fontSize)
		exec(new AxesSetFontSizeCmd(d, fontSize, i18n("%1: axes font size changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetWidth, double, width, update)
void Axes::setWidth(double width) {
	Q_D(Axes);
	if (!qFuzzyCompare(width, d->width))
		exec(new AxesSetWidthCmd(d, width, i18n("%1: axes width changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetXLabelColor, QColor, xLabelColor, update)
void Axes::setXLabelColor(const QColor& color){
	Q_D(Axes);
	if (color != d->xLabelColor)
		exec(new AxesSetXLabelColorCmd(d, color, i18n("%1: axes X label color changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetYLabelColor, QColor, yLabelColor, update)
void Axes::setYLabelColor(const QColor& color){
	Q_D(Axes);
	if (color != d->yLabelColor)
		exec(new AxesSetYLabelColorCmd(d, color, i18n("%1: axes Y label color changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetZLabelColor, QColor, zLabelColor, update)
void Axes::setZLabelColor(const QColor& color){
	Q_D(Axes);
	if (color != d->zLabelColor)
		exec(new AxesSetZLabelColorCmd(d, color, i18n("%1: axes Z label color changed")));
}

////////////////////////////////////////////////////////////////////////////////

AxesPrivate::AxesPrivate(vtkRenderer& renderer, Axes* parent)
	: q(parent)
	, showAxes(false)
	, type(Axes::Axes::AxesType_Plain)
	, fontSize(32)
	, width(10)
	, xLabelColor(Qt::red)
	, yLabelColor(Qt::green)
	, zLabelColor(Qt::blue)
	, renderer(renderer) {
	init();
}

AxesPrivate::~AxesPrivate() {
}

void AxesPrivate::show(bool pred) {
	hide();
	if (pred)
		init();
	emit q->parametersChanged();
}

QString AxesPrivate::name() const{
	return i18n("Axes");
}

void AxesPrivate::init() {
	if (!showAxes)
		return;

	double colors[][3] = {
		{xLabelColor.redF(), xLabelColor.greenF(), xLabelColor.blueF()},
		{yLabelColor.redF(), yLabelColor.greenF(), yLabelColor.blueF()},
		{zLabelColor.redF(), zLabelColor.greenF(), zLabelColor.blueF()}
	};

	if (type == Axes::AxesType_Cube) {
		vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();
		axes->SetCamera(renderer.GetActiveCamera());
		axes->DrawXGridlinesOn();
		axes->DrawYGridlinesOn();
		axes->DrawZGridlinesOn();

		axes->XAxisMinorTickVisibilityOff();
		axes->YAxisMinorTickVisibilityOff();
		axes->ZAxisMinorTickVisibilityOff();

		axes->SetLabelScaling(false, 0, 0, 0);

		for (int i = 0; i < 3; ++i) {
			vtkTextProperty *titleProp = axes->GetTitleTextProperty(i);
			titleProp->SetColor(colors[i]);
			titleProp->SetFontSize(fontSize);

			vtkTextProperty *labelProp = axes->GetLabelTextProperty(i);
			labelProp->SetColor(colors[i]);
			labelProp->SetFontSize(fontSize);
		}

		axes->GetXAxesLinesProperty()->SetLineWidth(width);
		axes->GetYAxesLinesProperty()->SetLineWidth(width);
		axes->GetZAxesLinesProperty()->SetLineWidth(width);

		vtkAxes = axes;
		q->updateBounds();
	} else if (type == Axes::AxesType_Plain) {
		vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();

		QVector<vtkCaptionActor2D*> captionActors;
		captionActors << axes->GetXAxisCaptionActor2D() << axes->GetYAxisCaptionActor2D()
				<< axes->GetZAxisCaptionActor2D();

		QVector<vtkProperty*> shaftProps;
		shaftProps << axes->GetXAxisShaftProperty() << axes->GetYAxisShaftProperty()
				<< axes->GetZAxisShaftProperty();

		QVector<vtkProperty*> tipProps;
		tipProps << axes->GetXAxisTipProperty() << axes->GetYAxisTipProperty()
				<< axes->GetZAxisTipProperty();

		for (int i = 0; i < 3; ++i) {
			vtkCaptionActor2D* const captionActor = captionActors[i];
			captionActor->GetTextActor()->SetTextScaleModeToNone();
			captionActor->GetCaptionTextProperty()->SetFontSize(fontSize);
			captionActor->GetCaptionTextProperty()->SetColor(colors[i]);

			shaftProps[i]->SetColor(colors[i]);
			tipProps[i]->SetColor(colors[i]);
		}

		vtkAxes = axes;
	}

	if (vtkAxes)
		renderer.AddActor(vtkAxes);
}

void AxesPrivate::hide() {
	if (vtkAxes)
		renderer.RemoveActor(vtkAxes);
}

void AxesPrivate::update() {
	show(showAxes);
}