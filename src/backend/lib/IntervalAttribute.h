/*
    File                 : IntervalAttribute.h
    Project              : LabPlot
    --------------------------------------------------------------------

    SPDX-FileCopyrightText: 2007 Knut Franke (knut.franke@gmx.de)
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)
    Description          : A class representing an interval-based attribute

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERVALATTRIBUTE_H
#define INTERVALATTRIBUTE_H

#include "Interval.h"
#include <QVector>

//! A class representing an interval-based attribute
template<class T> class IntervalAttribute {
public:
	void setValue(const Interval<int>& i, T value) {
		// first: subtract the new interval from all others
		QVector< Interval<int> > temp_list;
		for (int c = 0; c < m_intervals.size(); c++) {
			temp_list = Interval<int>::subtract(m_intervals.at(c), i);
			if (temp_list.isEmpty()) {
				m_intervals.removeAt(c);
				m_values.removeAt(c--);
			} else {
				m_intervals.replace(c, temp_list.at(0));
				if (temp_list.size() > 1) {
					m_intervals.insert(c, temp_list.at(1));
					m_values.insert(c, m_values.at(c));
				}
			}
		}

		// second: try to merge the new interval with an old one
		for (int c = 0; c < m_intervals.size(); c++) {
			if (m_intervals.at(c).touches(i) && m_values.at(c) == value) {
				m_intervals.replace(c, Interval<int>::merge(m_intervals.at(c), i));
				return;
			}
		}
		// if it could not be merged, just append it
		m_intervals.append(i);
		m_values.append(value);
	}

	// overloaded for convenience
	void setValue(int row, T value)
	{
		setValue(Interval<int>(row, row), value);
	}

	T value(int row) const
	{
		for(int c=m_intervals.size()-1; c>=0; c--)
		{
			if(m_intervals.at(c).contains(row))
				return m_values.at(c);
		}
		return T();
	}

	void insertRows(int before, int count)
	{
		QVector< Interval<int> > temp_list;
		// first: split all intervals that contain 'before'
		for(int c=0; c<m_intervals.size(); c++)
		{
			if(m_intervals.at(c).contains(before))
			{
				temp_list = Interval<int>::split(m_intervals.at(c), before);
				m_intervals.replace(c, temp_list.at(0));
				if(temp_list.size()>1)
				{
					m_intervals.insert(c, temp_list.at(1));
					m_values.insert(c, m_values.at(c));
					c++;
				}

			}
		}
		// second: translate all intervals that start at 'before' or later
		for(int c=0; c<m_intervals.size(); c++)
		{
			if(m_intervals.at(c).start() >= before)
				m_intervals[c].translate(count);
		}

	}

	void removeRows(int first, int count)
	{
		QVector< Interval<int> > temp_list;
		Interval<int> i(first, first+count-1);
		// first: remove the relevant rows from all intervals
		for(int c=0; c<m_intervals.size(); c++)
		{
			temp_list = Interval<int>::subtract(m_intervals.at(c), i);
			if(temp_list.isEmpty())
			{
				m_intervals.removeAt(c);
				m_values.removeAt(c--);
			}
			else
			{
				m_intervals.replace(c, temp_list.at(0));
				if(temp_list.size()>1)
				{
					m_intervals.insert(c, temp_list.at(1));
					m_values.insert(c, m_values.at(c));
					c++;
				}
			}
		}
		// second: translate all intervals that start at 'first+count' or later
		for(int c=0; c<m_intervals.size(); c++)
		{
			if(m_intervals.at(c).start() >= first+count)
				m_intervals[c].translate(-count);
		}
		// third: merge as many intervals as possible
		QVector<T> values_copy = m_values;
		QVector< Interval<int> > intervals_copy = m_intervals;
		m_values.clear();
		m_intervals.clear();
		for(int c=0; c<intervals_copy.size(); c++)
		{
			i = intervals_copy.at(c);
			T value = values_copy.at(c);
			for(int cc=0; cc<m_intervals.size(); cc++)
			{
				if( m_intervals.at(cc).touches(i) &&
						m_values.at(cc) == value )
				{
					m_intervals.replace(cc, Interval<int>::merge(m_intervals.at(cc),i));
					return;
				}
			}
			// if it could not be merged, just append it
			m_intervals.append(i);
			m_values.append(value);
		}
	}

	void clear() { m_values.clear(); m_intervals.clear(); }

	QVector< Interval<int> > intervals() const { return m_intervals; }
	QVector<T> values() const { return m_values; }

private:
	QVector<T> m_values;
	QVector< Interval<int> > m_intervals;
};

//! A class representing an interval-based attribute (bool version)
template<> class IntervalAttribute<bool>
{
	public:
		IntervalAttribute<bool>() {}
		IntervalAttribute<bool>(const QVector< Interval<int> >& intervals) : m_intervals(intervals) {}

		void setValue(const Interval<int>& i, bool value=true)
		{
			if(value)
			{
				foreach(const Interval<int>& iv, m_intervals)
					if(iv.contains(i))
						return;

				Interval<int>::mergeIntervalIntoList(&m_intervals, i);
			} else { // unset
				Interval<int>::subtractIntervalFromList(&m_intervals, i);
			}
		}

		void setValue(int row, bool value)
		{
			setValue(Interval<int>(row, row), value);
		}

		bool isSet(int row) const
		{
			foreach(Interval<int> iv, m_intervals)
				if(iv.contains(row))
					return true;
			return false;
		}

		bool isSet(const Interval<int>& i) const
		{
			foreach(Interval<int> iv, m_intervals)
				if(iv.contains(i))
					return true;
			return false;
		}

		void insertRows(int before, int count)
		{
			QVector< Interval<int> > temp_list;
			int c;
			// first: split all intervals that contain 'before'
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).contains(before))
				{
					temp_list = Interval<int>::split(m_intervals.at(c), before);
					m_intervals.replace(c, temp_list.at(0));
					if(temp_list.size()>1)
						m_intervals.insert(c++, temp_list.at(1));

				}
			}
			// second: translate all intervals that start at 'before' or later
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= before)
					m_intervals[c].translate(count);
			}

		}

		void removeRows(int first, int count)
		{
			int c;
			// first: remove the relevant rows from all intervals
			Interval<int>::subtractIntervalFromList(&m_intervals, Interval<int>(first, first+count-1));
			// second: translate all intervals that start at 'first+count' or later
			for(c=0; c<m_intervals.size(); c++)
			{
				if(m_intervals.at(c).start() >= first+count)
					m_intervals[c].translate(-count);
			}
			// third: merge as many intervals as possible
			for(c=m_intervals.size()-1; c>=0; c--)
			{
				Interval<int> iv = m_intervals.takeAt(c);
				int size_before = m_intervals.size();
				Interval<int>::mergeIntervalIntoList(&m_intervals, iv);
				if(size_before == m_intervals.size()) // merge successful
					c--;
			}
		}

		QVector< Interval<int> > intervals() const { return m_intervals; }

		void clear() { m_intervals.clear(); }

	private:
		QVector< Interval<int> > m_intervals;
};

#endif
