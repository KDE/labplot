/*
	File                 : Heatmap.cpp
	Project              : LabPlot
	Description          : Heatmap
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Heatmap.h"
#include "HeatmapPrivate.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macrosCurve.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "tools/ImageTools.h"

#include <QPainter>
#include <QPalette>
#include <QXmlStreamWriter>

#include <KConfigGroup>

QLatin1String Heatmap::saveName = QLatin1String("Heatmap");

using Dimension = CartesianCoordinateSystem::Dimension;

bool Heatmap::Format::operator!=(const Format& rhs) const {
	if (min != rhs.min || max != rhs.max || name != rhs.name || colors.size() != rhs.colors.size())
		return true;
	auto i = colors.begin();
	auto j = rhs.colors.begin();

	while (i != colors.end()) {
		if (*i != *j)
			return true;
		i++;
		j++;
	}
	return false;
}

int Heatmap::Format::index(double value) const {
	if (colors.size() == 0)
		return -1;
	const auto index = static_cast<int>((value - min) / (max - min) * colors.size());
	if (index < 0)
		return 0;
	else if (index >= colors.size())
		return colors.size() - 1;
	return index;
}

QColor Heatmap::Format::color(double value) const {
	const auto index = this->index(value);
	if (index >= 0)
		return colors.at(index);
	return QColor();
}

CURVE_COLUMN_CONNECT(Heatmap, X, x, recalc)
CURVE_COLUMN_CONNECT(Heatmap, Y, y, recalc)
void Heatmap::connectMatrix(const Matrix* matrix) {
	connect(matrix, &AbstractAspect::aspectAboutToBeRemoved, this, &Heatmap::matrixAboutToBeRemoved);
	/* When the matrix is reused with different name, the heatmap should be informed to disconnect */
	/* connect(matrix, &Matrix::reset, this, &Heatmap::matrixAboutToBeRemoved);   */
	connect(matrix, &AbstractAspect::aspectDescriptionChanged, this, &Heatmap::matrixNameChanged);

	connect(matrix, &Matrix::dataChanged, this, &Heatmap::recalc);
	connect(matrix, &Matrix::xStartChanged, this, &Heatmap::recalc);
	connect(matrix, &Matrix::xEndChanged, this, &Heatmap::recalc);
	connect(matrix, &Matrix::yStartChanged, this, &Heatmap::recalc);
	connect(matrix, &Matrix::yEndChanged, this, &Heatmap::recalc);

	connect(matrix, &Matrix::dataChanged, this, &Heatmap::dataChanged);
	connect(matrix, &Matrix::xStartChanged, this, &Heatmap::xDataChanged);
	connect(matrix, &Matrix::xEndChanged, this, &Heatmap::xDataChanged);
	connect(matrix, &Matrix::yStartChanged, this, &Heatmap::yDataChanged);
	connect(matrix, &Matrix::yEndChanged, this, &Heatmap::yDataChanged);
}

Heatmap::Heatmap(const QString& name)
	: Plot(name, new HeatmapPrivate(this), AspectType::Heatmap) {
}

