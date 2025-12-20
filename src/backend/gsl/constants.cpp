#include "constants.h"

#include <cstdlib>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>
#include <gsl/gsl_math.h>

#include <KLocalizedString>

namespace Parsing {

QString constantGroupsToString(ConstantGroups group) {
	switch (group) {
	case ConstantGroups::ProgrammingConstants:
		return i18n("Programming constants");
	case ConstantGroups::MathematicalConstants:
		return i18n("Mathematical constants");
	case ConstantGroups::FundamentalConstants:
		return i18n("Fundamental constants");
	case ConstantGroups::AstronomyAndAstrophysics:
		return i18n("Astronomy and Astrophysics");
	case ConstantGroups::AtomicAndNuclearPhysics:
		return i18n("Atomic and Nuclear Physics");
	case ConstantGroups::MeasurementOfTime:
		return i18n("Measurement of Time");
	case ConstantGroups::ImperialUnits:
		return i18n("Imperial Units");
	case ConstantGroups::SpeedAndNauticalUnits:
		return i18n("Speed and Nautical Units");
	case ConstantGroups::PrintersUnits:
		return i18n("Printers Units");
	case ConstantGroups::VolumeAreaAndLength:
		return i18n("Volume, Area and Length");
	case ConstantGroups::MassAndWeight:
		return i18n("Mass and Weight");
	case ConstantGroups::ThermalEnergyAndPower:
		return i18n("Thermal Energy and Power");
	case ConstantGroups::Pressure:
		return i18n("Pressure");
	case ConstantGroups::Viscosity:
		return i18n("Viscosity");
	case ConstantGroups::LightAndIllumination:
		return i18n("Light and Illumination");
	case ConstantGroups::Radioactivity:
		return i18n("Radioactivity");
	case ConstantGroups::ForceAndEnergy:
		return i18n("Force and Energy");
	case ConstantGroups::END:
		break;
	}
	return i18n("Unknown Constant");
}

// clang-format off

/* 
 * All constants which are available in the expression parser. The physical constants are in the MKSA system.
 * When adding a new physical constant, check that it is in MKSA system.
 *
 * Most of the physical constants come from GSL: https://www.gnu.org/software/gsl/doc/html/const.html
 */
struct cons _constants[] = {
	{[]() { return i18n("RAND_MAX");}, "RAND_MAX", RAND_MAX, "", ConstantGroups::ProgrammingConstants},

		// MathematicalConstants = addConstantsGroup(i18n("Mathematical constants"));
		{[]() { return i18n("Base of exponentials");}, "e", M_E, "", ConstantGroups::MathematicalConstants},
		{[]() { return i18n("Pi");}, "pi", M_PI, "", ConstantGroups::MathematicalConstants},
		{[]() { return i18n("Euler's constant");}, "euler", M_EULER, "", ConstantGroups::MathematicalConstants},
		{[]() { return i18n("Not a number");}, "nan", std::nan("0"), "", ConstantGroups::MathematicalConstants},

