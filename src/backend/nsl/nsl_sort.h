/*
    File                 : nsl_sort.h
    Project              : LabPlot
    Description          : NSL sorting functions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NSL_SORT_H
#define NSL_SORT_H

#include <stdlib.h>

/* compare size_t objects */
int nsl_sort_compare_size_t(const void* a, const void* b); 

/* sort size_t array of size n */
void nsl_sort_size_t(size_t array[], const size_t n);

#endif /* NSL_SORT_H */
