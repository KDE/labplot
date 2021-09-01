#!/usr/bin/env python
###########################################################################
#    File                 : nsl_smooth_check.h
#    Project              : LabPlot
#    Description          : NSL smooth functions
#    --------------------------------------------------------------------
#    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
#
###########################################################################

###########################################################################
#                                                                         #
#  SPDX-License-Identifier: GPL-2.0-or-later
#                                                                         #
###########################################################################

from scipy.signal import savgol_filter
import numpy as np
x = np.array([2, 2, 5, 2, 1, 0, 1, 4, 9])

m=5
order=2
np.set_printoptions(precision=4)
print x
print "mode:interp"
print savgol_filter(x, m, order, mode='interp')
print "mode:mirror"
print savgol_filter(x, m, order, mode='mirror')
print "mode:nearest"
print savgol_filter(x, m, order, mode='nearest')
print "mode:constant"
print savgol_filter(x, m, order, mode='constant')
print "mode:wrap"
print savgol_filter(x, m, order, mode='wrap')
