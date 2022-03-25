namespace Wakeup {

const QString filename = "Windows/Wakeup.raw";

QString refFileInfoString = R"(Title: * Wakeup\Wakeup.asc
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
)";// last \n is important



const int refDataRowCount = 189;
const int numberPreviewData = 103;

QStringList columnNames = { "time, time",
                            "V(n013), voltage",
                            "V(vin), voltage",
                            "V(n001), voltage",
                            "V(n008), voltage",
                            "V(n004), voltage",
                            "V(n018), voltage",
                            "V(p001), voltage",
                            "V(n003), voltage",
                            "V(n012), voltage",
                            "V(n015), voltage",
                            "V(n002), voltage",
                            "V(n009), voltage",
                            "V(n020), voltage",
                            "V(n023), voltage",
                            "V(n021), voltage",
                            "V(n022), voltage",
                            "V(n019), voltage",
                            "V(n005), voltage",
                            "V(n014), voltage",
                            "V(n016), voltage",
                            "V(n006), voltage",
                            "V(n010), voltage",
                            "V(n017), voltage",
                            "V(n007), voltage",
                            "V(n011), voltage",
                            "I(C1), device_current",
                            "I(D2), device_current",
                            "I(R1), device_current",
                            "I(R8), device_current",
                            "I(R9), device_current",
                            "I(R4), device_current",
                            "I(S7), device_current",
                            "I(S6), device_current",
                            "I(S5), device_current",
                            "I(S4), device_current",
                            "I(S2), device_current",
                            "I(S3), device_current",
                            "I(S1), device_current",
                            "I(V13), device_current",
                            "I(V12), device_current",
                            "I(V11), device_current",
                            "I(V10), device_current",
                            "I(V9), device_current",
                            "I(V1), device_current",
                            "I(V8), device_current",
                            "I(V7), device_current",
                            "I(V6), device_current",
                            "I(V4), device_current",
                            "I(V5), device_current",
                            "I(V3), device_current",
                            "I(V2), device_current",
                            "Ix(u1:1), subckt_current",
                            "Ix(u1:2), subckt_current"};
}
