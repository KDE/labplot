namespace DCTransfer {

const QString filename = "DCTransfer/DCTransfer.raw";

QString refFileInfoString = R"(Title: * C:\users\martin\My Documents\Labplot\ltspice\DCTransfer.asc
<br>Date: Thu Mar 31 18:17:29 2022
<br>Plotname: DC transfer characteristic
<br>Flags: real forward
<br>No. Variables: 6
<br>No. Points:            5
<br>Offset:   0.0000000000000000e+000
<br>Command: Linear Technology Corporation LTspice XVII
<br>Variables:
<br>	0	v1	voltage
<br>	1	V(n001)	voltage
<br>	2	V(n002)	voltage
<br>	3	I(R2)	device_current
<br>	4	I(R1)	device_current
<br>	5	I(V1)	device_current
)"; // last \n is important

const int refDataRowCount = 5;
const int numberPreviewData = 3;

QStringList columnNames = {"v1, voltage", "V(n001), voltage", "V(n002), voltage", "I(R2), device_current", "I(R1), device_current", "I(V1), device_current"};
}
