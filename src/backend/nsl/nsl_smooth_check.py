#!/usr/bin/env python
###########################################################################
#    File                 : nsl_smooth_check.h
#    Project              : LabPlot
#    Description          : NSL smooth functions
#    --------------------------------------------------------------------
#    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)
#
###########################################################################

###########################################################################
#                                                                         #
#  This program is free software; you can redistribute it and/or modify   #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation; either version 2 of the License, or      #
#  (at your option) any later version.                                    #
#                                                                         #
#  This program is distributed in the hope that it will be useful,        #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          #
#  GNU General Public License for more details.                           #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the Free Software           #
#   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    #
#   Boston, MA  02110-1301  USA                                           #
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
