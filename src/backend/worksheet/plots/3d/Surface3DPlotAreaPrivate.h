#ifndef SURFACE3DPLOTAREAPRIVATE_H
#define SURFACE3DPLOTAREAPRIVATE_H

#include "Surface3DPlotArea.h"

#include <backend/worksheet/WorksheetElementPrivate.h>

#include <backend/matrix/Matrix.h>
class Surface3DPlotArea;
class WorksheetElementPrivate;
class Surface3DPlotAreaPrivate : public WorksheetElementPrivate {
public:
	explicit Surface3DPlotAreaPrivate(Surface3DPlotArea* owner);
	Surface3DPlotArea* const q{nullptr};
	Surface3DPlotArea::DataSource sourceType;
	Surface3DPlotArea::MeshType meshType;
	Surface3DPlotArea::DrawMode drawMode;
	bool flatShading;
	bool gridVisibility;
	Surface3DPlotArea::ShadowQuality shadowQuality;
	bool smooth;

	QColor color;
	double opacity;

	// Spreadsheet properties
	const AbstractColumn* xColumn;
	const AbstractColumn* yColumn;
	const AbstractColumn* zColumn;

	// Matrix properties
	const Matrix* matrix;
	QString matrixPath;

	const AbstractColumn* firstNode;
	const AbstractColumn* secondNode;
	const AbstractColumn* thirdNode;

	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	QString firstNodePath;
	QString secondNodePath;
	QString thirdNodePath;

	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	void generateData() const;
	// Data generation
	void generateDemoData() const;
	void generateMatrixData() const;
	void generateSpreadsheetData() const;

	// Export
	void saveSpreadsheetConfig(QXmlStreamWriter*) const;
	void saveMatrixConfig(QXmlStreamWriter*) const;

	// Import
	bool loadSpreadsheetConfig(XmlStreamReader*);
	bool loadMatrixConfig(XmlStreamReader*);
	void recalc();
};
#endif // SURFACE3DPLOTAREAPRIVATE_H