Heatmap::~Heatmap() {
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(Heatmap, Heatmap::DataSource, dataSource, dataSource)
BASIC_SHARED_D_READER_IMPL(Heatmap, bool, equalNumberBins, equalNumberBins)
BASIC_SHARED_D_READER_IMPL(Heatmap, unsigned int, xNumberBins, xNumberBins)
BASIC_SHARED_D_READER_IMPL(Heatmap, unsigned int, yNumberBins, yNumberBins)
BASIC_SHARED_D_READER_IMPL(Heatmap, bool, matrixNumberBins, matrixNumberBins)
BASIC_SHARED_D_READER_IMPL(Heatmap, bool, drawEmpty, drawEmpty)
BASIC_SHARED_D_READER_IMPL(Heatmap, bool, automaticLimits, automaticLimits)
BASIC_SHARED_D_READER_IMPL(Heatmap, Heatmap::Format, format, format)
BASIC_SHARED_D_READER_IMPL(Heatmap, double, formatMin, format.min)
BASIC_SHARED_D_READER_IMPL(Heatmap, double, formatMax, format.max)
BASIC_SHARED_D_READER_IMPL(Heatmap, QString, formatName, format.name)
BASIC_SHARED_D_READER_IMPL(Heatmap, QVector<QColor>, formatColors, format.colors)
BASIC_SHARED_D_READER_IMPL(Heatmap, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(Heatmap, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(Heatmap, const Matrix*, matrix, matrix)
BASIC_SHARED_D_READER_IMPL(Heatmap, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(Heatmap, QString, yColumnPath, yColumnPath)
BASIC_SHARED_D_READER_IMPL(Heatmap, QString, matrixPath, matrixPath)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// TODO: disconnect matrix (only if currently active)!
//  TODO: disconnect columns if matrix will be connected (conenct matrix, only if currently active!)
class HeatmapSetDataSourceCmd : public StandardSetterCmd<Heatmap::Private, Heatmap::DataSource> {
public:
	HeatmapSetDataSourceCmd(Heatmap::Private* target, Heatmap::DataSource newValue, const KLocalizedString& description)
		: StandardSetterCmd<Heatmap::Private, Heatmap::DataSource>(target, &Heatmap::Private::dataSource, newValue, description) {
	}
	void connectDisconnect(const Heatmap::DataSource newDataSource) const {
		switch (newDataSource) {
		case Heatmap::DataSource::Matrix: { /* disconnect only when column valid, because otherwise all signals are disconnected */
			if (m_target->xColumn)
				QObject::disconnect(m_target->xColumn, nullptr, m_target->q, nullptr);
			if (m_target->yColumn)
				QObject::disconnect(m_target->yColumn, nullptr, m_target->q, nullptr);
			if (m_target->matrix)
				m_target->q->connectMatrix(m_target->matrix);
			break;
		}
		case Heatmap::DataSource::Spreadsheet: {
			if (m_target->xColumn)
				CURVE_COLUMN_CONNECT_CALL(m_target->q, m_target->xColumn, X);
			if (m_target->yColumn)
				CURVE_COLUMN_CONNECT_CALL(m_target->q, m_target->yColumn, X);
			if (m_target->matrix)
				QObject::disconnect(m_target->matrix, nullptr, m_target->q, nullptr);
			break;
		}
		}
	}

	void finalize() override {
		connectDisconnect(m_target->dataSource);
		/* emit DataChanged() in order to notify the plot about the changes */
		Q_EMIT m_target->q->dataChanged();
	}

private:
};

void Heatmap::setDataSource(DataSource dataSource) {
	Q_D(Heatmap);
	if (dataSource != d->dataSource)
		exec(new HeatmapSetDataSourceCmd(d, dataSource, ki18n("%1: Datasource changed")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetFormat, Heatmap::Format, format, retransform)
void Heatmap::setFormat(const Heatmap::Format& format) {
	Q_D(Heatmap);
	if (format != d->format)
		exec(new HeatmapSetFormatCmd(d, format, ki18n("%1: format changed")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetAutomaticLimits, bool, automaticLimits, retransform)
void Heatmap::setAutomaticLimits(const bool automatic) {
	Q_D(Heatmap);
	if (automatic != d->automaticLimits)
		exec(new HeatmapSetAutomaticLimitsCmd(d, automatic, ki18n("%1: automatic limits changed")));
}

STRUCT_SETTER_CMD_IMPL_F_S(Heatmap, SetFormatMin, Heatmap::Format, format, min, double, retransform)
void Heatmap::setFormatMin(const double min) {
	Q_D(Heatmap);
	if (min != d->format.min)
		exec(new HeatmapSetFormatMinCmd(d, min, ki18n("%1: format min changed")));
}

STRUCT_SETTER_CMD_IMPL_F_S(Heatmap, SetFormatMax, Heatmap::Format, format, max, double, retransform)
void Heatmap::setFormatMax(const double max) {
	Q_D(Heatmap);
	if (max != d->format.max)
		exec(new HeatmapSetFormatMaxCmd(d, max, ki18n("%1: format max changed")));
}

STRUCT_SETTER_CMD_IMPL_F_S(Heatmap, SetFormatName, Heatmap::Format, format, name, QString, retransform)
void Heatmap::setFormatName(const QString& name) {
	Q_D(Heatmap);
	if (name != d->format.name)
		exec(new HeatmapSetFormatNameCmd(d, name, ki18n("%1: format name changed")));
}

STRUCT_SETTER_CMD_IMPL_F_S(Heatmap, SetFormatColors, Heatmap::Format, format, colors, QVector<QColor>, retransform)
void Heatmap::setFormatColors(const QVector<QColor>& colors) {
	Q_D(Heatmap);
	if (colors.size() == d->format.colors.size()) {
		bool equal = true;
		for (int i = 0; i < colors.size(); i++) {
			if (colors.at(i) != d->format.colors.at(i)) {
				equal = false;
				break;
			}
		}
		if (equal)
			return;
	}
	exec(new HeatmapSetFormatColorsCmd(d, colors, ki18n("%1: format colors changed")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetDrawEmpty, bool, drawEmpty, retransform)
void Heatmap::setDrawEmpty(bool drawEmpty) {
	Q_D(Heatmap);
	if (drawEmpty != d->drawEmpty)
		exec(new HeatmapSetDrawEmptyCmd(d, drawEmpty, ki18n("%1: drawEmpty changed")));
}

HEATMAP_COLUMN_SETTER_CMD_IMPL_S(Heatmap, X, x)
void Heatmap::setXColumn(const AbstractColumn* column) {
	Q_D(Heatmap);
	if (column != d->xColumn)
		exec(new HeatmapSetXColumnCmd(d, column, ki18n("%1: x-data source changed")));
}

HEATMAP_COLUMN_SETTER_CMD_IMPL_S(Heatmap, Y, y)
void Heatmap::setYColumn(const AbstractColumn* column) {
	Q_D(Heatmap);
	if (column != d->yColumn)
		exec(new HeatmapSetYColumnCmd(d, column, ki18n("%1: y-data source changed")));
}

void Heatmap::setXColumnPath(const QString& path) {
	Q_D(Heatmap);
	if (path != d->xColumnPath)
		d->xColumnPath = path;
}

void Heatmap::setYColumnPath(const QString& path) {
	Q_D(Heatmap);
	if (path != d->yColumnPath)
		d->yColumnPath = path;
}

void Heatmap::setMatrixPath(const QString& path) {
	Q_D(Heatmap);
	if (path != d->matrixPath)
		d->matrixPath = path;
}

class HeatmapSetMatrixCmd : public StandardSetterCmd<Heatmap::Private, const Matrix*> {
public:
	HeatmapSetMatrixCmd(HeatmapPrivate* target, const Matrix* newValue, const KLocalizedString& description)
		: StandardSetterCmd<HeatmapPrivate, const Matrix*>(target, &HeatmapPrivate::matrix, newValue, description)
		, m_private(target)
		, m_matrix(newValue) {
	}
	virtual void finalize() override {
		Q_EMIT m_target->q->matrixChanged(m_target->*m_field); // TODO: why this works????
		if (m_private->dataSource == Heatmap::DataSource::Matrix) { /* emit dataChanged() in order to notify the plot about the changes */
			Q_EMIT m_private->q->dataChanged();
		}
	}
	void redo() override {
		if (m_private->matrix) { /* disconnect only when column valid, because otherwise all \ signals are disconnected */
			QObject::disconnect(m_private->matrix, nullptr, m_private->q, nullptr);
		}
		auto* matrixOld = m_private->matrix;
		m_private->matrix = m_matrix;
		m_matrix = matrixOld;

		if (m_private->matrix) {
			m_private->q->setMatrixPath(m_private->matrix->path());
			if (m_private->dataSource == Heatmap::DataSource::Matrix) {
				m_private->q->connectMatrix(m_private->matrix);
			}
		} else
			m_private->q->setMatrixPath(QStringLiteral(""));
		finalize();
		// Q_EMIT m_private->q->matrixChanged(m_matrix);
	}
	void undo() override {
		redo();
	}

private:
	Heatmap::Private* m_private;
	const Matrix* m_matrix{nullptr};
};

void Heatmap::setMatrix(const Matrix* matrix) {
	Q_D(Heatmap);
	if (matrix != d->matrix)
		exec(new HeatmapSetMatrixCmd(d, matrix, ki18n("%1: matrix changed")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetMatrixNumberBins, bool, matrixNumberBins, retransform)
void Heatmap::setMatrixNumberBins(bool matrixNumberBins) {
	Q_D(Heatmap);
	if (matrixNumberBins != d->matrixNumberBins)
		exec(new HeatmapSetMatrixNumberBinsCmd(d, matrixNumberBins, ki18n("%1: Set matrix number bins")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetEqualNumberBins, bool, equalNumberBins, retransform)
void Heatmap::setEqualNumberBins(bool equal) {
	Q_D(Heatmap);
	if (equal != d->equalNumberBins)
		exec(new HeatmapSetEqualNumberBinsCmd(d, equal, ki18n("%1: Set number bins equal")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetXNumberBins, unsigned int, xNumberBins, retransform)
void Heatmap::setXNumberBins(unsigned int numBins) {
	Q_D(Heatmap);
	if (numBins != d->xNumberBins)
		exec(new HeatmapSetXNumberBinsCmd(d, numBins, ki18n("%1: number bins for x changed")));
}

STD_SETTER_CMD_IMPL_F_S(Heatmap, SetYNumberBins, unsigned int, yNumberBins, retransform)
void Heatmap::setYNumberBins(unsigned int numBins) {
	Q_D(Heatmap);
	if (numBins != d->yNumberBins)
		exec(new HeatmapSetYNumberBinsCmd(d, numBins, ki18n("%1: number bins for y changed")));
}

// ##############################################################################

// void Heatmap::recalcLogicalPoints() {
//	Q_D(Heatmap);
//	d->recalcLogicalPoints();
// }

bool Heatmap::minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars) const {
	Q_UNUSED(includeErrorBars);
	switch (dataSource()) {
	case DataSource::Spreadsheet:
		return minMaxSpreadsheet(dim, indexRange, r);
	case DataSource::Matrix:
		return minMaxMatrix(dim, indexRange, r);
	}
	return false;
}

bool Heatmap::minMaxSpreadsheet(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r) const {
	switch (dim) {
	case Dimension::X:
		return XYCurve::minMax(xColumn(), yColumn(), ErrorBar::ErrorType::NoError, nullptr, nullptr, indexRange, r, false);
	case Dimension::Y:
		return XYCurve::minMax(yColumn(), xColumn(), ErrorBar::ErrorType::NoError, nullptr, nullptr, indexRange, r, false);
	}
	return false;
}

bool Heatmap::minMaxMatrix(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r) const {
	Q_D(const Heatmap);
	double numberElements = 0;
	double start = std::nan("0");
	double end = std::nan("0");
	switch (dim) {
	case Dimension::X: {
		start = d->matrix->xStart();
		end = d->matrix->xEnd();
		numberElements = d->matrix->columnCount();
		break;
	}
	case Dimension::Y:
		start = d->matrix->yStart();
		end = d->matrix->yEnd();
		numberElements = d->matrix->rowCount();
		break;
	}

	if (numberElements == 0)
		return false;

	const double diff = end - start;
	if (indexRange.start() <= 0)
		r.setStart(start);
	else if (indexRange.start() >= numberElements - 1)
		r.setStart(end);
	else
		r.setStart(start + diff / numberElements * indexRange.start());

	if (indexRange.end() >= numberElements - 1)
		r.setEnd(end);
	if (indexRange.end() <= 0)
		r.setEnd(start);
	else
		r.setEnd(start + diff / numberElements * indexRange.end());
	return false;
}

bool Heatmap::indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const {
	switch (dataSource()) {
	case DataSource::Spreadsheet: {
		if (column(dim))
			return column(dim)->indicesMinMax(v1, v2, start, end);
		break;
	}
	case DataSource::Matrix:
		return indicesMinMaxMatrix(dim, v1, v2, start, end);
	}
	return false;
}

bool Heatmap::indicesMinMaxMatrix(const Dimension dim, double v1, double v2, int& start, int& end) const {
	Q_D(const Heatmap);
	double numberElements = 0;
	double startValue = 0;
	double endValue = 0;
	switch (dim) {
	case Dimension::X: {
		startValue = d->matrix->xStart();
		endValue = d->matrix->xEnd();
		numberElements = d->matrix->columnCount();
		break;
	}
	case Dimension::Y:
		startValue = d->matrix->yStart();
		endValue = d->matrix->yEnd();
		numberElements = d->matrix->rowCount();
		break;
	}

	if (v1 > v2)
		qSwap(v1, v2);

	if (startValue > endValue)
		qSwap(startValue, endValue);

	if (numberElements == 0)
		return false;

	const double diff = endValue - startValue;
	const bool increasing = diff > 0;

	if ((increasing && v1 >= endValue) || (!increasing && v1 <= endValue)) {
		start = numberElements - 1; // last index
	} else if ((increasing && v1 <= startValue) || (!increasing && v1 >= startValue))
		start = 0;
	else {
		start = static_cast<int>((v1 - startValue) / diff * (numberElements - 1));
	}

	if ((increasing && v2 >= endValue) || (!increasing && v2 <= endValue))
		end = numberElements - 1; // last index
	else if ((increasing && v2 <= startValue) || (!increasing && v2 >= startValue))
		end = 0;
	else {
		end = ceil((v2 - startValue) / diff * (numberElements - 1));
	}
	return true;
}

double Heatmap::minimum(Dimension dim) const {
	switch (dataSource()) {
	case DataSource::Spreadsheet:
		return minimumSpreadsheet(dim);
	case DataSource::Matrix:
		return minimumMatrix(dim);
	}
	Q_ASSERT(false);
	return std::nan("0");
}

double Heatmap::maximum(Dimension dim) const {
	switch (dataSource()) {
	case DataSource::Spreadsheet:
		return maximumSpreadsheet(dim);
	case DataSource::Matrix:
		return maximumMatrix(dim);
	}
	Q_ASSERT(false);
	return std::nan("0");
}

double Heatmap::minimumSpreadsheet(Dimension dim) const {
	if (auto* c = column(dim))
		return c->minimum();
	return std::nan("0");
}
double Heatmap::minimumMatrix(Dimension dim) const {
	Q_D(const Heatmap);
	switch (dim) {
	case Dimension::X: {
		if (d->matrix)
			return qMin(d->matrix->xStart(), d->matrix->xEnd());
		break;
	}
	case Dimension::Y: {
		if (d->matrix)
			return qMin(d->matrix->yStart(), d->matrix->yEnd());
		break;
	}
	}
	Q_ASSERT(false);
	return std::nan("0");
}
double Heatmap::maximumSpreadsheet(Dimension dim) const {
	if (auto* c = column(dim))
		return c->maximum();
	return std::nan("0");
}
double Heatmap::maximumMatrix(Dimension dim) const {
	Q_D(const Heatmap);
	switch (dim) {
	case Dimension::X: {
		if (d->matrix)
			return qMax(d->matrix->xStart(), d->matrix->xEnd());
		break;
	}
	case Dimension::Y: {
		if (d->matrix)
			return qMax(d->matrix->yStart(), d->matrix->yEnd());
		break;
	}
	}
	Q_ASSERT(false);
	return std::nan("0");
}

bool Heatmap::hasData() const {
	Q_D(const Heatmap);
	switch (d->dataSource) {
	case DataSource::Spreadsheet:
		return (d->xColumn != nullptr && d->yColumn != nullptr);
	case DataSource::Matrix:
		return (d->matrix != nullptr);
	}
	Q_ASSERT(false);
	return false;
}

int Heatmap::dataCount(Dimension dim) const {
	Q_D(const Heatmap);
	if (!hasData())
		return 0;

	switch (d->dataSource) {
	case DataSource::Spreadsheet: {
		return dim == Dimension::X ? d->xColumn->rowCount() : d->yColumn->rowCount();
	}
	case DataSource::Matrix:
		return dim == Dimension::X ? d->matrix->columnCount() : d->matrix->rowCount();
	}
	Q_ASSERT(false);
	return 0;
}

bool Heatmap::usingColumn(const AbstractColumn* column, bool indirect) const {
	Q_D(const Heatmap);
	if (d->dataSource == DataSource::Spreadsheet) {
		return d->xColumn == column || d->yColumn == column;
	}
	return false;
}

void Heatmap::recalc() {
	D(Heatmap);
	d->recalc();
}

QColor Heatmap::color() const {
	return QColor();
}

bool Heatmap::activatePlot(QPointF mouseScenePos, double maxDist) {
	Q_ASSERT(false); // Not yet implemented
	return false;
}

void Heatmap::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Heatmap);
	if (aspect == d->xColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->xColumn = nullptr;
		if (d->dataSource == DataSource::Spreadsheet) {
			CURVE_COLUMN_REMOVED(x);
		}
	}
}

const AbstractColumn* Heatmap::column(Dimension dim) const {
	Q_D(const Heatmap);
	switch (dim) {
	case Dimension::X: {
		return d->xColumn;
	}
	case Dimension::Y: {
		return d->yColumn;
	}
	}
	assert(false);
	return nullptr;
}

void Heatmap::xColumnNameChanged() {
	Q_D(Heatmap);
	setXColumnPath(d->xColumn->path());
}

void Heatmap::yColumnNameChanged() {
	Q_D(Heatmap);
	setYColumnPath(d->yColumn->path());
}

void Heatmap::matrixNameChanged() {
	Q_D(Heatmap);
	setMatrixPath(d->matrix->path());
}

void Heatmap::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Heatmap);
	if (aspect == d->yColumn) {
		disconnect(aspect, nullptr, this, nullptr);
		d->yColumn = nullptr;
		if (d->dataSource == DataSource::Spreadsheet) {
			CURVE_COLUMN_REMOVED(x);
		}
	}
}

void Heatmap::matrixAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Heatmap);
	if (aspect == d->matrix) {
		disconnect(aspect, nullptr, this, nullptr);
		d->matrix = nullptr;
		if (d->dataSource == DataSource::Matrix)
			d->retransform();
	}
}

void Heatmap::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	DEBUG(Q_FUNC_INFO);
	// 	Q_D(Heatmap);
	// 	double ratio = 0;
	// 	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
	// 		ratio = std::max(horizontalRatio, verticalRatio);
	// 	else
	// 		ratio = std::min(horizontalRatio, verticalRatio);
}

void Heatmap::retransform() {
	Q_D(Heatmap);
	d->retransform();
}

void Heatmap::handleAspectUpdated(const QString& path, const AbstractAspect* aspect) {
	Q_D(Heatmap);

	const auto column = dynamic_cast<const AbstractColumn*>(aspect);

	setUndoAware(false);

	if (path == d->xColumnPath)
		setXColumn(column);
	if (path == d->yColumnPath)
		setYColumn(column);

	const auto matrix = dynamic_cast<const Matrix*>(aspect);
	if (path == d->matrixPath)
		setMatrix(matrix);

	setUndoAware(true);
}

// #################################################################################################
// ##### Heatmap Private ###########################################################################
// #################################################################################################

HeatmapPrivate::HeatmapPrivate(Heatmap* heatmap)
	: PlotPrivate(heatmap)
	, q(heatmap) {
}

void HeatmapPrivate::retransform() {
	const QRectF& r = update();
	recalcShapeAndBoundingRect(r);
}

void HeatmapPrivate::recalc() {
}

void HeatmapPrivate::recalcShapeAndBoundingRect() {
	recalcShapeAndBoundingRect(m_boundingRectangle);
}

QRectF HeatmapPrivate::update() {
	// TODO: create CalculateScenePoints
	data.clear();
	int xNumValues = 0, yNumValues = 0;
	switch (dataSource) {
	case Heatmap::DataSource::Spreadsheet:
		if (!xColumn || !yColumn) {
			DEBUG(Q_FUNC_INFO << ", WARNING: xColumn or yColumn not available");
			return QRectF();
		}

		if (xColumn->rowCount() != yColumn->rowCount()) {
			DEBUG(Q_FUNC_INFO << ", WARNING: xColumn and yColumn do not have the same size");
			return QRectF();
		}

		if (!xColumn->isNumeric() || !yColumn->isNumeric()) {
			DEBUG(Q_FUNC_INFO << ", WARNING: xColumn or yColumn not numeric");
			return QRectF();
		}
		xNumValues = xColumn->rowCount();
		yNumValues = yColumn->rowCount();
		break;
	case Heatmap::DataSource::Matrix:
		if (!matrix) {
			DEBUG(Q_FUNC_INFO << ", WARNING: matrix not available");
			return QRectF();
		}
		xNumValues = matrix->columnCount();
		yNumValues = matrix->rowCount();
		break;
	}

	if (xNumValues == 0 || yNumValues == 0)
		return QRectF();

	// Range<double> xRange;
	// minMax(Dimension::X, Range<int>(0, xColumn->rowCount(), xRange);
	// Range<double> yRange;
	// minMax(Dimension::Y, Range<int>(0, yColumn->rowCount(), yRange);
	// TODO: Bin size for linear ranges. Required also for logarithmic?
	auto xMin = q->minimum(Dimension::X);
	auto yMin = q->minimum(Dimension::Y);
	auto xMax = q->maximum(Dimension::X);
	auto yMax = q->maximum(Dimension::Y);

	auto xBinSize = xMax - xMin;
	auto yBinSize = yMax - yMin;
	if (matrixNumberBins && dataSource == Heatmap::DataSource::Matrix) {
		xBinSize /= xNumValues;
		yBinSize /= yNumValues;
	} else if (equalNumberBins) {
		xBinSize /= xNumberBins;
		yBinSize /= xNumberBins;
	} else {
		xBinSize /= xNumberBins;
		yBinSize /= yNumberBins;
	}

	if (xBinSize <= 0 || yBinSize <= 0)
		return QRectF();

	// Check which region is visible
	const auto xPlotRange = q->plot()->range(Dimension::X, q->coordinateSystemIndex());
	const auto xIncreasing = xPlotRange.length() > 0;
	const auto yPlotRange = q->plot()->range(Dimension::Y, q->coordinateSystemIndex());
	const auto yIncreasing = yPlotRange.length() > 0;

	const double xRangeMin = qMin(xPlotRange.start(), xPlotRange.end());
	const double xRangeMax = qMax(xPlotRange.start(), xPlotRange.end());
	const double yRangeMin = qMin(yPlotRange.start(), yPlotRange.end());
	const double yRangeMax = qMax(yPlotRange.start(), yPlotRange.end());

	auto calculate_numberBinsVisible = [](double min, double max, double& minValid, double& maxValid, double binSize, double rangeMin, double rangeMax) {
		// min + nstart * binSize >= rangeMin --> nstart = floor((rangeMin - min) / binSize)
		int nStart = floor((rangeMin - min) / binSize);
		// min + nend * binSize >= rangeMax --> nend = ceil((rangeMax - min) / binSize)
		int nEnd = ceil((qMin(max, rangeMax) - min) / binSize);

		if (nStart < 0) {
			// min larger than range min
			nStart = 0;
		}

		if (nEnd < 0) {
			// min larger than range max
			nEnd = 0;
			return -1;
		}

		minValid = min + nStart * binSize;
		maxValid = min + nEnd * binSize;
		return nEnd - nStart;
	};

	double xMinValid;
	double xMaxValid;
	int xNumberBinsVisible = calculate_numberBinsVisible(xMin, xMax, xMinValid, xMaxValid, xBinSize, xRangeMin, xRangeMax);
	if (xNumberBinsVisible < 0) {
		// Can happen if the column does not contain any valid values
		return QRectF();
	}

	double yMinValid;
	double yMaxValid;
	int yNumberBinsVisible = calculate_numberBinsVisible(yMin, yMax, yMinValid, yMaxValid, yBinSize, yRangeMin, yRangeMax);
	if (yNumberBinsVisible < 0) {
		// Can happen if the column does not contain any valid values
		return QRectF();
	}

	std::vector<std::vector<double>> map(xNumberBinsVisible, std::vector<double>(yNumberBinsVisible, 0));
	double minValue = INFINITY;
	double maxValue = -INFINITY;

	auto calculateIndex = [](double val, double maxData, double minValid, double maxValid, double binSize, int numberBinsVisible) {
		// TODO: make option if the border shall be included, or not
		if (val == maxValid && val == maxData)
			return numberBinsVisible - 1; // include Right Border
		const auto index = (val - minValid) / binSize;
		if (index < 0)
			return -1;
		return static_cast<int>(floor(index));
	};

	switch (dataSource) {
	case Heatmap::DataSource::Spreadsheet: {
		// For spreadsheets the values are counts of the occurances
		if (drawEmpty) {
			minValue = 0;
			maxValue = 0;
		} else {
			minValue = 1;
			maxValue = 1;
		}

		// mapping values to index map
		// xColumn and yColumn have same size, was checked above
		int xIndex, yIndex;
		for (int i = 0; i < xNumValues; i++) {
			if (!xColumn->isValid(i) || !yColumn->isValid(i) || xColumn->isMasked(i) || yColumn->isMasked(i))
				continue;
			const auto xVal = xColumn->valueAt(i);
			const auto yVal = yColumn->valueAt(i);

			xIndex = calculateIndex(xVal, xMax, xMinValid, xMaxValid, xBinSize, xNumberBinsVisible);
			yIndex = calculateIndex(yVal, yMax, yMinValid, yMaxValid, yBinSize, yNumberBinsVisible);

			if (xIndex >= 0 && xIndex < xNumberBinsVisible && yIndex >= 0 && yIndex < yNumberBinsVisible) {
				map[xIndex][yIndex] += 1; // Summing up
				if (map[xIndex][yIndex] > maxValue)
					maxValue = map[xIndex][yIndex];
			}
		}
		break;
	}
	case Heatmap::DataSource::Matrix:
		const double xStepSize = (xMax - xMin) / xNumValues;
		const double yStepSize = (yMax - yMin) / yNumValues;
		for (int row = 0; row < yNumValues; row++) {
			const double yVal = yMin + row * yStepSize + 0.5 * yStepSize; // 0.5 * yStepSize because it is assuming that a cell is in the center
			const int yIndex = calculateIndex(yVal, yMax, yMinValid, yMaxValid, yBinSize, yNumberBinsVisible);
			for (int column = 0; column < xNumValues; column++) {
				const double xVal = xMin + column * xStepSize + 0.5 * xStepSize;
				const int xIndex = calculateIndex(xVal, xMax, xMinValid, xMaxValid, xBinSize, xNumberBinsVisible);

				const double value = matrix->cell<double>(row, column);
				if (value > maxValue)
					maxValue = value;
				if (value < minValue)
					minValue = value;

				if (xIndex >= 0 && xIndex < xNumberBinsVisible && yIndex >= 0 && yIndex < yNumberBinsVisible)
					map[xIndex][yIndex] = value;
			}
		}
		break;
	}

	if (xNumberBinsVisible * yNumberBinsVisible <= 25) {
		// This part is used for the tests only
		for (int yIndex = 0; yIndex < yNumberBinsVisible; yIndex++) {
			for (int xIndex = 0; xIndex < xNumberBinsVisible; xIndex++) {
				const double value = map[xIndex][yIndex];
				if (dataSource == Heatmap::DataSource::Spreadsheet && !drawEmpty && value == 0)
					continue;
				const double xPosStart = xMinValid + xIndex * xBinSize;
				const double yPosStart = yMinValid + yIndex * yBinSize;
				const double xPosEnd = xPosStart + xBinSize;
				const double yPosEnd = yPosStart + yBinSize;
				Q_EMIT q->valueDrawn(xPosStart, yPosStart, xPosEnd, yPosEnd, value);
			}
		}
	}

	// Adjust formatting
	if (automaticLimits) {
		format.min = minValue;
		format.max = maxValue;
	}

	// Calculate rectangles
	const auto* cSystem = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	Points points(2);
	for (int yIndex = 0; yIndex < yNumberBinsVisible; yIndex++) {
		for (int xIndex = 0; xIndex < xNumberBinsVisible; xIndex++) {
			const double value = map[xIndex][yIndex];
			if (dataSource == Heatmap::DataSource::Spreadsheet && !drawEmpty && value == 0)
				continue;
			const double xPosStart = xMinValid + xIndex * xBinSize;
			const double yPosStart = yMinValid + yIndex * yBinSize;
			const double xPosEnd = xPosStart + xBinSize;
			const double yPosEnd = yPosStart + yBinSize;
			points[0] = QPointF(xPosStart, yPosStart);
			points[1] = QPointF(xPosEnd, yPosEnd);
			cSystem->mapLogicalToSceneFast(points, AbstractCoordinateSystem::MappingFlag::Limit);
			assert(points.size() == 2);
			data.push_back({QRectF(points.at(0), points.at(1)), format.color(value)}); // TODO: storing QColor or just value and convertering every time
			// TODO: sort and cluster the rectangles?
		}
	}

	const auto xEndPoint = xMinValid + xNumberBinsVisible * xBinSize;
	const auto yEndPoint = yMinValid + yNumberBinsVisible * yBinSize;

	points[0] = QPointF(xMinValid, yMinValid);
	points[1] = QPointF(xEndPoint, yEndPoint);
	cSystem->mapLogicalToSceneFast(points, AbstractCoordinateSystem::MappingFlag::Limit);

	return QRectF(qMin(points.at(0).x(), points.at(1).x()),
				  qMin(points.at(0).y(), points.at(1).y()),
				  qAbs(points.at(0).x() - points.at(1).x()),
				  qAbs(points.at(0).y() - points.at(1).y())); // New bounding rectangle
}

void HeatmapPrivate::updatePixmap() {
	DEBUG(Q_FUNC_INFO << ", suppressRecalc = " << suppressRecalc);
	if (suppressRecalc)
		return;

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	if (m_boundingRectangle.width() == 0 || m_boundingRectangle.height() == 0) {
		DEBUG(Q_FUNC_INFO << ", boundingRectangle.width() or boundingRectangle.height() == 0");
		m_pixmap = QPixmap();
		return;
	}
	m_pixmap = QPixmap(ceil(m_boundingRectangle.width()), ceil(m_boundingRectangle.height()));
	m_pixmap.fill(Qt::transparent);
	QPainter painter(&m_pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-m_boundingRectangle.topLeft());

	draw(&painter);
}

void HeatmapPrivate::draw(QPainter* painter) {
	auto pen = painter->pen();
	pen.setStyle(Qt::PenStyle::SolidLine);
	pen.setBrush(Qt::NoBrush);

	for (const auto& d : data) {
		painter->setPen(pen);
		painter->fillRect(d.rect, QBrush(d.color));
	}
}

#define DEBUG_BOUNDING_RECT 1

void HeatmapPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (!q->isPrinting() && Settings::group(QStringLiteral("Settings_Worksheet")).readEntry<bool>("DoubleBuffering", true))
		painter->drawPixmap(m_boundingRectangle.topLeft(), m_pixmap); // draw the cached pixmap (fast)
	else
		draw(painter); // draw directly again (slow)

#if DEBUG_BOUNDING_RECT
	painter->setPen(QColor(Qt::GlobalColor::red));
	painter->drawRect(m_boundingRectangle);

	painter->setPen(QColor(Qt::GlobalColor::darkGreen));
	painter->drawRect(m_boundingRectangle.marginsAdded(QMarginsF(3, 3, 3, 3)));

	painter->setPen(QColor(Qt::GlobalColor::blue));
	painter->drawEllipse(QRectF(-5, -5, 10, 10));
#endif

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn); // source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Shadow));
			p.end();

			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
		return;
	}

	if (isSelected() && !q->isPrinting()) {
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Highlight));
			p.end();

			m_selectionEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_selectionEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
	}
}

void HeatmapPrivate::recalcShapeAndBoundingRect(const QRectF& rect) {
	DEBUG(Q_FUNC_INFO << ", suppressRecalc = " << suppressRecalc);
	if (suppressRecalc)
		return;

#ifdef PERFTRACE_CURVES
	PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String(", heatmap ") + name());
#endif

	m_shape = QPainterPath();

	prepareGeometryChange();
	m_boundingRectangle = rect;
	m_shape.addRect(rect); // Currently shape and boundingRectangle are the same

	updatePixmap();
}

// void HeatmapPrivate::recalcColors() {
//	if (automaticLimits) { }
// }

namespace {
namespace XML {
const QLatin1String general("general");
const QLatin1String dataSource("dataSource");
const QLatin1String xColumn("xColumn");
const QLatin1String yColumn("yColumn");
const QLatin1String matrix("matrix");
const QLatin1String equalNumberBins("equalNumberBins");
const QLatin1String xNumberBins("xNumberBins");
const QLatin1String yNumberBins("yNumberBins");
const QLatin1String matrixNumberBins("matrixNumberBins");
const QLatin1String drawEmpty("drawEmpty");
const QLatin1String automaticLimits("automaticLimits");

const QLatin1String format("Format");
const QLatin1String formatName("name");
const QLatin1String formatColors("colors");
const QLatin1String formatMin("min");
const QLatin1String formatMax("max");
} // namespace XML

constexpr QLatin1String configGroupName("Heatmap");
} // anonymous namespace

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Heatmap::save(QXmlStreamWriter* writer) const {
	Q_D(const Heatmap);

	using namespace XML;

	writer->writeStartElement(saveName);
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(XML::general);

	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));

	// if the data columns are valid, write their current paths.
	// if not, write the last used paths so the columns can be restored later
	// when the columns with the same path are added again to the project
	writer->writeAttribute(XML::dataSource, QString::number(static_cast<int>(d->dataSource)));
	if (d->xColumn)
		writer->writeAttribute(XML::xColumn, d->xColumn->path());
	else
		writer->writeAttribute(XML::xColumn, d->xColumnPath);

	if (d->yColumn)
		writer->writeAttribute(XML::yColumn, d->yColumn->path());
	else
		writer->writeAttribute(XML::yColumn, d->yColumnPath);

	if (d->matrix)
		writer->writeAttribute(XML::matrix, d->matrix->path());
	else
		writer->writeAttribute(XML::matrix, d->matrixPath);

	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(XML::equalNumberBins, QString::number(d->equalNumberBins));
	writer->writeAttribute(XML::matrixNumberBins, QString::number(d->matrixNumberBins));
	writer->writeAttribute(XML::automaticLimits, QString::number(d->automaticLimits));
	writer->writeAttribute(XML::xNumberBins, QString::number(d->xNumberBins));
	writer->writeAttribute(XML::yNumberBins, QString::number(d->yNumberBins));
	writer->writeAttribute(XML::drawEmpty, QString::number(d->drawEmpty));

	writer->writeStartElement(XML::format);
	writer->writeAttribute(XML::formatName, d->format.name);
	writer->writeAttribute(XML::formatMin, QString::number(d->format.min));
	writer->writeAttribute(XML::formatMax, QString::number(d->format.max));
	writer->writeStartElement(XML::formatColors);
	for (size_t i = 0; i < d->format.colors.size(); i++) {
		const auto i_str = QStringLiteral("i") + QString::number(i);
		WRITE_QCOLOR3(d->format.colors.at(i), i_str);
	}
	writer->writeEndElement(); // close color section
	writer->writeEndElement(); // close format section

	writer->writeEndElement(); // close general section

	writer->writeEndElement(); // close "heatmap" section
}

