import sys
import struct

if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise Exception("Pass the filename as second argument!")
    filename = sys.argv[1]

    with open(filename, "rb") as f:

        number_variables = 0
        number_points = 0
        line = f.readline()
        flags = b""
        while b"Binary:" not in line:
            if b"No. Variables: " in line:
                number_variables = int(line.split(b"No. Variables: ")[1])
            if b"No. Points: " in line:
                number_points = int(line.split(b"No. Points: ")[1])
            if b"Flags: " in line:
                flags = line.split(b"Flags: ")[1]
            line = f.readline()

        isComplex = b"complex" in flags
        #d = bytearray(f.read())
        data_str = ""
        for r in range(number_points):
            line = ''
            for c in range(number_variables):
                value = struct.unpack('d', f.read(8))[0]  # every data is a double (8byte)
                line += f'"{value:{1}.{15}e}"'
                if isComplex:
                    line += ", "
                    value = struct.unpack('d', f.read(8))[0]  # every data is a double (8byte)
                    line += f'"{value:{1}.{15}e}"'
                if c < number_variables - 1:
                    line += ", "


            line = f"{{{line}}}"
            if r < number_points - 1:
                line += ",\n"
            data_str += line
        data_str = f"QVector<QStringList> refData = {{{data_str}}};"

        with open(filename + ".data", "w") as f:
            f.write(data_str)

