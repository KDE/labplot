namespace LowPassFilter_transient_doubleFlag {

const QString filename = "LowPassFilter_transient_doubleFlag.raw";

QString refFileInfoString = R"(Title: * Z:\home\martin\GITProjekte\labplot\tests\import_export\Spice\data\ltspice\LowPassFilter_transient_doubleFlag.asc
<br>Date: Sat Mar 12 08:33:33 2022
<br>Plotname: Transient Analysis
<br>Flags: real double forward
<br>No. Variables: 6
<br>No. Points:          535
<br>Offset:   0.0000000000000000e+000
<br>Command: Linear Technology Corporation LTspice XVII
<br>Variables:
<br>	0	time	time
<br>	1	V(n001)	voltage
<br>	2	V(n002)	voltage
<br>	3	I(C1)	device_current
<br>	4	I(R1)	device_current
<br>	5	I(V1)	device_current
)"; // last \n is important

const int refDataRowCount = 535;
const int numberPreviewData = 103;

QStringList columnNames = {	"time, time",
							"V(n001), voltage",
                            "V(n002), voltage",
                            "I(C1), device_current",
                            "I(R1), device_current",
							"I(V1), device_current"};
}
