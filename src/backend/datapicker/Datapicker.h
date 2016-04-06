/***************************************************************************
    File                 : Datapicker.h
    Project              : LabPlot
    Description          : Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

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
#include "backend/core/AbstractScriptingEngine.h"

class Spreadsheet;
class DatapickerCurve;
class DatapickerImage;
class QXmlStreamWriter;
class XmlStreamReader;
class Transform;
class QPointF;
class QVector3D;

class Datapicker : public AbstractPart, public scripted {
	Q_OBJECT

	public:
		explicit Datapicker(AbstractScriptingEngine* engine, const QString& name, const bool loading = false);
		virtual ~Datapicker();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;

        virtual bool exportView() const;
        virtual bool printView();
		virtual void printPreview() const;

		DatapickerCurve* activeCurve();
		Spreadsheet* currentSpreadsheet() const;
		DatapickerImage* image() const;

		void setChildSelectedInView(int index, bool selected);
		void setSelectedInView(const bool);
		void addNewPoint(const QPointF&, AbstractAspect*);

		QVector3D mapSceneToLogical(const QPointF&) const;
		QVector3D mapSceneLengthToLogical(const QPointF&) const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

	public slots:
		virtual void childSelected(const AbstractAspect*);

	private:
		DatapickerCurve* m_activeCurve;
		Transform* m_transform;
		DatapickerImage* m_image;
		void init();
		void handleChildAspectAboutToBeRemoved(const AbstractAspect*);
		void handleChildAspectAdded(const AbstractAspect*);

	private slots:
		virtual void childDeselected(const AbstractAspect*);
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);

	signals:
		void datapickerItemSelected(int);
		void requestUpdateActions();
};

#endif
