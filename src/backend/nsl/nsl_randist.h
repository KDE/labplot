/*
	File                 : nsl_randist.h
	Project              : LabPlot
	Description          : NSL random number distributions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NSL_RANDIST_H
#define NSL_RANDIST_H

#include <gsl/gsl_rng.h>

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

/* triangular distribution */
double nsl_ran_triangular(gsl_rng* r, double min, double max, double mode);

__END_DECLS

#endif /* NSL_RANDIST_H */
