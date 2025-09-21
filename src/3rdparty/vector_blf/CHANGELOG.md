# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [2.4.2] - 2023-01-19
### Fixed
- Removed default constructor in ObjectQueue<ObjectHeaderBase>. Added initializers.

## [2.4.1] - 2021-11-12
### Changed
- Drop CMAKE_BUILD_TYPE from CMakeLists to allow passing it to cmake.
- CanFdMessage handling. ValidDataBytes set automatically.
- Introduced SPDX REUSE specification.
### Fixed
- Fix ObjectQueue virtuals. Clean queue in destructor.
- UncompressedFile stops reading, when bufferSize is reached.

## [2.4.0] - 2021-01-29
### Added
- New Types: CanSettingChanged, DistributedObjectMember, AttributeEvent
- Add support for Restore Points (which are stored in Unknown115 objects)
### Changed
- Drop VectorTypes.h and use std types instead.
### Fixed
- Fixes in CanErrorFrameExt, CanFdMessage, CanFdMessage64, CanMessage2, EthernetFrame, LinMessage2
- Fix potential deadlock in File::close
- Fix FileStatistics: application, apiNumber, compressionLevel, objectsRead

## [2.3.1] - 2020-07-16
### Fixed
- Fix Unknown115:write

## [2.3.0] - 2020-05-27
### Added
- Add method hasExtData to CanFdErrorFrame64 and CanFdMessage64.
### Changed
- Variable length of reserved data for CanFdExtFrameData and Unknown115.
- Parser example enhanced for CanFdExtFrameData.

## [2.2.0] - 2020-05-07
### Changed
- Brought some more insight in Unknown115.
- ObjectHeader constructors improved. Clang warning fixed.
- Changed some methods/arguments to const, where possible.

## [2.1.9] - 2020-04-30
### Fixed
- Fix a free, on an undefined pointer.

## [2.1.8] - 2020-04-29
### Fixed
- Fix a memory leak with large chunks of data.

## [2.1.7] - 2020-01-20
### Changed
- Convert classes to struct. Only public members. No invariants.
- Change style to Google

## [2.1.6] - 2019-08-26
### Changed
- Make use of C++11 and member initializer lists
### Fixed
- Type/operator issues with std::streamoff/streamsize/streampos

## [2.1.5] - 2019-03-02
### Fixed
- CanErrorFrame's reserved field is optional

## [2.1.4] - 2019-02-26
### Fixed
- WaterMarkEvent has a reserved field

## [2.1.3] - 2019-01-30
### Fixed
- Fixed sporadic exception on close

## [2.1.2] - 2018-11-24
### Added
- Example that shows how to write files
### Fixed
- Fix empty list check in UncompressedFile::write()
- Test AllLogFiles fixed

## [2.1.1] - 2018-11-11
### Added
- Jenkinsfile and Dockerfiles added
### Changed
- Update binlog examples to 4.7.1.0
### Fixed
- Fix padding in FlexRayVFrReceiveMsgEx
- Compilation of example parser with VS >=2015

## [2.1.0] - 2018-09-15
### Added
- New Objects: FunctionBus, DataLostBegin/End, WaterMarkEvent, TriggerCondition
### Changed
- Updated from binlog API version 4.5.2.2 to 4.7.1.0
### Fixed
- Fixed CanFdMessage64

## [2.0.1] - 2018-08-23
### Fixed
- Fixed memory leak with introduction of std::shared_ptr

## [2.0.0] - 2018-03-06
### Changed
- Multi-Threading support using two threads between three data structures.
- File::OpenMode replaced by standard std::ios_base::openmode.
- LogContainer now contains uncompressedFile and compressedFile containers.
- LogContainer now contains uncompress/compress methods.
- UncompressedFile now based on std::list<LogContainer *> for performance.

## [1.2.1] - 2017-12-29
### Added
- Preliminary support for object type 115. EOF message incl. FileStatistics.
### Fixed
- Further tests, especially writing files.
- Fix log write with compressionLevel = 0.
- Correct test BLF files.

## [1.2.0] - 2017-12-20
### Added
- Object generator to create blf test samples under Windows.
### Changed
- Updated from binlog API version 3.9.6.0 to 4.5.2.2.
### Fixed
- Implemented test cases for all object types, and fixed them.

## [1.1.2] - 2017-10-27
### Changed
- CXX_EXTENSIONS added and gcc pedantic flag for portability reasons
### Fixed
- CXX_STANDARD_REQUIRED was fixed to reflect C++11 requirement

## [1.1.1] - 2017-09-22
### Changed
- Update to latest project template
- "using" instead of "typedef"
### Fixed
- generate_export_header got lost.

## [1.1.0] - 2016-03-22
### Added
- Exceptions to handle unexpected file or object signature.
### Changed
- C++11 override and final clauses
- Fixed further static compiler warnings under msvc, clang
### Fixed
- compressionLevel is now used. Previously only default compression was applied.
- Update CMakeLists.txt to latest project standard. Maybe fix #1.

## [1.0.2] - 2016-06-30
### Changed
- Performance improvements in File::read
- Closed many static compiler warnings under gcc, msvc, clang
### Fixed
- Fixed crash when using Visual Studio and 32-bit compilation
- Compiler hardening flags made compatible with gcc-4.8

## [1.0.1] - 2016-05-20
### Changed
- Compiler hardening flags enabled

## [1.0.0] - 2016-05-20
### Added
- Initial version
