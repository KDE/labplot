/***************************************************************************
    File                 : Fit.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Abstract base class for data analysis operations

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
#include "Filter.h"
#include "lib/ColorBox.h"

#include "graph/TextEnrichment.h"
#include "table/Table.h"
#include "graph/FunctionCurve.h"
#include "graph/PlotCurve.h"
#include "graph/Layer.h"

#include <QApplication>
#include <QMessageBox>
#include <QLocale>

#include <gsl/gsl_sort.h>

Filter::Filter( ApplicationWindow *parent, Layer *g, const char * name)
: QObject( parent, name)
{
	init();
	m_layer = g;
}

Filter::Filter( ApplicationWindow *parent, Table *t, const char * name)
: QObject( parent, name)
{
	init();
	m_table = t;
}

void Filter::init()
{
	m_n = 0;
	m_curveColorIndex = 1;
	m_tolerance = 1e-4;
	m_points = 100;
	m_max_iterations = 1000;
	m_curve = 0;
	m_prec = ((ApplicationWindow *)parent())->fit_output_precision;
	m_init_err = false;
    m_sort_data = false;
    m_min_points = 2;
    m_explanation = QString(name());
    m_layer = 0;
    m_table = 0;
}

void Filter::setInterval(double from, double to)
{
	if (!m_curve)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
				tr("Please assign a curve first!"));
		return;
	}
	setDataFromCurve (m_curve->title().text(), from, to);
}

void Filter::setDataCurve(int curve, double start, double end)
{
	if (m_n > 0)
	{//delete previousely allocated memory
		delete[] m_x;
		delete[] m_y;
	}

	m_init_err = false;
	m_curve = m_layer->curve(curve);
    if (m_sort_data)
        m_n = sortedCurveData(m_curve, start, end, &m_x, &m_y);
    else
    	m_n = curveData(m_curve, start, end, &m_x, &m_y);

	if (m_n == -1)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
				tr("Several data points have the same x value causing divisions by zero, operation aborted!"));
		m_init_err = true;
        return;
	}
    else if (m_n < m_min_points)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
				tr("You need at least %1 points in order to perform this operation!").arg(m_min_points));
		m_init_err = true;
        return;
	}

    m_from = start;
    m_to = end;
}

int Filter::curveIndex(const QString& curveTitle, Layer *g)
{
	if (curveTitle.isEmpty())
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("Filter Error"),
				tr("Please enter a valid curve name!"));
		m_init_err = true;
		return -1;
	}

	if (g)
		m_layer = g;

	if (!m_layer)
	{
		m_init_err = true;
		return -1;
	}

	return m_layer->curveIndex(curveTitle);
}

bool Filter::setDataFromCurve(const QString& curveTitle, Layer *g)
{
	int index = curveIndex(curveTitle, g);
	if (index < 0)
	{
		m_init_err = true;
		return false;
	}

  	m_layer->range(index, &m_from, &m_to);
    setDataCurve(index, m_from, m_to);
	return true;
}

bool Filter::setDataFromCurve(const QString& curveTitle, double from, double to, Layer *g)
{
	int index = curveIndex(curveTitle, g);
	if (index < 0)
	{
		m_init_err = true;
		return false;
	}

	setDataCurve(index, from, to);
	return true;
}

void Filter::setColor(const QString& colorName)
{
    QColor c = QColor(colorName);
    if (colorName == "green")
        c = QColor(Qt::green);
    else if (colorName == "darkYellow")
        c = QColor(Qt::darkYellow);
    if (!ColorBox::isValidColor(c))
    {
        QMessageBox::critical((ApplicationWindow *)parent(), tr("Color Name Error"),
				tr("The color name '%1' is not valid, a default color (red) will be used instead!").arg(colorName));
        m_curveColorIndex = 1;
        return;
    }

	m_curveColorIndex = ColorBox::colorIndex(c);
}

void Filter::showLegend()
{
	TextEnrichment* mrk = m_layer->newLegend(legendInfo());
	if (m_layer->hasLegend())
	{
		TextEnrichment* legend = m_layer->legend();
		QPoint p = legend->rect().bottomLeft();
		mrk->setOrigin(QPoint(p.x(), p.y()+20));
	}
	m_layer->replot();
}

bool Filter::run()
{
	if (m_init_err)
		return false;

	if (m_n < 0)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
				tr("You didn't specify a valid data set for this operation!"));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

    output();//data analysis and output
    ((ApplicationWindow *)parent())->updateLog(logInfo());

	QApplication::restoreOverrideCursor();
    return true;
}

void Filter::output()
{
    double *X = new double[m_points];
    double *Y = new double[m_points];

    //do the data analysis
    calculateOutputData(X, Y);

	addResultCurve(X, Y);
}

int Filter::sortedCurveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
    if (!c)
        return 0;

    int i_start = 0, i_end = c->dataSize();
    for (int i = 0; i < i_end; i++)
  	    if (c->x(i) >= start)
        {
  	      i_start = i;
          break;
        }
    for (int i = i_end-1; i >= 0; i--)
  	    if (c->x(i) <= end)
        {
  	      i_end = i;
          break;
        }
    int n = i_end - i_start + 1;
    (*x) = new double[n];
    (*y) = new double[n];
    double *xtemp = new double[n];
    double *ytemp = new double[n];

  	int j=0;
    for (int i = i_start; i <= i_end; i++)
    {
        xtemp[j] = c->x(i);
        ytemp[j++] = c->y(i);
    }
    size_t *p = new size_t[n];
    gsl_sort_index(p, xtemp, 1, n);
    for (int i=0; i<n; i++)
    {
        (*x)[i] = xtemp[p[i]];
  	    (*y)[i] = ytemp[p[i]];
    }
    delete[] xtemp;
    delete[] ytemp;
    delete[] p;
    return n;
}

int Filter::curveData(QwtPlotCurve *c, double start, double end, double **x, double **y)
{
    if (!c)
        return 0;

    int i_start = 0, i_end = c->dataSize();
    for (int i = 0; i < i_end; i++)
  	    if (c->x(i) >= start)
        {
  	      i_start = i;
          break;
        }
    for (int i = i_end-1; i >= 0; i--)
  	    if (c->x(i) <= end)
        {
  	      i_end = i;
          break;
        }
    int n = i_end - i_start + 1;
    (*x) = new double[n];
    (*y) = new double[n];

    int j=0;
    for (int i = i_start; i <= i_end; i++)
    {
        (*x)[j] = c->x(i);
        (*y)[j++] = c->y(i);
    }
    return n;
}

QwtPlotCurve* Filter::addResultCurve(double *x, double *y)
{
    ApplicationWindow *app = (ApplicationWindow *)parent();
    const QString tableName = app->generateUniqueName(QString(this->name()));
    Table *t = app->newHiddenTable(tableName, m_explanation + " " + tr("of") + " " + m_curve->title().text(), m_points, 2);
	for (int i=0; i<m_points; i++)
	{
		t->setText(i, 0, QLocale().toString(x[i], 'g', app->m_decimal_digits));
		t->setText(i, 1, QLocale().toString(y[i], 'g', app->m_decimal_digits));
	}

	DataCurve *c = new DataCurve(t, tableName + "_1", tableName + "_2");
	c->setData(x, y, m_points);
    c->setPen(QPen(ColorBox::color(m_curveColorIndex), 1));
	m_layer->insertPlotItem(c, Layer::Line);
    m_layer->updatePlot();

    delete[] x;
	delete[] y;
	return (QwtPlotCurve*)c;
}

Filter::~Filter()
{
	if (m_n > 0)
	{//delete the memory allocated for the data
		delete[] m_x;
		delete[] m_y;
	}
}
