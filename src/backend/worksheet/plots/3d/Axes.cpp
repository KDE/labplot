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

BASIC_SHARED_D_READER_IMPL(Axes, Axes::Format, formatX, formatX)
BASIC_SHARED_D_READER_IMPL(Axes, Axes::Format, formatY, formatY)
BASIC_SHARED_D_READER_IMPL(Axes, Axes::Format, formatZ, formatZ)
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

STD_SETTER_CMD_IMPL_F_S(Axes, SetFormatX, Axes::Format, formatX, update)
STD_SETTER_IMPL(Axes, FormatX, Axes::Format, formatX, "%1: format x changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetFormatY, Axes::Format, formatY, update)
STD_SETTER_IMPL(Axes, FormatY, Axes::Format, formatY, "%1: format y changed")

STD_SETTER_CMD_IMPL_F_S(Axes, SetFormatZ, Axes::Format, formatZ, update)
STD_SETTER_IMPL(Axes, FormatZ, Axes::Format, formatZ, "%1: format z changed")

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
	, formatX(Axes::Format_Decimal)
	, formatY(Axes::Format_Decimal)
	, formatZ(Axes::Format_Decimal)
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
	QString numToString(double num, Axes::Format format) {
		if (format == Axes::Format_Decimal)
			return QString::number(num, 'f', 2);
		if (format == Axes::Format_Scientific)
			return QString::number(num, 'e', 1);
		if (format == Axes::Format_PowerOf10)
			return QString("10^") + QString::number(log10(num), 'f', 2);
		if (format == Axes::Format_PowerOf2)
			return QString("2^") + QString::number(log2(num), 'f', 2);
		if (format == Axes::Format_PowerOfE)
			return QString("e^") + QString::number(log(num), 'f', 2);
		return QString::number(num / M_PI, 'f', 2) + QString("Pi");
	}
	void addLabels(double init, double delta, int numIter,
			vtkStringArray* labels, double factor, Axes::Format format) {
		if (factor != 0) {
			for (int i = 0; i < numIter; ++i) {
				const double scaledLabel = std::pow(factor, init + i * delta);
				labels->InsertValue(i, numToString(scaledLabel, format).toAscii());
			}

			for (int i = numIter; i > 0; --i) {
				if (labels->GetValue(i).compare(labels->GetValue(i - 1)) == 0)
					labels->GetValue(i) = "";
			}
		} else {
			for (int i = 0; i < numIter; ++i) {
				const double scaledLabel = init + i * delta;
				labels->InsertValue(i, numToString(scaledLabel, format).toAscii());
			}
		}
	}

	double scaleFactor(Plot3D::Scaling scale) {
		if (scale == Plot3D::Scaling_Log10)
			return 10;
		if (scale == Plot3D::Scaling_Log2)
			return 2;
		if (scale == Plot3D::Scaling_Ln)
			return M_E;
		return 0;
	}

	// int calculateNumIter(double length, double maxLength) {
	// 	const int iter = floor(abs(6 * length / maxLength));
	// 	return iter < 2 ? 2 : iter;
	// }
}

void AxesPrivate::updateLabels(const BoundingBox& bounds, vtkCubeAxesActor* cubeAxes) const {
	qDebug() << "Scale X Axes";
	// const int numXValues = calculateNumIter(bounds.GetLength(0), bounds.GetMaxLength());
	const int numXValues = 6;
	qDebug() << "Num X Values:" << numXValues;
	const double dx = (bounds.xMax() - bounds.xMin()) / (numXValues - 1);
	vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();

	addLabels(bounds.xMin(), dx, numXValues, labels, scaleFactor(xScaling), formatX);

	cubeAxes->SetAxisLabels(0, labels);

	qDebug() << "Scale Y Axes";
	//const int numYValues = calculateNumIter(bounds.GetLength(1), bounds.GetMaxLength());
	const int numYValues = 6;
	qDebug() << "Num Y Values:" << numYValues;
	const double dy = (bounds.yMax() - bounds.yMin()) / (numYValues - 1);
	labels = vtkSmartPointer<vtkStringArray>::New();

	addLabels(bounds.yMin(), dy, numYValues, labels, scaleFactor(yScaling), formatY);

	cubeAxes->SetAxisLabels(1, labels);

	qDebug() << "Scale Z Axes";
	//const int numZValues = calculateNumIter(bounds.GetLength(2), bounds.GetMaxLength());
	const int numZValues = 6;
	qDebug() << "Num Z Values:" << numZValues;
	const double dz = (bounds.zMax() - bounds.zMin()) / (numZValues - 1);
	labels = vtkSmartPointer<vtkStringArray>::New();

	addLabels(bounds.zMin(), dz, numZValues, labels, scaleFactor(zScaling), formatZ);

	cubeAxes->SetAxisLabels(2, labels);
}

void AxesPrivate::objectScaled(vtkActor* actor) const {
	BoundingBox bounds = systemBounds();
	makeCube(bounds);
	vtkCubeAxesActor* cubeAxes = dynamic_cast<vtkCubeAxesActor*>(actor);
	updateLabels(bounds, cubeAxes);
}

void AxesPrivate::updateBounds(vtkActor* actor) const {
	if (!isInitialized())
		return;

	BoundingBox bounds = systemBounds();
	makeCube(bounds);
	vtkCubeAxesActor* cubeAxes = dynamic_cast<vtkCubeAxesActor*>(actor);

	cubeAxes->SetBounds(bounds.getBounds());

	updateLabels(bounds, cubeAxes);
}

void AxesPrivate::modifyActor(vtkRenderer* renderer, vtkActor* actor) const {
	vtkCubeAxesActor* axes = dynamic_cast<vtkCubeAxesActor*>(actor);
	Q_ASSERT(axes != 0);

	axes->SetCamera(renderer->GetActiveCamera());

	axes->SetXTitle(xLabel.toAscii());
	axes->SetYTitle(yLabel.toAscii());
	axes->SetZTitle(zLabel.toAscii());

	//axes->SetLabelScaling(false, 0, 0, 0);
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