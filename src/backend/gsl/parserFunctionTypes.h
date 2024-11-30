/*
	File                 : parserFunctionTypes.h
	Project              : LabPlot
	Description          : Parser for mathematical expressions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSERFUNCTIONTYPES_H
#define PARSERFUNCTIONTYPES_H

#include <functional>
#include <memory>
#include <string_view>

namespace Parser {

struct Payload;

/* Function types */
using func_t = std::function<double(void)>;
using func_t1 = std::function<double(double)>;
using func_t2 = std::function<double(double, double)>;
using func_t3 = std::function<double(double, double, double)>;
using func_t4 = std::function<double(double, double, double, double)>;
using func_t5 = std::function<double(double, double, double, double, double)>;
using func_tPayload = std::function<double(const std::weak_ptr<Payload>)>;
using func_tValuePayload = std::function<double(double, const std::weak_ptr<Payload>)>;
using func_t2ValuePayload = std::function<double(double, double, const std::weak_ptr<Payload>)>;
using func_tVariablePayload = std::function<double(const std::string_view&, const std::weak_ptr<Payload>)>;
using func_tValueVariablePayload = std::function<double(double, const std::string_view&, const std::weak_ptr<Payload>)>;
using func_t2ValueVariablePayload = std::function<double(double, double, const std::string_view&, const std::weak_ptr<Payload>)>;
using func_t3ValueVariablePayload = std::function<double(double, double, double, const std::string_view&, const std::weak_ptr<Payload>)>;
} // namespace Parser

#endif /*PARSERFUNCTIONTYPES_H*/
