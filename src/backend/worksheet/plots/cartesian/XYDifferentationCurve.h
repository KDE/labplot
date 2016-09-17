/***************************************************************************
    File                 : XYDifferentationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an differentation
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef XYDIFFERENTATIONCURVE_H
#define XYDIFFERENTATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYDifferentationCurvePrivate;
class XYDifferentationCurve: public XYCurve {
	Q_OBJECT

	public:
		struct DifferentationData {
			DifferentationData() : type(0) {};

			int type;			// type of differentation
		};
		struct DifferentationResult {
			DifferentationResult() : available(false), valid(false), elapsedTime(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
		};

		explicit XYDifferentationCurve(const QString& name);
		virtual ~XYDifferentationCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(DifferentationData, differentationData, DifferentationData)
		const DifferentationResult& differentationResult() const;
		bool isSourceDataChangedSinceLastDifferentation() const;

		typedef WorksheetElement BaseClass;
		typedef XYDifferentationCurvePrivate Private;

	protected:
		XYDifferentationCurve(const QString& name, XYDifferentationCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYDifferentationCurve)
		void init();

	private slots:
		void handleSourceDataChanged();

	signals:
		friend class XYDifferentationCurveSetXDataColumnCmd;
		friend class XYDifferentationCurveSetYDataColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYDifferentationCurveSetDifferentationDataCmd;
		void differentationDataChanged(const XYDifferentationCurve::DifferentationData&);
		void sourceDataChangedSinceLastDifferentation();
};

#endif
