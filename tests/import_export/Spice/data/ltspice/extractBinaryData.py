import sys
import ltspice
import os

if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise Exception("Pass the filename as second argument!")
    filenameWithPath = sys.argv[1]

    (path, filename) = os.path.split(filenameWithPath)

    namespace = filename.split(".")[0]

    l = ltspice.Ltspice(filenameWithPath)
    l.parse()

    isComplex = False
    if "complex" in l.flags:
        isComplex = True

    data_str = ""
    size = len(l.x_raw)
    for r in range(size):
        line = ""
        for index, v in enumerate(l.variables):
            if v == "time":
                s = l.get_time()
            elif v == "frequency":
                s = l.get_frequency()
            else:
                s = l.get_data(v)
            line += f'{s[r].real:{1}.{15}e}'
            if isComplex:
                line += ","
                line += f'{s[r].imag:{1}.{15}e}'

            if index < len(l.variables) - 1:
                line += ","
        data_str += f"{line}\n"

    #data_str = f"QVector<QStringList> refData = {{{data_str}}};"
    #filename_string = f'const QString filename = "{filename}";\n'


    #data_str = f"namespace {namespace} {{\n\n{filename_string}\n{data_str}\n}}"



    with open(filenameWithPath + ".refdata", "w") as f:
        f.write(data_str)

