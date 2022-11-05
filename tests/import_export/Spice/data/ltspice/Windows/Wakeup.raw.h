/*
	File                 : Wakeup.raw.h
	Project              : LabPlot
	Description          : LTSpice reference data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
namespace Wakeup {

const QString filename = QStringLiteral("Windows/Wakeup.raw");

QString refFileInfoString = QLatin1String(R"(Title: * C:\Users\martinm\Documents\GIT\ACPB_BMS\Electronics Design\Wakeup\Wakeup.asc
<br>Date: Wed Mar 23 09:44:38 2022
<br>Plotname: Transient Analysis
<br>Flags: real forward
<br>No. Variables: 54
<br>No. Points:          189
<br>Offset:   0.0000000000000000e+000
<br>Command: Linear Technology Corporation LTspice XVII
<br>Backannotation: u1 1 2
<br>Variables:
<br>	0	time	time
<br>	1	V(n013)	voltage
<br>	2	V(vin)	voltage
<br>	3	V(n001)	voltage
<br>	4	V(n008)	voltage
<br>	5	V(n004)	voltage
<br>	6	V(n018)	voltage
<br>	7	V(p001)	voltage
<br>	8	V(n003)	voltage
<br>	9	V(n012)	voltage
<br>	10	V(n015)	voltage
<br>	11	V(n002)	voltage
<br>	12	V(n009)	voltage
<br>	13	V(n020)	voltage
<br>	14	V(n023)	voltage
<br>	15	V(n021)	voltage
<br>	16	V(n022)	voltage
<br>	17	V(n019)	voltage
<br>	18	V(n005)	voltage
<br>	19	V(n014)	voltage
<br>	20	V(n016)	voltage
<br>	21	V(n006)	voltage
<br>	22	V(n010)	voltage
<br>	23	V(n017)	voltage
<br>	24	V(n007)	voltage
<br>	25	V(n011)	voltage
<br>	26	I(C1)	device_current
<br>	27	I(D2)	device_current
<br>	28	I(R1)	device_current
<br>	29	I(R8)	device_current
<br>	30	I(R9)	device_current
<br>	31	I(R4)	device_current
<br>	32	I(S7)	device_current
<br>	33	I(S6)	device_current
<br>	34	I(S5)	device_current
<br>	35	I(S4)	device_current
<br>	36	I(S2)	device_current
<br>	37	I(S3)	device_current
<br>	38	I(S1)	device_current
<br>	39	I(V13)	device_current
<br>	40	I(V12)	device_current
<br>	41	I(V11)	device_current
<br>	42	I(V10)	device_current
<br>	43	I(V9)	device_current
<br>	44	I(V1)	device_current
<br>	45	I(V8)	device_current
<br>	46	I(V7)	device_current
<br>	47	I(V6)	device_current
<br>	48	I(V4)	device_current
<br>	49	I(V5)	device_current
<br>	50	I(V3)	device_current
<br>	51	I(V2)	device_current
<br>	52	Ix(u1:1)	subckt_current
<br>	53	Ix(u1:2)	subckt_current
)"); // last \n is important

const int refDataRowCount = 189;
const int numberPreviewData = 103;

QStringList columnNames = {QStringLiteral("time, time"),
						   QStringLiteral("V(n013), voltage"),
						   QStringLiteral("V(vin), voltage"),
						   QStringLiteral("V(n001), voltage"),
						   QStringLiteral("V(n008), voltage"),
						   QStringLiteral("V(n004), voltage"),
						   QStringLiteral("V(n018), voltage"),
						   QStringLiteral("V(p001), voltage"),
						   QStringLiteral("V(n003), voltage"),
						   QStringLiteral("V(n012), voltage"),
						   QStringLiteral("V(n015), voltage"),
						   QStringLiteral("V(n002), voltage"),
						   QStringLiteral("V(n009), voltage"),
						   QStringLiteral("V(n020), voltage"),
						   QStringLiteral("V(n023), voltage"),
						   QStringLiteral("V(n021), voltage"),
						   QStringLiteral("V(n022), voltage"),
						   QStringLiteral("V(n019), voltage"),
						   QStringLiteral("V(n005), voltage"),
						   QStringLiteral("V(n014), voltage"),
						   QStringLiteral("V(n016), voltage"),
						   QStringLiteral("V(n006), voltage"),
						   QStringLiteral("V(n010), voltage"),
						   QStringLiteral("V(n017), voltage"),
						   QStringLiteral("V(n007), voltage"),
						   QStringLiteral("V(n011), voltage"),
						   QStringLiteral("I(C1), device_current"),
						   QStringLiteral("I(D2), device_current"),
						   QStringLiteral("I(R1), device_current"),
						   QStringLiteral("I(R8), device_current"),
						   QStringLiteral("I(R9), device_current"),
						   QStringLiteral("I(R4), device_current"),
						   QStringLiteral("I(S7), device_current"),
						   QStringLiteral("I(S6), device_current"),
						   QStringLiteral("I(S5), device_current"),
						   QStringLiteral("I(S4), device_current"),
						   QStringLiteral("I(S2), device_current"),
						   QStringLiteral("I(S3), device_current"),
						   QStringLiteral("I(S1), device_current"),
						   QStringLiteral("I(V13), device_current"),
						   QStringLiteral("I(V12), device_current"),
						   QStringLiteral("I(V11), device_current"),
						   QStringLiteral("I(V10), device_current"),
						   QStringLiteral("I(V9), device_current"),
						   QStringLiteral("I(V1), device_current"),
						   QStringLiteral("I(V8), device_current"),
						   QStringLiteral("I(V7), device_current"),
						   QStringLiteral("I(V6), device_current"),
						   QStringLiteral("I(V4), device_current"),
						   QStringLiteral("I(V5), device_current"),
						   QStringLiteral("I(V3), device_current"),
						   QStringLiteral("I(V2), device_current"),
						   QStringLiteral("Ix(u1:1), subckt_current"),
						   QStringLiteral("Ix(u1:2), subckt_current")};
}
