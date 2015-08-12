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
#include <KIcon>

#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>

Axes::Axes()
	: Base3D(new AxesPrivate(i18n("Axes"), this)) {
}

Axes::~Axes() {
}

QIcon Axes::icon() const {
	return KIcon("axis-horizontal");
}

void Axes::updateBounds() {
	Q_D(Axes);
	d->updateBounds();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Axes, int, fontSize, fontSize)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, xLabelColor, xLabelColor)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, yLabelColor, yLabelColor)
BASIC_SHARED_D_READER_IMPL(Axes, QColor, zLabelColor, zLabelColor)
BASIC_SHARED_D_READER_IMPL(Axes, QString, xLabel, xLabel)
BASIC_SHARED_D_READER_IMPL(Axes, QString, yLabel, yLabel)
BASIC_SHARED_D_READER_IMPL(Axes, QString, zLabel, zLabel)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Axes, SetFontSize, int, fontSize, update)
STD_SETTER_IMPL(Axes, FontSize, int, fontSize, "%1: axes font size changed")

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

AxesPrivate::AxesPrivate(const QString& name, Axes* parent)
	: Base3DPrivate(name, parent)
	, q(parent)
	, fontSize(32)
	, xLabelColor(Qt::red)
	, yLabelColor(Qt::green)
	, zLabelColor(Qt::blue)
	, xLabel("X")
	, yLabel("Y")
	, zLabel("Z") {
}

AxesPrivate::~AxesPrivate() {
}

void AxesPrivate::getBoundingBox(double bounds[6]) {
	vtkPropCollection* actors = renderer->GetViewProps();
	actors->InitTraversal();

	vtkBoundingBox bb;
	if (actors->GetNumberOfItems() > 1) {
		vtkProp* actor = 0;
		while ((actor = actors->GetNextProp()) != 0) {
			if (actor == this->actor.GetPointer())
				continue;

			bb.AddBounds(actor->GetBounds());
		}

		bb.GetBounds(bounds);
	} else {
		bounds[0] = bounds[2] = bounds[4] = -1;
		bounds[1] = bounds[3] = bounds[5] = 1;
	}
}

void AxesPrivate::updateBounds() {
	if (!actor || !renderer)
		return;

	double bounds[6];
	getBoundingBox(bounds);
	dynamic_cast<vtkCubeAxesActor*>(actor.GetPointer())->SetBounds(bounds);
}


void AxesPrivate::createActor() {
	vtkSmartPointer<vtkCubeAxesActor> axes = vtkSmartPointer<vtkCubeAxesActor>::New();
	axes->SetCamera(renderer->GetActiveCamera());

	axes->SetXTitle(xLabel.toAscii());
	axes->SetYTitle(yLabel.toAscii());
	axes->SetZTitle(zLabel.toAscii());

	axes->SetScreenSize(fontSize);
	axes->DrawXGridlinesOn();
	axes->DrawYGridlinesOn();
	axes->DrawZGridlinesOn();

	axes->XAxisMinorTickVisibilityOn();
	axes->YAxisMinorTickVisibilityOn();
	axes->ZAxisMinorTickVisibilityOn();

	axes->GetXAxesGridlinesProperty()->SetLineWidth(3);
	axes->GetYAxesGridlinesProperty()->SetLineWidth(3);
	axes->GetZAxesGridlinesProperty()->SetLineWidth(3);

	axes->GetXAxesLinesProperty()->SetLineWidth(5);
	axes->GetYAxesLinesProperty()->SetLineWidth(5);
	axes->GetZAxesLinesProperty()->SetLineWidth(5);

	axes->SetFlyModeToOuterEdges();

	axes->PickableOn();
	axes->SetGridLineLocation(2);

	double colors[][3] = {
		{xLabelColor.redF(), xLabelColor.greenF(), xLabelColor.blueF()},
		{yLabelColor.redF(), yLabelColor.greenF(), yLabelColor.blueF()},
		{zLabelColor.redF(), zLabelColor.greenF(), zLabelColor.blueF()}
	};

	for (int i = 0; i < 3; ++i) {
		vtkTextProperty *titleProp = axes->GetTitleTextProperty(i);
		titleProp->SetColor(colors[i]);
		titleProp->SetFontSize(fontSize);

		vtkTextProperty *labelProp = axes->GetLabelTextProperty(i);
		labelProp->SetColor(colors[i]);
		labelProp->SetFontSize(fontSize);
	}

	actor = axes;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Axes::save(QXmlStreamWriter* writer) const {
	Q_D(const Axes);

	writer->writeStartElement("axes");
		writer->writeAttribute("fontSize", QString::number(d->fontSize));
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
	attributeReader.checkAndLoadAttribute("fontSize", d->fontSize);
	attributeReader.checkAndLoadAttribute("xLabelColor", d->xLabelColor);
	attributeReader.checkAndLoadAttribute("yLabelColor", d->yLabelColor);
	attributeReader.checkAndLoadAttribute("zLabelColor", d->zLabelColor);
	return true;
}