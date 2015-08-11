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
#include "XmlAttributeReader.h"

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

Axes::Axes(vtkRenderer* renderer)
	: AbstractAspect(i18n("Axes"))
	, d_ptr(new AxesPrivate(renderer, this)) {
}

Axes::~Axes() {
}

void Axes::setRenderer(vtkRenderer* renderer) {
	Q_D(Axes);
	d->renderer = renderer;
	if (renderer)
		d->init();
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
	d->updateBounds();
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
BASIC_SHARED_D_READER_IMPL(Axes, QString, xLabel, xLabel)
BASIC_SHARED_D_READER_IMPL(Axes, QString, yLabel, yLabel)
BASIC_SHARED_D_READER_IMPL(Axes, QString, zLabel, zLabel)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Axes, SetType, Axes::AxesType, type, update)
STD_SETTER_IMPL(Axes, Type, Axes::AxesType, type, "%1: axes type changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetFontSize, int, fontSize, update)
STD_SETTER_IMPL(Axes, FontSize, int, fontSize, "%1: axes font size changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetWidth, double, width, update)
void Axes::setWidth(double width) {
	Q_D(Axes);
	if (!qFuzzyCompare(width, d->width))
		exec(new AxesSetWidthCmd(d, width, i18n("%1: axes width changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axes, SetXLabelColor, QColor, xLabelColor, update)
STD_SETTER_IMPL(Axes, XLabelColor, const QColor&, xLabelColor, "%1: axes X label color changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetYLabelColor, QColor, yLabelColor, update)
STD_SETTER_IMPL(Axes, YLabelColor, const QColor&, yLabelColor, "%1: axes Y label color changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetZLabelColor, QColor, zLabelColor, update)
STD_SETTER_IMPL(Axes, ZLabelColor, const QColor&, zLabelColor, "%1: axes Z label color changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetXLabel, QString, xLabel, update)
STD_SETTER_IMPL(Axes, XLabel, const QString&, xLabel, "%1: axes X label changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetYLabel, QString, yLabel, update)
STD_SETTER_IMPL(Axes, YLabel, const QString&, yLabel, "%1: axes Y label changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetZLabel, QString, zLabel, update)
STD_SETTER_IMPL(Axes, ZLabel, const QString&, zLabel, "%1: axes Z label changed")

////////////////////////////////////////////////////////////////////////////////

AxesPrivate::AxesPrivate(vtkRenderer* renderer, Axes* parent)
	: q(parent)
	, showAxes(false)
	, type(Axes::Axes::AxesType_Plain)
	, fontSize(32)
	, width(10)
	, xLabelColor(Qt::red)
	, yLabelColor(Qt::green)
	, zLabelColor(Qt::blue)
	, xLabel("X")
	, yLabel("Y")
	, zLabel("Z")
	, renderer(renderer) {
}

AxesPrivate::~AxesPrivate() {
}

void AxesPrivate::show(bool pred) {
	hide();
	if (pred)
		init();
	emit q->parametersChanged();
}

QString AxesPrivate::name() const {
	return i18n("Axes");
}

void AxesPrivate::getBoundingBox(double bounds[6]) {
	vtkPropCollection* actors = renderer->GetViewProps();
	actors->InitTraversal();

	vtkBoundingBox bb;
	if (actors->GetNumberOfItems() > 1) {
		vtkProp* actor = 0;
		while ((actor = actors->GetNextProp()) != 0) {
			if (actor == vtkAxes)
				continue;
			double *tb = actor->GetBounds();
			qDebug() << "Actor" << tb[0] << tb[1] << tb[2] << tb[3] << tb[4] << tb[5];
			bb.AddBounds(actor->GetBounds());
		}

		bb.GetBounds(bounds);
	} else {
		qDebug() << Q_FUNC_INFO;
		bounds[0] = bounds[2] = bounds[4] = -1;
		bounds[1] = bounds[3] = bounds[5] = 1;
	}
}

void AxesPrivate::updateBounds() {
	if (!showAxes || !vtkAxes.GetPointer() || !renderer)
		return;

	if (type == Axes::AxesType_Cube) {
		double bounds[6];
		getBoundingBox(bounds);
		qDebug() << Q_FUNC_INFO << bounds[0] << bounds[1] << bounds[2] << bounds[3]
				<< bounds[4] << bounds[5];
		dynamic_cast<vtkCubeAxesActor*>(vtkAxes.GetPointer())->SetBounds(bounds);
	}
}

void AxesPrivate::createCubeAxes() {
	vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();
	axes->SetCamera(renderer->GetActiveCamera());
	axes->SetScreenSize(30.0);
	axes->DrawXGridlinesOn();
	axes->DrawYGridlinesOn();
	axes->DrawZGridlinesOn();

	axes->XAxisMinorTickVisibilityOn();
	axes->YAxisMinorTickVisibilityOn();
	axes->ZAxisMinorTickVisibilityOn();
	renderer->AddViewProp(axes);

	axes->GetXAxesGridlinesProperty()->SetLineWidth(3);
	axes->GetYAxesGridlinesProperty()->SetLineWidth(3);
	axes->GetZAxesGridlinesProperty()->SetLineWidth(3);

	axes->GetXAxesLinesProperty()->SetLineWidth(5);
	axes->GetYAxesLinesProperty()->SetLineWidth(5);
	axes->GetZAxesLinesProperty()->SetLineWidth(5);

	axes->SetFlyModeToOuterEdges();

	axes->PickableOn();
	axes->SetGridLineLocation(2);
	vtkAxes = axes;
	updateBounds();
}

void AxesPrivate::createPlainAxes() {
	double colors[][3] = {
		{xLabelColor.redF(), xLabelColor.greenF(), xLabelColor.blueF()},
		{yLabelColor.redF(), yLabelColor.greenF(), yLabelColor.blueF()},
		{zLabelColor.redF(), zLabelColor.greenF(), zLabelColor.blueF()}
	};

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

	axes->SetXAxisLabelText(xLabel.toAscii());
	axes->SetYAxisLabelText(yLabel.toAscii());
	axes->SetZAxisLabelText(zLabel.toAscii());

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

void AxesPrivate::init() {
	if (!showAxes || !renderer)
		return;

	if (type == Axes::AxesType_Cube)
		createCubeAxes();
	else if (type == Axes::AxesType_Plain)
		createPlainAxes();

	if (vtkAxes)
		renderer->AddActor(vtkAxes);
}

void AxesPrivate::hide() {
	if (vtkAxes && renderer)
		renderer->RemoveActor(vtkAxes);
}

void AxesPrivate::update() {
	show(showAxes);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Axes::save(QXmlStreamWriter* writer) const {
	Q_D(const Axes);

	writer->writeStartElement("axes");
		writer->writeAttribute("showAxes", QString::number(d->showAxes));
		writer->writeAttribute("type", QString::number(d->type));
		writer->writeAttribute("fontSize", QString::number(d->fontSize));
		writer->writeAttribute("width", QString::number(d->width));

		writer->writeAttribute("xLabelColor", d->xLabelColor.name());
		writer->writeAttribute("yLabelColor", d->yLabelColor.name());
		writer->writeAttribute("zLabelColor", d->zLabelColor.name());
	writer->writeEndElement();
}


//! Load from XML
bool Axes::load(XmlStreamReader* reader) {
	Q_D(Axes);

	const QXmlStreamAttributes& attribs = reader->attributes();
	XmlAttributeReader attributeReader(reader, attribs);
	attributeReader.checkAndLoadAttribute("showAxes", d->showAxes);
	attributeReader.checkAndLoadAttribute<AxesType>("type", d->type);
	attributeReader.checkAndLoadAttribute("fontSize", d->fontSize);
	attributeReader.checkAndLoadAttribute("width", d->width);
	attributeReader.checkAndLoadAttribute("xLabelColor", d->xLabelColor);
	attributeReader.checkAndLoadAttribute("yLabelColor", d->yLabelColor);
	attributeReader.checkAndLoadAttribute("zLabelColor", d->zLabelColor);
	return true;
}