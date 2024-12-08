/*
	File                 : Heatmap.h
	Project              : LabPlot
	Description          : Heatmap
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HEATMAP_H
#define HEATMAP_H

#include "Plot.h"
#include "qcolor.h"

class HeatmapPrivate;
class AbstractColumn;
class Matrix;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Heatmap : public Plot {
#else
class Heatmap : public Plot {
#endif
	Q_OBJECT
public:
	using Dimension = CartesianCoordinateSystem::Dimension;

	explicit Heatmap(const QString& name);
	virtual ~Heatmap();

	bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	bool indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const override;
	double minimum(Dimension dim) const override;
	double maximum(Dimension dim) const override;

	bool hasData() const override;
	int dataCount(Dimension dim) const override;
	QColor color() const override;

	bool usingColumn(const AbstractColumn*, bool indirect = true) const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	bool activatePlot(QPointF mouseScenePos, double maxDist = -1);

	void recalc() override;

	enum class DataSource {
		Matrix,
		Spreadsheet,
	};
	static QLatin1String saveName;

	BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)

	/*!
	 * Number of bins when using Spreadsheet as datasource. This is required to define
	 * the size of the Bins. The size of the bin is defined as (maxVal - minVal) / numBins
	 */
	BASIC_D_ACCESSOR_DECL(bool, numBinsEqual, NumBinsEqual)
	BASIC_D_ACCESSOR_DECL(unsigned int, xNumBins, XNumBins)
	BASIC_D_ACCESSOR_DECL(unsigned int, yNumBins, YNumBins)
	/*!
	 * Only for spreadsheet! Drawing not handled positions
	 * and including into the value range or ignoring those values
	 */
	BASIC_D_ACCESSOR_DECL(bool, drawEmpty, DrawEmpty)

	// DataSource::Spreadsheet
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)

	POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix)
	CLASS_D_ACCESSOR_DECL(QString, matrixPath, MatrixPath)

	BASIC_D_ACCESSOR_DECL(bool, automaticLimits, AutomaticLimits) // If true, the limits of format are automatically determined
	struct Format {
		double start = 0.0;
		double end = 1.0;
		QString name;
		std::vector<QColor> colors;
		bool operator!=(const Format& rhs) const;
		int index(double value) const;
		QColor color(double value) const;
	};
	CLASS_D_ACCESSOR_DECL(Format, format, Format)
	BASIC_D_ACCESSOR_DECL(double, formatMin, FormatMin)
	BASIC_D_ACCESSOR_DECL(double, formatMax, FormatMax)
	CLASS_D_ACCESSOR_DECL(QString, formatName, FormatName)
	CLASS_D_ACCESSOR_DECL(std::vector<QColor>, formatColors, FormatColors)

	void retransform() override;

	using Private = HeatmapPrivate;

protected:
	void handleAspectUpdated(const QString& path, const AbstractAspect* aspect);

Q_SIGNALS:
	void valueDrawn(double xPosStart, double yPosStart, double xPosEnd, double yPosEnd, double value);
	void xDataChanged();
	void yDataChanged();
	void dataChanged();

	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void matrixChanged(const Matrix*);
	void drawEmptyChanged(bool);

	void xNumBinsChanged(unsigned int);
	void yNumBinsChanged(unsigned int);

	void formatChanged(const Format&);

private:
	Q_DECLARE_PRIVATE(Heatmap)

	void connectXColumn(const AbstractColumn*);
	void connectYColumn(const AbstractColumn*);
	void connectMatrix(const Matrix*);
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void matrixAboutToBeRemoved(const AbstractAspect*);
	void xColumnNameChanged();
	void yColumnNameChanged();
	void matrixNameChanged();

	const AbstractColumn* column(Dimension) const;

	bool minMaxSpreadsheet(const Dimension, const Range<int>& indexRange, Range<double>& r) const;
	bool minMaxMatrix(const Dimension, const Range<int>& indexRange, Range<double>& r) const;
	bool indicesMinMaxMatrix(const Dimension, double v1, double v2, int& start, int& end) const;
	// void recalcLogicalPoints();
	double minimumSpreadsheet(Dimension) const;
	double minimumMatrix(Dimension) const;
	double maximumSpreadsheet(Dimension) const;
	double maximumMatrix(Dimension) const;

	friend class HeatmapSetDataSourceCmd;
	friend class HeatmapSetXColumnCmd;
	friend class HeatmapSetYColumnCmd;
	friend class HeatmapSetMatrixCmd;
	friend class HeatmapTest;
};

#endif // HEATMAP_H
