/***************************************************************************
    File                 : Interval.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke@web.de)
    Description          : Auxiliary class for interval based data

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

#ifndef INTERVAL_H
#define INTERVAL_H

#include <QVector>

extern "C" {
#include "backend/nsl/nsl_math.h"
}

template<class T> class Interval;

template<class T> class IntervalBase {
	public:
		IntervalBase() : m_start(-1), m_end(-1) {}
		IntervalBase(T start, T end) {
			m_start = start;
			m_end = end;
		}
		virtual ~IntervalBase() = default;
		T start() const { return m_start; }
		T end() const { return m_end; }
		void setStart(T start) { m_start = start; }
		void setEnd(T end) { m_end = end; }
		bool contains(const Interval<T>& other) const { return ( m_start <= other.start() && m_end >= other.end() ); }
		bool contains(T value) const { return ( m_start <= value && m_end >= value ); }
		bool fuzzyContains(T value) const {
			bool rc1 = nsl_math_definitely_less_than(m_start, value);
			bool rc2 = nsl_math_definitely_greater_than(m_end, value);
			return (rc1 && rc2);
		}
		bool intersects(const Interval<T>& other) const { return ( contains(other.start()) || contains(other.end()) ); }
		//! Return the intersection of two intervals
		/**
		 * This function returns an invalid interval if the two intervals do not intersect.
		 */
		static Interval<T> intersection(const Interval<T>& first, const Interval<T>& second)
		{
			return Interval<T>( qMax(first.start(), second.start()), qMin(first.end(), second.end()) );
		}
		void translate(T offset) { m_start += offset; m_end += offset; }
		bool operator==(const Interval<T>& other) const { return ( m_start == other.start() && m_end == other.end() ); }
		Interval<T>& operator=(const Interval<T>& other) {
			m_start = other.start();
			m_end = other.end();
			return *this;
		}
		//! Returns true if no gap is between two intervals.
		virtual bool touches(const Interval<T>& other) const = 0;
		//! Merge two intervals that touch or intersect
		static Interval<T> merge(const Interval<T>& a, const Interval<T>& b) {
			if( !(a.intersects(b) || a.touches(b)) )
				return a;
			return Interval<T>( qMin(a.start(), b.start()), qMax(a.end(), b.end()) );
		}
		//! Subtract an interval from another
		static QVector< Interval<T> > subtract(const Interval<T>& src_iv, const Interval<T>& minus_iv) {
			QVector< Interval<T> > list;
			if( (src_iv == minus_iv) || (minus_iv.contains(src_iv)) )
				return list;

			if( !src_iv.intersects(minus_iv) )
				list.append(src_iv);
			else if( src_iv.end() <= minus_iv.end() )
				list.append( Interval<T>(src_iv.start(), minus_iv.start()-1) );
			else if( src_iv.start() >= minus_iv.start() )
				list.append( Interval<T>(minus_iv.end()+1, src_iv.end()) );
			else {
				list.append( Interval<T>(src_iv.start(), minus_iv.start()-1) );
				list.append( Interval<T>(minus_iv.end()+1, src_iv.end()) );
			}

			return list;
		}
		//! Split an interval into two
		static QVector< Interval<T> > split(const Interval<T>& i, T before) {
			QVector< Interval<T> > list;
			if( before < i.start() || before > i.end() )
			{
				list.append(i);
			}
			else
			{
				Interval<T> left(i.start(), before-1);
				Interval<T> right(before, i.end());
				if(left.isValid())
					list.append(left);
				if(right.isValid())
					list.append(right);
			}
			return list;
		}
		//! Merge an interval into a list
		/*
		 * This function merges all intervals in the list until none of them
		 * intersect or touch anymore.
		 */
		static void mergeIntervalIntoList(QVector< Interval<T> > * list, Interval<T> i) {
			for (int c = 0; c < list->size(); c++) {
				if ( list->at(c).touches(i) || list->at(c).intersects(i) ) {
					Interval<T> result = merge(list->takeAt(c), i);
					mergeIntervalIntoList(list, result);
					return;
				}
			}
			list->append(i);
		}
		//! Restrict all intervals in the list to their intersection with a given interval
		/**
		 * Remark: This may decrease the list size.
		 */
		static void restrictList(QVector< Interval<T> > * list, Interval<T> i)
		{
			Interval<T> temp;
			for(int c=0; c<list->size(); c++)
			{
				temp = intersection(list->at(c), i);
				if(!temp.isValid())
					list->removeAt(c--);
				else
					list->replace(c, temp);
			}

		}
		//! Subtract an interval from all intervals in the list
		/**
		 * Remark: This may increase or decrease the list size.
		 */
		static void subtractIntervalFromList(QVector< Interval<T> > * list, Interval<T> i) {
			QVector< Interval<T> > temp_list;
			for(int c=0; c<list->size(); c++)
			{
				temp_list = subtract(list->at(c), i);
				if(temp_list.isEmpty())
					list->removeAt(c--);
				else
				{
					list->replace(c, temp_list.at(0));
					if(temp_list.size()>1)
						list->insert(c, temp_list.at(1));
				}
			}
		}
		QVector< Interval<T> > operator-(QVector< Interval<T> > subtrahend) {
			QVector< Interval<T> > *tmp1, *tmp2;
			tmp1 = new QVector< Interval<T> >();
			*tmp1 << *static_cast< Interval<T>* >(this);
			foreach(Interval<T> i, subtrahend) {
				tmp2 = new QVector< Interval<T> >();
				foreach(Interval<T> j, *tmp1)
					*tmp2 << subtract(j, i);
				delete tmp1;
				tmp1 = tmp2;
			}
			QVector< Interval<T> > result = *tmp1;
			delete tmp1;
			return result;
		}

		//! Return a string in the format '[start,end]'
		QString toString() const {
			return "[" + QString::number(m_start) + "," + QString::number(m_end) + "]";
		}

	protected:
		//! Interval start
		T m_start;
		//! Interval end
		T m_end;
};