#define DEBUG_XML 1

//! Load from XML
bool Heatmap::load(XmlStreamReader* reader, bool preview) {
	Q_D(Heatmap);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		if (DEBUG_XML)
			std::cout << reader->tokenString().toStdString() << "; " << reader->text().toString().toStdString() << "; "
					  << reader->name().toString().toStdString() << std::endl;
		reader->readNext();
		if (reader->isEndElement() && reader->name() == saveName)
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == XML::general) {
			attribs = reader->attributes();

			str = attribs.value(XML::dataSource).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
			else
				d->dataSource = static_cast<DataSource>(str.toInt());

			READ_INT_VALUE(XML::automaticLimits, automaticLimits, bool)
			READ_INT_VALUE(XML::equalNumberBins, equalNumberBins, bool)
			READ_INT_VALUE(XML::matrixNumberBins, matrixNumberBins, bool)
			READ_INT_VALUE(XML::xNumberBins, xNumberBins, unsigned int)
			READ_INT_VALUE(XML::yNumberBins, yNumberBins, unsigned int)
			READ_INT_VALUE(XML::drawEmpty, drawEmpty, bool)

			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);
			READ_MATRIX(matrix);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
			else
				d->setVisible(str.toInt());
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);
		} else if (!preview && reader->name() == XML::format) {
			attribs = reader->attributes();
			READ_STRING_VALUE(XML::formatName, format.name);
			READ_DOUBLE_VALUE(XML::formatMin, format.min);
			READ_DOUBLE_VALUE(XML::formatMax, format.max);
		} else if (!preview && reader->name() == XML::formatColors) {
			attribs = reader->attributes();
			int index = 0;
			bool found = true;
			d->format.colors.clear();
			while (found) {
				QColor color;
				READ_QCOLOR3(color, QStringLiteral("i") + QString::number(index), found);
				if (found)
					d->format.colors.push_back(color);
				index++;
			}
		}
	}

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Heatmap::loadThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group(configGroupName);

	//	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	//	if (!plot)
	//		return;
	//	const int index = plot->curveChildIndex(this);
	//	const QColor themeColor = plot->themeColorPalette(index);

	//	Q_D(XYCurve);
	//	d->suppressRecalc = true;

	//	d->line->loadThemeConfig(group, themeColor);
	//	d->dropLine->loadThemeConfig(group, themeColor);
	//	d->symbol->loadThemeConfig(group, themeColor);
	//	d->background->loadThemeConfig(group);
	//	d->errorBarsLine->loadThemeConfig(group, themeColor);

	//	// Values
	//	this->setValuesOpacity(group.readEntry("ValuesOpacity", 1.0));
	//	this->setValuesColor(group.readEntry("ValuesColor", themeColor));

	//	// margins
	//	if (plot->theme() == QLatin1String("Tufte")) {
	//		if (d->xColumn && d->xColumn->rowCount() < 100) {
	//			setRugEnabled(true);
	//			setRugOrientation(WorksheetElement::Orientation::Both);
	//		}
	//	} else
	//		setRugEnabled(false);

	//	d->suppressRecalc = false;
	//	d->recalcShapeAndBoundingRect();
}

void Heatmap::saveThemeConfig(const KConfig& config) {
	//	KConfigGroup group = config.group(configGroupName);
	//	Q_D(const Heatmap);

	//	d->line->saveThemeConfig(group);
	//	d->dropLine->saveThemeConfig(group);
	//	d->background->saveThemeConfig(group);
	//	d->symbol->saveThemeConfig(group);
	//	d->errorBarsLine->saveThemeConfig(group);

	//	// Values
	//	group.writeEntry("ValuesOpacity", this->valuesOpacity());
	//	group.writeEntry("ValuesColor", (QColor)this->valuesColor());
	//	group.writeEntry("ValuesFont", this->valuesFont());

	//	const int index = parentAspect()->indexOfChild<XYCurve>(this);
	//	if (index < 5) {
	//		KConfigGroup themeGroup = config.group("Theme");
	//		for (int i = index; i < 5; i++) {
	//			QString s = QStringLiteral("ThemePaletteColor") + QString::number(i + 1);
	//			themeGroup.writeEntry(s, (QColor)d->line->pen().color());
	//		}
	//	}
}
