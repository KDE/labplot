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

#include <limits>
#include <cmath>

#include <vtkRenderer.h>
#include <vtkCubeAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkStringArray.h>

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
	d->Base3DPrivate::updateBounds();
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
	: Base3DPrivate(name, parent, vtkCubeAxesActor::New())
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

namespace {
	void addLabels(double init, double delta, int numIter, vtkStringArray* labels, double (*scaleFunction)(double)) {
		for (int i = 0; i < numIter; ++i) {
			const double scaledLabel = scaleFunction(init + i * delta);
			labels->InsertValue(i, QString::number(scaledLabel, 'g', 2).toAscii());
		}
	}
}

void AxesPrivate::objectScaled(vtkActor* actor) const {
	vtkCubeAxesActor* cubeAxes = dynamic_cast<vtkCubeAxesActor*>(actor);
	BoundingBox bounds = systemBounds();
	if (xScaling != Plot3D::Scaling_Linear) {
		bounds.setXMin(0.00000001);
		const int numXValues = 3;
		const double dx = (bounds.xMax() - bounds.xMin()) / (numXValues - 1);
		vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();

		if (xScaling == Plot3D::Scaling_Log10)
			addLabels(bounds.xMin(), dx, numXValues, labels, log10);
		else if (xScaling == Plot3D::Scaling_Log2)
			addLabels(bounds.xMin(), dx, numXValues, labels, log2);
		else if (xScaling == Plot3D::Scaling_Ln)
			addLabels(bounds.xMin(), dx, numXValues, labels, log);

		cubeAxes->SetAxisLabels(0, labels);
	} else
		cubeAxes->SetAxisLabels(0, 0);

	if (yScaling != Plot3D::Scaling_Linear) {
		bounds.setYMin(0.00000001);
		const int numYValues = 3;
		const double dy = (bounds.yMax() - bounds.yMin()) / (numYValues - 1);
		vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();

		if (yScaling == Plot3D::Scaling_Log10)
			addLabels(bounds.yMin(), dy, numYValues, labels, log10);
		else if (yScaling == Plot3D::Scaling_Log2)
			addLabels(bounds.yMin(), dy, numYValues, labels, log2);
		else if (yScaling == Plot3D::Scaling_Ln)
			addLabels(bounds.yMin(), dy, numYValues, labels, log);

		cubeAxes->SetAxisLabels(1, labels);
	} else
		cubeAxes->SetAxisLabels(1, 0);

	if (zScaling != Plot3D::Scaling_Linear) {
		bounds.setZMin(0.00000001);
		const int numZValues = 3;
		const double dz = (bounds.zMax() - bounds.zMin()) / (numZValues - 1);
		vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();

		if (zScaling == Plot3D::Scaling_Log10)
			addLabels(bounds.zMin(), dz, numZValues, labels, log10);
		else if (zScaling == Plot3D::Scaling_Log2)
			addLabels(bounds.zMin(), dz, numZValues, labels, log2);
		else if (zScaling == Plot3D::Scaling_Ln)
			addLabels(bounds.zMin(), dz, numZValues, labels, log);

		cubeAxes->SetAxisLabels(2, labels);
	} else
		cubeAxes->SetAxisLabels(2, 0);
}

void AxesPrivate::updateBounds(vtkActor* actor) const {
	if (!isInitialized())
		return;

	const BoundingBox& bb = systemBounds();
	vtkCubeAxesActor* cubeAxes = dynamic_cast<vtkCubeAxesActor*>(actor);

	cubeAxes->SetBounds(bb.getBounds());
}

void AxesPrivate::modifyActor(vtkRenderer* renderer, vtkActor* actor) const {
	vtkCubeAxesActor* axes = dynamic_cast<vtkCubeAxesActor*>(actor);
	Q_ASSERT(axes != 0);

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