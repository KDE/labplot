/*
	File                 : nsl_peak.h
	Project              : LabPlot
	Description          : NSL peak detection and related methods
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_PEAK_H
#define NSL_PEAK_H

#include <cmath>
#include <cstddef>

template<typename T>
std::size_t* nsl_peak_detect(T* data, std::size_t n, std::size_t& np, T height = -INFINITY, std::size_t distance = 0);

/* TODO: more advanced peak detection (CWT, etc.)*/

#endif
