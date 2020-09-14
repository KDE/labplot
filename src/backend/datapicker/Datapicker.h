/***************************************************************************
    File                 : Datapicker.h
    Project              : LabPlot
    Description          : Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2017 by Alexander Semke (alexander.semke@web.de)

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

#ifndef DATAPICKER_H
#define DATAPICKER_H

#include "backend/core/AbstractPart.h"

class Spreadsheet;
class DatapickerCurve;
class DatapickerImage;
class DatapickerView;

class QXmlStreamWriter;
class XmlStreamReader;
class Transform;
class QPointF;
class QVector3D;

class Datapicker : public AbstractPart {
	Q_OBJECT

public:
	explicit Datapicker(const QString& name, const bool loading = false);
	~Datapicker() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	DatapickerCurve* activeCurve();
	Spreadsheet* currentSpreadsheet() const;
	DatapickerImage* image() const;

	void setChildSelectedInView(int index, bool selected);
	void setSelectedInView(const bool);
	void addNewPoint(QPointF, AbstractAspect*);

	QVector3D mapSceneToLogical(QPointF) const;
	QVector3D mapSceneLengthToLogical(QPointF) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public slots:
	void childSelected(const AbstractAspect*) override;

private:
	mutable DatapickerView* m_view{nullptr};
	DatapickerCurve* m_activeCurve{nullptr};
	Transform* m_transform;
	DatapickerImage* m_image{nullptr};
	void init();
	void handleChildAspectAboutToBeRemoved(const AbstractAspect*);
	void handleChildAspectAdded(const AbstractAspect*);

private slots:
	void childDeselected(const AbstractAspect*) override;
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);

signals:
	void datapickerItemSelected(int);
	void requestUpdateActions();
};

#endif
