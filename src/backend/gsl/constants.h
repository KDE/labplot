/***************************************************************************
    File                 : constans.h
    Project              : LabPlot
    Description          : definition of mathematical and physical constants
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef GSL_CONSTANTS_H
#define GSL_CONSTANTS_H

#include <gsl/gsl_math.h>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>

#include "parser.h"

/* sync with ExpressionParser.cpp */
struct con _constants[] = {
	/* Mathematical constants */
	{"e", M_E},
	{"pi", M_PI},
	{"euler", M_EULER},

	/* Physical constants: http://www.gnu.org/software/gsl/manual/html_node/Physical-Constants.html */
	/* Physical constants in MKSA system */

	/* Fundamental Constants */
	{"cL", GSL_CONST_MKSA_SPEED_OF_LIGHT},
	{"mu0", GSL_CONST_MKSA_VACUUM_PERMEABILITY},
	{"e0", GSL_CONST_MKSA_VACUUM_PERMITTIVITY},
	{"hPlanck", GSL_CONST_MKSA_PLANCKS_CONSTANT_H},
	{"hbar", GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR},
	{"NA", GSL_CONST_NUM_AVOGADRO},
	{"Faraday", GSL_CONST_MKSA_FARADAY},
	{"kB", GSL_CONST_MKSA_BOLTZMANN},
	{"r0", GSL_CONST_MKSA_MOLAR_GAS},
	{"v0", GSL_CONST_MKSA_STANDARD_GAS_VOLUME},
	{"sigma", GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT},
	{"Gauss", GSL_CONST_MKSA_GAUSS},

	/* Astronomy and Astrophysics */
	{"au", GSL_CONST_MKSA_ASTRONOMICAL_UNIT},
	{"G", GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT},
	{"ly", GSL_CONST_MKSA_LIGHT_YEAR},
	{"pc", GSL_CONST_MKSA_PARSEC},
	{"gg", GSL_CONST_MKSA_GRAV_ACCEL},
	{"ms", GSL_CONST_MKSA_SOLAR_MASS},

	/* Atomic and Nuclear Physics */
	{"ee", GSL_CONST_MKSA_ELECTRON_CHARGE},
	{"ev", GSL_CONST_MKSA_ELECTRON_VOLT},
	{"amu", GSL_CONST_MKSA_UNIFIED_ATOMIC_MASS},
	{"me", GSL_CONST_MKSA_MASS_ELECTRON},
	{"mmu", GSL_CONST_MKSA_MASS_MUON},
	{"mp", GSL_CONST_MKSA_MASS_PROTON},
	{"mn", GSL_CONST_MKSA_MASS_NEUTRON},
	{"alpha", GSL_CONST_NUM_FINE_STRUCTURE},
	{"Ry", GSL_CONST_MKSA_RYDBERG},
	{"aB", GSL_CONST_MKSA_BOHR_RADIUS},
	{"ao", GSL_CONST_MKSA_ANGSTROM},
	{"barn", GSL_CONST_MKSA_BARN},
	{"muB", GSL_CONST_MKSA_BOHR_MAGNETON},
	{"mun", GSL_CONST_MKSA_NUCLEAR_MAGNETON},
	{"mue", GSL_CONST_MKSA_ELECTRON_MAGNETIC_MOMENT},
	{"mup", GSL_CONST_MKSA_PROTON_MAGNETIC_MOMENT},
	{"sigmaT", GSL_CONST_MKSA_THOMSON_CROSS_SECTION},
	{"pD", GSL_CONST_MKSA_DEBYE},

	/* Measurement of Time */
	{"min", GSL_CONST_MKSA_MINUTE},
	{"hour", GSL_CONST_MKSA_HOUR},
	{"day", GSL_CONST_MKSA_DAY},
	{"week", GSL_CONST_MKSA_WEEK},

	/* Imperial Units */
	{"in", GSL_CONST_MKSA_INCH},
	{"ft", GSL_CONST_MKSA_FOOT},
	{"yard", GSL_CONST_MKSA_YARD},
	{"mile", GSL_CONST_MKSA_MILE},
	{"mil", GSL_CONST_MKSA_MIL},