		// FundamentalConstants = addConstantsGroup(i18n("Fundamental constants"));
		{[]() { return i18n("Speed of light");}, "cL", GSL_CONST_MKSA_SPEED_OF_LIGHT, "m / s", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Vacuum permeability");}, "mu0", GSL_CONST_MKSA_VACUUM_PERMEABILITY, "kg m / A^2 s^2", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Vacuum permittivity");}, "e0", GSL_CONST_MKSA_VACUUM_PERMITTIVITY, "A^2 s^4 / kg m^3", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Planck constant");}, "hPlanck", GSL_CONST_MKSA_PLANCKS_CONSTANT_H, "kg m^2 / s", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Reduced Planck constant");}, "hbar", GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR, "kg m^2 / s", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Avogadro constant");}, "NA", GSL_CONST_NUM_AVOGADRO, "1 / mol", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Faraday");}, "Faraday", GSL_CONST_MKSA_FARADAY, "A s / mol", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Boltzmann constant");}, "kB", GSL_CONST_MKSA_BOLTZMANN, "kg m^2 / K s^2", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Molar gas");}, "r0", GSL_CONST_MKSA_MOLAR_GAS, "kg m^2 / K mol s^2", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Standard gas volume");}, "v0", GSL_CONST_MKSA_STANDARD_GAS_VOLUME, "m^3 / mol", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Stefan-Boltzmann constant");}, "sigma", GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT, "kg / K^4 s^3", ConstantGroups::FundamentalConstants},
		{[]() { return i18n("Gauss");}, "Gauss", GSL_CONST_MKSA_GAUSS, "kg / A s^2", ConstantGroups::FundamentalConstants},

		// AstronomyAndAstrophysics = addConstantsGroup(i18n("Astronomy and Astrophysics"));
		{[]() { return i18n("Astronomical unit");}, "au", GSL_CONST_MKSA_ASTRONOMICAL_UNIT, "m", ConstantGroups::AstronomyAndAstrophysics},
		{[]() { return i18n("Gravitational constant");}, "G", GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT, "m^3 / kg s^2", ConstantGroups::AstronomyAndAstrophysics},
		{[]() { return i18n("Light year");}, "ly", GSL_CONST_MKSA_LIGHT_YEAR, "m", ConstantGroups::AstronomyAndAstrophysics},
		{[]() { return i18n("Parsec");}, "pc", GSL_CONST_MKSA_PARSEC, "m", ConstantGroups::AstronomyAndAstrophysics},
		{[]() { return i18n("Gravitational acceleration");}, "gg", GSL_CONST_MKSA_GRAV_ACCEL, "m / s^2", ConstantGroups::AstronomyAndAstrophysics},
		{[]() { return i18n("Solar mass");}, "ms", GSL_CONST_MKSA_SOLAR_MASS, "kg", ConstantGroups::AstronomyAndAstrophysics},

		// AtomicAndNuclearPhysics = addConstantsGroup(i18n("Atomic and Nuclear Physics"));
		{[]() { return i18n("Charge of the electron");}, "ee", GSL_CONST_MKSA_ELECTRON_CHARGE, "A s", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Energy of 1 electron volt");}, "ev", GSL_CONST_MKSA_ELECTRON_VOLT, "kg m^2 / s^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Unified atomic mass");}, "amu", GSL_CONST_MKSA_UNIFIED_ATOMIC_MASS, "kg", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Mass of the electron");}, "me", GSL_CONST_MKSA_MASS_ELECTRON, "kg", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Mass of the muon");}, "mmu", GSL_CONST_MKSA_MASS_MUON, "kg", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Mass of the proton");}, "mp", GSL_CONST_MKSA_MASS_PROTON, "kg", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Mass of the neutron");}, "mn", GSL_CONST_MKSA_MASS_NEUTRON, "kg", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Electromagnetic fine structure constant");}, "alpha", GSL_CONST_NUM_FINE_STRUCTURE, "", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Rydberg constant");}, "Ry", GSL_CONST_MKSA_RYDBERG, "kg m^2 / s^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Bohr radius");}, "aB", GSL_CONST_MKSA_BOHR_RADIUS, "m", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Length of 1 angstrom");}, "ao", GSL_CONST_MKSA_ANGSTROM, "m", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Area of 1 barn");}, "barn", GSL_CONST_MKSA_BARN, "m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Bohr Magneton");}, "muB", GSL_CONST_MKSA_BOHR_MAGNETON, "A m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Nuclear Magneton");}, "mun", GSL_CONST_MKSA_NUCLEAR_MAGNETON, "A m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Magnetic moment of the electron [absolute value]");}, "mue", GSL_CONST_MKSA_ELECTRON_MAGNETIC_MOMENT, "A m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Magnetic moment of the proton");}, "mup", GSL_CONST_MKSA_PROTON_MAGNETIC_MOMENT, "A m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Thomson cross section");}, "sigmaT", GSL_CONST_MKSA_THOMSON_CROSS_SECTION, "m^2", ConstantGroups::AtomicAndNuclearPhysics},
		{[]() { return i18n("Electric dipole moment of 1 Debye");}, "pD", GSL_CONST_MKSA_DEBYE, "A s^2 / m^2", ConstantGroups::AtomicAndNuclearPhysics},

		// MeasurementOfTime = addConstantsGroup(i18n("Measurement of Time"));
		{[]() { return i18n("Number of seconds in 1 minute");}, "minute", GSL_CONST_MKSA_MINUTE, "s", ConstantGroups::MeasurementOfTime},
		{[]() { return i18n("Number of seconds in 1 hour");}, "hour", GSL_CONST_MKSA_HOUR, "s", ConstantGroups::MeasurementOfTime},
		{[]() { return i18n("Number of seconds in 1 day");}, "day", GSL_CONST_MKSA_DAY, "s", ConstantGroups::MeasurementOfTime},
		{[]() { return i18n("Number of seconds in 1 week");}, "week", GSL_CONST_MKSA_WEEK, "s", ConstantGroups::MeasurementOfTime},

		// ImperialUnits = addConstantsGroup(i18n("Imperial Units"));
		{[]() { return i18n("Length of 1 inch");}, "in", GSL_CONST_MKSA_INCH, "m", ConstantGroups::ImperialUnits},
		{[]() { return i18n("Length of 1 foot");}, "ft", GSL_CONST_MKSA_FOOT, "m", ConstantGroups::ImperialUnits},
		{[]() { return i18n("Length of 1 yard");}, "yard", GSL_CONST_MKSA_YARD, "m", ConstantGroups::ImperialUnits},
		{[]() { return i18n("Length of 1 mile");}, "mile", GSL_CONST_MKSA_MILE, "m", ConstantGroups::ImperialUnits},
		{[]() { return i18n("Length of 1/1000th of an inch");}, "mil", GSL_CONST_MKSA_MIL, "m", ConstantGroups::ImperialUnits},

		// SpeedAndNauticalUnits = addConstantsGroup(i18n("Speed and Nautical Units"));
		{[]() { return i18n("Speed of 1 kilometer per hour");}, "v_km_per_h", GSL_CONST_MKSA_KILOMETERS_PER_HOUR, "m / s", ConstantGroups::SpeedAndNauticalUnits},
		{[]() { return i18n("Speed of 1 mile per hour");}, "v_mile_per_h", GSL_CONST_MKSA_MILES_PER_HOUR, "m / s", ConstantGroups::SpeedAndNauticalUnits},
		{[]() { return i18n("Length of 1 nautical mile");}, "nmile", GSL_CONST_MKSA_NAUTICAL_MILE, "m", ConstantGroups::SpeedAndNauticalUnits},
		{[]() { return i18n("Length of 1 fathom");}, "fathom", GSL_CONST_MKSA_FATHOM, "m", ConstantGroups::SpeedAndNauticalUnits},
		{[]() { return i18n("Speed of 1 knot");}, "knot", GSL_CONST_MKSA_KNOT, "m / s", ConstantGroups::SpeedAndNauticalUnits},

		// PrintersUnits = addConstantsGroup(i18n("Printers Units"));
		{[]() { return i18n("length of 1 printer's point [1/72 inch]");}, "pt", GSL_CONST_MKSA_POINT, "m", ConstantGroups::PrintersUnits},
		{[]() { return i18n("length of 1 TeX point [1/72.27 inch]");}, "texpt", GSL_CONST_MKSA_TEXPOINT, "m", ConstantGroups::PrintersUnits},

		// VolumeAreaAndLength = addConstantsGroup(i18n("Volume, Area and Length"));
		{[]() { return i18n("Length of 1 micron");}, "micron", GSL_CONST_MKSA_MICRON, "m", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Area of 1 hectare");}, "hectare", GSL_CONST_MKSA_HECTARE, "m^2", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Area of 1 acre");}, "acre", GSL_CONST_MKSA_ACRE, "m^2", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 liter");}, "liter", GSL_CONST_MKSA_LITER, "m^3", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 US gallon");}, "us_gallon", GSL_CONST_MKSA_US_GALLON, "m^3", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 Canadian gallon");}, "can_gallon", GSL_CONST_MKSA_CANADIAN_GALLON, "m^3", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 UK gallon");}, "uk_gallon", GSL_CONST_MKSA_UK_GALLON, "m^3", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 quart");}, "quart", GSL_CONST_MKSA_QUART, "m^3", ConstantGroups::VolumeAreaAndLength},
		{[]() { return i18n("Volume of 1 pint");}, "pint", GSL_CONST_MKSA_PINT, "m^3", ConstantGroups::VolumeAreaAndLength},

		// MassAndWeight = addConstantsGroup(i18n("Mass and Weight"));
		{[]() { return i18n("Mass of 1 pound");}, "pound", GSL_CONST_MKSA_POUND_MASS, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 ounce");}, "ounce", GSL_CONST_MKSA_OUNCE_MASS, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 ton");}, "ton", GSL_CONST_MKSA_TON, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 metric ton [1000 kg]");}, "mton", GSL_CONST_MKSA_METRIC_TON, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 UK ton");}, "uk_ton", GSL_CONST_MKSA_UK_TON, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 troy ounce");}, "troy_ounce", GSL_CONST_MKSA_TROY_OUNCE, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Mass of 1 carat");}, "carat", GSL_CONST_MKSA_CARAT, "kg", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Force of 1 gram weight");}, "gram_force", GSL_CONST_MKSA_GRAM_FORCE, "kg m / s^2", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Force of 1 pound weight");}, "pound_force", GSL_CONST_MKSA_POUND_FORCE, "kg m / s^2", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Force of 1 kilopound weight");}, "kilepound_force", GSL_CONST_MKSA_KILOPOUND_FORCE, "kg m / s^2", ConstantGroups::MassAndWeight},
		{[]() { return i18n("Force of 1 poundal");}, "poundal", GSL_CONST_MKSA_POUNDAL, "kg m / s^2", ConstantGroups::MassAndWeight},

		// ThermalEnergyAndPower = addConstantsGroup(i18n("Thermal Energy and Power"));
		{[]() { return i18n("Energy of 1 calorie");}, "cal", GSL_CONST_MKSA_CALORIE, "kg m^2 / s^2", ConstantGroups::ThermalEnergyAndPower},
		{[]() { return i18n("Energy of 1 British Thermal Unit");}, "btu", GSL_CONST_MKSA_BTU, "kg m^2 / s^2", ConstantGroups::ThermalEnergyAndPower},
		{[]() { return i18n("Energy of 1 Therm");}, "therm", GSL_CONST_MKSA_THERM, "kg m^2 / s^2", ConstantGroups::ThermalEnergyAndPower},
		{[]() { return i18n("Power of 1 horsepower");}, "hp", GSL_CONST_MKSA_HORSEPOWER, "kg m^2 / s^3", ConstantGroups::ThermalEnergyAndPower},

		// Pressure = addConstantsGroup(i18n("Pressure"));
		{[]() { return i18n("Pressure of 1 bar");}, "bar", GSL_CONST_MKSA_BAR, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 standard atmosphere");}, "atm", GSL_CONST_MKSA_STD_ATMOSPHERE, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 torr");}, "torr", GSL_CONST_MKSA_TORR, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 meter of mercury");}, "mhg", GSL_CONST_MKSA_METER_OF_MERCURY, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 inch of mercury");}, "inhg", GSL_CONST_MKSA_INCH_OF_MERCURY, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 inch of water");}, "inh2o", GSL_CONST_MKSA_INCH_OF_WATER, "kg / m s^2", ConstantGroups::Pressure},
		{[]() { return i18n("Pressure of 1 pound per square inch");}, "psqi", GSL_CONST_MKSA_PSI, "kg / m s^2", ConstantGroups::Pressure},

		// Viscosity = addConstantsGroup(i18n("Viscosity"));
		{[]() { return i18n("Dynamic viscosity of 1 poise");}, "poise", GSL_CONST_MKSA_POISE, "kg / m s", ConstantGroups::Viscosity},
		{[]() { return i18n("Kinematic viscosity of 1 stokes");}, "stokes", GSL_CONST_MKSA_STOKES, "m^2 / s", ConstantGroups::Viscosity},

		// LightAndIllumination = addConstantsGroup(i18n("Light and Illumination"));
		{[]() { return i18n("Luminance of 1 stilb");}, "stilb", GSL_CONST_MKSA_STILB, "cd / m^2", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Luminous flux of 1 lumen");}, "lumen", GSL_CONST_MKSA_LUMEN, "cd sr", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Illuminance of 1 lux");}, "lux", GSL_CONST_MKSA_LUX, "cd sr / m^2", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Illuminance of 1 phot");}, "phot", GSL_CONST_MKSA_PHOT, "cd sr / m^2", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Illuminance of 1 footcandle");}, "ftcandle", GSL_CONST_MKSA_FOOTCANDLE, "cd sr / m^2", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Luminance of 1 lambert");}, "lambert", GSL_CONST_MKSA_LAMBERT, "cd sr / m^2", ConstantGroups::LightAndIllumination},
		{[]() { return i18n("Luminance of 1 footlambert");}, "ftlambert", GSL_CONST_MKSA_FOOTLAMBERT, "cd sr / m^2", ConstantGroups::LightAndIllumination},

		// Radioactivity = addConstantsGroup(i18n("Radioactivity"));
		{[]() { return i18n("Activity of 1 curie");}, "Curie", GSL_CONST_MKSA_CURIE, "1 / s", ConstantGroups::Radioactivity},
		{[]() { return i18n("Exposure of 1 roentgen");}, "Roentgen", GSL_CONST_MKSA_ROENTGEN, "A s / kg", ConstantGroups::Radioactivity},
		{[]() { return i18n("Absorbed dose of 1 rad");}, "rad", GSL_CONST_MKSA_RAD, "m^2 / s^2", ConstantGroups::Radioactivity},

		// ForceAndEnergy = addConstantsGroup(i18n("Force and Energy"));
		{[]() { return i18n("SI unit of force");}, "Newton", GSL_CONST_MKSA_NEWTON, "kg m / s^2", ConstantGroups::ForceAndEnergy},
		{[]() { return i18n("Force of 1 Dyne");}, "dyne", GSL_CONST_MKSA_DYNE, "kg m / s^2", ConstantGroups::ForceAndEnergy},
		{[]() { return i18n("SI unit of energy");}, "Joule", GSL_CONST_MKSA_JOULE, "kg m^2 / s^2", ConstantGroups::ForceAndEnergy},
		{[]() { return i18n("Energy 1 erg");}, "erg", GSL_CONST_MKSA_ERG, "kg m^2 / s^2", ConstantGroups::ForceAndEnergy},
	};

// clang-format on

const int _number_constants = sizeof(_constants) / sizeof(cons);

} // namespace Parsing
