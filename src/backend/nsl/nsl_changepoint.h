/*
	File                 : nsl_changepoint.h
	Project              : LabPlot
	Description          : NSL change point detection functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_CHANGEPOINT_H
#define NSL_CHANGEPOINT_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS /* empty */
#define __END_DECLS /* empty */
#endif
__BEGIN_DECLS

#include <stdlib.h>

#define NSL_CHANGEPOINT_METHOD_COUNT 2
typedef enum { nsl_changepoint_method_binary_segmentation, nsl_changepoint_method_pelt } nsl_changepoint_method;
extern const char* nsl_changepoint_method_name[];

/* Binary Segmentation: recursive splitting at point of max cost reduction
   x, y - data arrays
   n - data size
   penalty - controls segment count (higher = fewer segments)
   min_segment_size - minimum points per segment
   changepoints - output array of changepoint indices (caller allocates)
   max_changepoints - size of changepoints array
   returns: number of changepoints found */
size_t nsl_changepoint_binary_segmentation(const double x[],
										   const double y[],
										   size_t n,
										   double penalty,
										   size_t min_segment_size,
										   size_t changepoints[],
										   size_t max_changepoints);

/* PELT (Pruned Exact Linear Time): optimal segmentation with pruning
   x, y - data arrays
   n - data size
   penalty - controls segment count
   min_segment_size - minimum points per segment
   changepoints - output array of changepoint indices
   max_changepoints - size of changepoints array
   returns: number of changepoints found */
size_t
nsl_changepoint_pelt(const double x[], const double y[], size_t n, double penalty, size_t min_segment_size, size_t changepoints[], size_t max_changepoints);

__END_DECLS

#endif /* NSL_CHANGEPOINT_H */
