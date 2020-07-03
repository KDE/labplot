/***************************************************************************
    File                 : Range.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : basic data range class

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

#ifndef RANGE_H
#define RANGE_H

#include <QString>

//! Auxiliary class for a data range 
/**
 *	This class represents a data range [left, right] with right >= left.
 *
 *	Only types supporting comparison are supported
 */
template<class T>
class Range {
public:
	Range() : m_left(0), m_right(0) {}
	Range(T left, T right) {
		this->setRange(left, right);
	}
	~Range() = default;
	T left() const { return m_left; }
	T right() const { return m_right; }
	void setLeft(T left) { m_left = left; }	// no check (use first)
	void setRight(T right) { m_right = std::max(m_left, right); }
	void setRange(T left, T right) {
		m_left = left;
		m_right = std::max(left, right);
	}
	T size() const { return m_right - m_left; }
	bool isZero() const { return (m_right == m_left); }
	bool inside(const Range<T>& other) const { return ( m_left <= other.left() && m_right >= other.right() ); }
	bool inside(T value) const { return ( m_left <= value && m_right >= value ); }
	void translate(T offset) { m_left += offset; m_right += offset; }
	bool operator==(const Range<T>& other) const { return ( m_left == other.left() && m_right == other.right() ); }
	Range<T>& operator=(const Range<T>& other) {
		m_left = other.left();
		m_right = other.right();
		return *this;
	}

	//! Return a string in the format '[left, right]'
	QString toString() const {
		return "[" + QString::number(m_left) + ", " + QString::number(m_right) + "]";
	}
	//TODO: touches(), merge(), subtract(), split(), etc. (see Interval)

private:
	T m_left;	// lower limit
	T m_right;	// upper limit
};

#endif

