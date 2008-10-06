/***************************************************************************
    File                 : MyParser.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Parser class based on muParser
                           
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
#include "MyParser.h"

MyParser::MyParser()
:Parser()
{
DefineConst("pi", M_PI);
DefineConst("Pi", M_PI);
DefineConst("PI", M_PI);

DefineFun("bessel_j0", bessel_J0);
DefineFun("bessel_j1", bessel_J1);
DefineFun("bessel_jn", bessel_Jn);
DefineFun("bessel_y0", bessel_Y0);
DefineFun("bessel_y1", bessel_Y1);
DefineFun("bessel_yn", bessel_Yn);
DefineFun("beta", beta);
DefineFun("erf", erf);
DefineFun("erfc", erfc);
DefineFun("erfz", erfz);
DefineFun("erfq", erfq);
DefineFun("gamma", gamma);
DefineFun("gammaln", gammaln);
DefineFun("hazard", hazard);
}

QStringList MyParser::functionsList()
{
QStringList l;
l << "abs()" << "acos()" << "acosh()" << "asin()" << "asinh()" << "atan()";
l << "atanh()" << "avg(,)" << "bessel_j0()" << "bessel_j1()" << "bessel_jn(,)";
l << "bessel_y0()" << "bessel_y1()" << "bessel_yn(,)" << "beta(,)";
l << "cos()" << "cosh()" << "erf()" << "erfc()" << "erfz()" << "erfq()";
l << "exp()" << "gamma()" << "gammaln()" << "hazard()";
l << "if( , , )" << "ln()" << "log()" << "log2()" << "min()" << "max()";
l << "rint()" << "sign()" << "sin()" << "sinh()" << "sqrt()" << "tan()" << "tanh()";
return l;
}

QString MyParser::explainFunction(int index)
{
QString blabla;
switch (index)
	{
case 0:
	blabla="abs(x):\n Absolute value of x.";
break;

case 1:
	blabla="acos(x):\n Inverse cos function.";
break;

case 2:
	blabla="acosh(x):\n Hyperbolic inverse cos function.";
break;

case 3:
	blabla="asin(x):\n Inverse sin function.";
break;

case 4:
	blabla="asinh(x):\n Hyperbolic inverse sin function.";
break;
case 5:
	blabla="atan(x):\n Inverse tan function.";
break;
case 6:
	blabla="atanh(x):\n  Hyperbolic inverse tan function.";
break;
case 7:
	blabla="avg(x,y,...):\n  Mean value of all arguments.";
break;
case 8:
	blabla="bessel_j0(x):\n  Regular cylindrical Bessel function of zeroth order, J_0(x).";
break;
case 9:
	blabla="bessel_j1(x):\n  Regular cylindrical Bessel function of first order, J_1(x).";
break;
case 10:
	blabla="bessel_j1(double x, int n):\n Regular cylindrical Bessel function of order n, J_n(x).";
break;
case 11:
	blabla="bessel_y0(x):\n Irregular cylindrical Bessel function of zeroth order, Y_0(x), for x>0.";
break;
case 12:
	blabla="bessel_y1(x):\n Irregular cylindrical Bessel function of first order, Y_1(x), for x>0.";
break;
case 13:
	blabla="bessel_yn(double x, int n):\n Irregular cylindrical Bessel function of order n, Y_n(x), for x>0.";
break;
case 14:
	blabla="beta (a,b):\n Computes the Beta Function, B(a,b) = Gamma(a)*Gamma(b)/Gamma(a+b) for a > 0, b > 0.";
break;
case 15:
	blabla="cos (x):\n Calculate cosine.";
break;
case 16:
	blabla="cosh(x):\n Hyperbolic cos function.";
break;
case 17:
	blabla="erf(x):\n  The error function.";
break;
case 18:
	blabla="erfc(x):\n Complementary error function erfc(x) = 1 - erf(x).";
break;
case 19:
	blabla="erfz(x):\n The Gaussian probability density function Z(x).";
break;
case 20:
	blabla="erfq(x):\n The upper tail of the Gaussian probability function Q(x).";
break;
case 21:
	blabla="exp(x):\n Exponential function: e raised to the power of x.";
break;
case 22:
	blabla="gamma(x):\n Computes the Gamma function, subject to x not being a negative integer.";
break;
case 23:
	blabla="gammaln(x):\n Computes the logarithm of the Gamma function, subject to x not a being negative integer. For x<0, log(|Gamma(x)|) is returned.";
break;
case 24:
	blabla="hazard(x):\n Computes the hazard function for the normal distribution h(x) = erfz(x)/erfq(x).";
break;
case 25:
	blabla="if(e1, e2, e3):	if e1 then e2 else e3.";
break;
case 26:
	blabla="ln(x):\n Calculate natural logarithm.";
break;
case 27:
	blabla="log(x):\n Calculate decimal logarithm.";
break;
case 28:
	blabla="log2(x):\n Calculate 	logarithm to the base 2.";
break;
case 29:
	blabla="min(x,y,...):\n Calculate minimum of all arguments.";
break;
case 30:
	blabla="max(x,y,...):\n Calculate maximum of all arguments.";
break;
case 31:
	blabla="rint(x):\n Round to nearest integer.";
break;
case 32:
	blabla="sign(x):\n Sign function: -1 if x<0; 1 if x>0.";
break;
case 33:
	blabla="sin(x):\n Calculate sine.";
break;
case 34:
	blabla="sinh(x):\n Hyperbolic sin function.";
break;
case 35:
	blabla="sqrt(x):\n Square root function.";
break;
case 36:
	blabla="tan(x):\n Calculate tangent function.";
break;
case 37:
	blabla="tanh(x):\n Hyperbolic tan function.";
break;
	}
return blabla;
}
