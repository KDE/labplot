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

template<typename T>
size_t* nsl_peak_detect(T* data, size_t n, size_t* np, T height = -INFINITY, size_t distance = 0);

/* TODO: more advanced peak detection (CWT, etc.)*/

#endif
