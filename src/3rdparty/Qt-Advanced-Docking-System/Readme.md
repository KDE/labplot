Copy of Qt-Advanced-Docking-System
https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System

Commit id: 812ce784ce9ee2eba89d6a91e46cc4bcc86e9fd0 - v5.0 plus three minor fixes

Steps to update the library:
* pull the new code
* copy the new code into this folder
* adjust the code to our rules and conventions that are enforced:
    * replace Windows by Unix end of line character (find . -type f -print0 | xargs -0 dos2unix --)
    * replace the keyword 'emit' with 'Q_EMIT'
    * replace keyword 'signals' with 'Q_SIGNALS'
    * make use of QStringLiteral for char* strings
* Update this readme file to mention the new commit id

Our Modifications:
* CMakeLists.txt.patch
* The 4.4.0 version introduced a regression which we reverted in our copy. See https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/pull/640 - still required with v5.0?