//! Auxiliary class for interval based data
/**
 *	This class represents a data range of
 *	the type [start,end] where start > end is possible.
 *
 *	For the template argument (T), only numerical types ((unsigned) short, (unsigned) int,
 *	(unsigned) long, float, double, long double) are supported.
 */
template<class T> class Interval : public IntervalBase<T> {
	public:
		Interval() = default;
		Interval(T start, T end) : IntervalBase<T>(start, end) {}
		Interval(const Interval<T>&) = default;
		T size() const {	// why "+1"?
			return IntervalBase<T>::m_end - IntervalBase<T>::m_start + 1;
		}
		bool isValid() const {	// why >=0 ?
			return ( IntervalBase<T>::m_start >= 0 && IntervalBase<T>::m_end >= 0 &&
					IntervalBase<T>::m_start <= IntervalBase<T>::m_end );
		}
		bool touches(const Interval<T>& other) const override {	// why "+/- 1" ?
			return ( (other.end() == IntervalBase<T>::m_start - 1) ||
					(other.start() == IntervalBase<T>::m_end + 1) );
		}
};

template<> class Interval<float> : public IntervalBase<float> {
	public:
		Interval() {}
		Interval(float start, float end) : IntervalBase<float>(start, end) {}
		Interval(const Interval<float>& other) = default;
		float size() const { return IntervalBase<float>::m_end - IntervalBase<float>::m_start; }
		bool isValid() const { return ( IntervalBase<float>::m_start <= IntervalBase<float>::m_end ); }
		bool touches(const Interval<float>& other) const override {
			return ( (other.end() == IntervalBase<float>::m_start) ||
					(other.start() == IntervalBase<float>::m_end) );
		}
};

template<> class Interval<double> : public IntervalBase<double> {
	public:
		Interval() {}
		Interval(double start, double end) : IntervalBase<double>(start, end) {}
		Interval(const Interval<double>&) = default;
		double size() const { return IntervalBase<double>::m_end - IntervalBase<double>::m_start; }
		bool isValid() const { return ( IntervalBase<double>::m_start <= IntervalBase<double>::m_end ); }
		bool touches(const Interval<double>& other) const override {
			return ( (other.end() == IntervalBase<double>::m_start) ||
					(other.start() == IntervalBase<double>::m_end) );
		}
};

template<> class Interval<long double> : public IntervalBase<long double> {
	public:
		Interval() {}
		Interval(long double start, long double end) : IntervalBase<long double>(start, end) {}
		Interval(const Interval<long double>& other) = default;
		long double size() const { return IntervalBase<long double>::m_end - IntervalBase<long double>::m_start; }
		bool isValid() const { return ( IntervalBase<long double>::m_start <= IntervalBase<long double>::m_end ); }
		bool touches(const Interval<long double>& other) const override {
			return ( (other.end() == IntervalBase<long double>::m_start) ||
					(other.start() == IntervalBase<long double>::m_end) );
		}
};

#endif