	/* Speed and Nautical Units */
	{"v_km_per_h", GSL_CONST_MKSA_KILOMETERS_PER_HOUR},
	{"v_mile_per_h", GSL_CONST_MKSA_MILES_PER_HOUR},
	{"nmile", GSL_CONST_MKSA_NAUTICAL_MILE},
	{"fathom", GSL_CONST_MKSA_FATHOM},
	{"knot", GSL_CONST_MKSA_KNOT},

	/* Printers Units */
	{"pt", GSL_CONST_MKSA_POINT},
	{"texpt", GSL_CONST_MKSA_TEXPOINT},

	/* Volume, Area and Length */
	{"micron", GSL_CONST_MKSA_MICRON},
	{"hectare", GSL_CONST_MKSA_HECTARE},
	{"acre", GSL_CONST_MKSA_ACRE},
	{"liter", GSL_CONST_MKSA_LITER},
	{"us_gallon", GSL_CONST_MKSA_US_GALLON},
	{"can_gallon", GSL_CONST_MKSA_CANADIAN_GALLON},
	{"uk_gallon", GSL_CONST_MKSA_UK_GALLON},
	{"quart", GSL_CONST_MKSA_QUART},
	{"pint", GSL_CONST_MKSA_PINT},

	/* Mass and Weight */
	{"pound", GSL_CONST_MKSA_POUND_MASS},
	{"ounce", GSL_CONST_MKSA_OUNCE_MASS},
	{"ton", GSL_CONST_MKSA_TON},
	{"mton", GSL_CONST_MKSA_METRIC_TON},
	{"uk_ton", GSL_CONST_MKSA_UK_TON},
	{"troy_ounce", GSL_CONST_MKSA_TROY_OUNCE},
	{"carat", GSL_CONST_MKSA_CARAT},
	{"gram_force", GSL_CONST_MKSA_GRAM_FORCE},
	{"pound_force", GSL_CONST_MKSA_POUND_FORCE},
	{"kilepound_force", GSL_CONST_MKSA_KILOPOUND_FORCE},
	{"poundal", GSL_CONST_MKSA_POUNDAL},

	/* Thermal Energy and Power */
	{"cal", GSL_CONST_MKSA_CALORIE},
	{"btu", GSL_CONST_MKSA_BTU},
	{"therm", GSL_CONST_MKSA_THERM},
	{"hp", GSL_CONST_MKSA_HORSEPOWER},

	/* Pressure */
	{"bar", GSL_CONST_MKSA_BAR},
	{"atm", GSL_CONST_MKSA_STD_ATMOSPHERE},
	{"torr", GSL_CONST_MKSA_TORR},
	{"mhg", GSL_CONST_MKSA_METER_OF_MERCURY},
	{"inhg", GSL_CONST_MKSA_INCH_OF_MERCURY},
	{"inh2o", GSL_CONST_MKSA_INCH_OF_WATER},
	{"psi", GSL_CONST_MKSA_PSI},

	/* Viscosity */
	{"poise", GSL_CONST_MKSA_POISE},
	{"stokes", GSL_CONST_MKSA_STOKES},

	/* Light and Illumination */
	{"stilb", GSL_CONST_MKSA_STILB},
	{"lumen", GSL_CONST_MKSA_LUMEN},
	{"lux", GSL_CONST_MKSA_LUX},
	{"phot", GSL_CONST_MKSA_PHOT},
	{"ftcandle", GSL_CONST_MKSA_FOOTCANDLE},
	{"lambert", GSL_CONST_MKSA_LAMBERT},
	{"ftlambert", GSL_CONST_MKSA_FOOTLAMBERT},

	/* Radioactivity */
	{"Curie", GSL_CONST_MKSA_CURIE},
	{"Roentgen", GSL_CONST_MKSA_ROENTGEN},
	{"rad", GSL_CONST_MKSA_RAD},

	/* Force and Energy */
	{"Newton", GSL_CONST_MKSA_NEWTON},
	{"dyne", GSL_CONST_MKSA_DYNE},
	{"Joule", GSL_CONST_MKSA_JOULE},
	{"erg", GSL_CONST_MKSA_ERG},

	/* ignore '...' */
	{"...", 0},
	{0, 0}
};

#endif /* CONSTANTS_H */
