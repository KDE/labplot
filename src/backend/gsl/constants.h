/*
	File                 : constants.h
	Project              : LabPlot
	Description          : definition of mathematical and physical constants
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2014-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GSL_CONSTANTS_H
#define GSL_CONSTANTS_H

#include <QString>
#include <functional>

namespace Parser {

enum class ConstantGroups;

struct cons {
	std::function<QString(void)> description;
	const char* name;
	double value;
	const char* unit;
	ConstantGroups group;
};

extern struct cons _constants[];
extern const int _number_constants;

enum class ConstantGroups : int {
	MathematicalConstants,
	FundamentalConstants,
	AstronomyAndAstrophysics,
	AtomicAndNuclearPhysics,
	MeasurementOfTime,
	ImperialUnits,
	SpeedAndNauticalUnits,
	PrintersUnits,
	VolumeAreaAndLength,
	MassAndWeight,
	ThermalEnergyAndPower,
	Pressure,
	Viscosity,
	LightAndIllumination,
	Radioactivity,
	ForceAndEnergy,

	//---------------
	END
};

QString constantGroupsToString(ConstantGroups group);

} // namespace Parser

#endif /* CONSTANTS_H */
