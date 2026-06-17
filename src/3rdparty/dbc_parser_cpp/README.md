# C++ DBC Parser

This is to provide a library header only file to read in DBC files. I was looking around and couldn't
find a simple library that didn't have dependencies. So here we are making one. I got some inspiration
from the python dbc library here: https://pypi.org/project/cantools/

## Building

I am using Cmake to be able to build the tests and the lib. I plan on doing more with it but this is what it
is for now. This doesn't mean that IDEs aren't
welcome but the build process might not be suited for this. You will need to modify it for your
needs. Feel free to submit changes so the building process will be more robust.

Here are the steps to get started:
```bash
# Release Build
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -H.

# Debug Build
cmake -DCMAKE_BUILD_TYPE=Debug -Bbuild -H.

# Run the build
cmake --build build
```

### Listing Build Options

You can check the latest build options with cmake. After you configure cmake you can run this.
```shell
cd build

# List this projects options
cmake -LH .. | grep -B1 "DBC_"

# To see all the included project cache variables and options
cmake -LAH ..
```

### Creating a Single Header File

It requires you have `cargo` installed from rust. See these instructions if you don't have that https://www.rust-lang.org/tools/install.
It uses the https://github.com/Felerius/cpp-amalgamate crate to do the single header file creation.

The output will be generated in the `build/single_header/libdbc/` folder. You can run a cmake command to build this as well as other targets.

To just build the single header you can simply run the target:
```shell
cmake -Bbuild -H. -DDBC_GENERATE_SINGLE_HEADER=ON

cmake --build build --parallel `nproc` --target single_header
```


## Testing

I am trying to always make sure that this is very well tested code. I am using Catch2 to do this
testing and if you aren't familiar here is the documentation: https://github.com/catchorg/Catch2/blob/master/docs/Readme.md#top

There is one option you will want for testing: `DBC_TEST_LOCALE_INDEPENDENCE`. This requires the `de_DE.UTF-8` locale installed to test. It is for checking we don't rely on locale to convert floats. i.e. 1.23 vs 1,23

You will need to configure the project to enable this: `cmake -DCMAKE_BUILD_TYPE=Release -DDBC_TEST_LOCALE_INDEPENDENCE=ON -Bbuild -H.`. You will get a warning if it isn't enabled because it isn't enabled by default.

To run the tests locally you can use the following. Assuming you have built the project you should get a test executable.
```bash
ctest --output-on-failure --test-dir build
```

## Scripts

To use the scripts in `scripts/` you will need to install the requirements
from the top level directory.
```bash
pip install -r script-requirements.txt
```

## Contributing

I welcome all help! Please feel free to fork and start some pull requests!
You can see the issues sections for some ideas what might need to be done.