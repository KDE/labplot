#define MyAppName "LabPlot2"
#define MyAppVersion "2.7.0"
#define MyAppPublisher "Stefan Gerlach"
#define MyAppURL "https://labplot.kde.org"
#define MyAppExeName "labplot2.exe"
#define ImageMagickVersion "ImageMagick-7.0.7-Q16"
#define CraftRoot "C:\CraftRoot"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{EAFA7C2D-F2C4-4337-A4D3-3912BEA4F535}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf64}\{#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=labplot-{#MyAppVersion}-64bit-setup
ArchitecturesAllowed=x64
;use "none" for testing (much faster)
Compression=lzma
SolidCompression=yes
Uninstallable=yes
;we install a file association for lml projects
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#CraftRoot}\bin\labplot2.exe"; DestDir: "{app}"; Flags: ignoreversion
; use: windeployqt.exe --release bin\labplot2.exe --dir DEPLOY
Source: "{#CraftRoot}\DEPLOY\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion
; fix https://stackoverflow.com/questions/20495620/qt-5-1-1-application-failed-to-start-because-platform-plugin-windows-is-missi
Source: "{#CraftRoot}\mingw64\bin\libssp-0.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libz.dll"; DestDir: "{app}";Flags: ignoreversion
;Source: "{#CraftRoot}\bin\liblzma.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\liblzma-5.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libintl-8.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\iconv.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libeay32.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libfreetype-6.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libpng15.dll"; DestDir: "{app}";Flags: ignoreversion
;Source: "{#CraftRoot}\bin\ssleay32.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libssl-1_1-x64.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libcrypto-1_1-x64.dll"; DestDir: "{app}";Flags: ignoreversion
;Source: "{#CraftRoot}\bin\libdbus-1-3.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\Qt5DBus.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libkdewin.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Archive.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Attica.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Auth.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5AuthCore.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Bookmarks.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Codecs.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Completion.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5ConfigCore.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5ConfigGui.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5ConfigWidgets.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5CoreAddons.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Crash.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5DBusAddons.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5GlobalAccel.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5GuiAddons.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5I18n.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5IconThemes.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5ItemViews.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5JobWidgets.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5KIOCore.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5KIOFileWidgets.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5KIOWidgets.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5NewStuff.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5NewStuffCore.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Parts.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Service.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5Solid.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5SonnetCore.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5SonnetUi.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5SyntaxHighlighting.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5TextEditor.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5TextWidgets.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5WidgetsAddons.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5WindowSystem.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libKF5XmlGui.dll"; DestDir: "{app}";Flags: ignoreversion
;Source: "{#CraftRoot}\bin\libKF5Notifications.dll"; DestDir: "{app}";Flags: ignoreversion
; missing lib on minimal Windows 10
Source: "C:\Python36\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "{#CraftRoot}\dev-utils\bin\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion
; Cantor
Source: "{#CraftRoot}\bin\cantor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\cantor_scripteditor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\cantor_python3server.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\cantor_juliaserver.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\libcantorlibs.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\libcantor_config.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\libcantor_pythonbackend.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\plugins\libcantorpart.dll"; DestDir: "{app}\plugins"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\plugins\cantor\*"; DestDir: "{app}\plugins\cantor"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\kxmlgui5\cantor\*"; DestDir: "{app}\data"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\cantor\*"; DestDir: "{app}\data"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\doc\HTML\en\cantor\*"; DestDir: "{app}\doc\HTML\en\cantor";Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\etc\xdg\cantor*"; DestDir: "{app}\etc\xdg"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\metainfo\org.kde.cantor.appdata.xml"; DestDir: "{app}\data\metainfo"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\data\applications\org.kde.cantor.desktop"; DestDir: "{app}\data\applications"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\data\config.kcfg\*"; DestDir: "{app}\data\config.kcfg"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\icons\hicolor\48x48\apps\*"; DestDir: "{app}\data\icons\hicolor\48x48\apps\"; Flags: recursesubdirs ignoreversion
; misc
Source: "{#CraftRoot}\bin\libfftw3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\netcdf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\hdf5_hl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\hdf5.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\szip.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\liblz4.so.1.8.3.dll"; DestDir: "{app}"; DestName: "liblz4.dll"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\libmysql.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\libpq.dll"; DestDir: "{app}"; Flags: ignoreversion
; TODO craft does not install own version (check)
;Source: "C:\Program Files\cfitsio\cfitsio.dll"; DestDir: "{app}";Flags: ignoreversion

Source: "{#CraftRoot}\bin\data\labplot2\*"; Excludes: "splash.png,\pics,\themes,\colorschemes"; DestDir: "{app}\labplot2"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\kxmlgui5\labplot2\labplot2ui.rc"; DestDir: "{app}\labplot2"; Flags: ignoreversion
; TODO check if needed:
Source: "{#CraftRoot}\bin\data\kxmlgui5\labplot2\labplot2ui.rc"; DestDir: "{app}\kxmlgui5\labplot2"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\data\labplot2\pics\*"; DestDir: "{app}\data\pics"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\labplot2\themes\*"; DestDir: "{app}\data\themes"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\labplot2\color-schemes\*"; DestDir: "{app}\data\color-schemes"; Flags: recursesubdirs ignoreversion
Source: "{#CraftRoot}\bin\data\labplot2\splash.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\data\metainfo\org.kde.labplot2.appdata.xml"; DestDir: "{app}\data\metainfo"; Flags: ignoreversion
Source: "{#CraftRoot}\bin\data\applications\org.kde.labplot2.desktop"; DestDir: "{app}\data\applications"; Flags: ignoreversion
; Source: "{#CraftRoot}\labplot\labplot2.cmd"; DestDir: "{app}";Flags: ignoreversion

; icon theme
Source: "{#CraftRoot}\bin\data\icontheme.rcc"; DestDir: "{app}\data";Flags: ignoreversion
; oxygen icons
;Source: "{#CraftRoot}\bin\data\icons\hicolor\*"; DestDir: "{app}\icons\hicolor"; Flags: recursesubdirs ignoreversion

; handbook
Source: "{#CraftRoot}\bin\data\doc\HTML\en\labplot2\*"; DestDir: "{app}\doc\HTML\en\labplot2";Flags: recursesubdirs ignoreversion
; for SVG icons
Source: "{#CraftRoot}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\iconengines";Flags: ignoreversion
Source: "{#CraftRoot}\bin\libexpat.dll"; DestDir: "{app}";Flags: ignoreversion

; convert
; TODO
Source: "C:\Program Files\{#ImageMagickVersion}\convert.exe"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_MagickCore_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_MagickWand_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_bzlib_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_glib_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_lcms_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_lqr_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_png_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_ttf_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\CORE_RL_zlib_.dll"; DestDir: "{app}";Flags: ignoreversion
; Source: "C:\Program Files\{#ImageMagickVersion}\msvcr120.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\vcomp120.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\delegates.xml"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\magic.xml"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\modules\coders\IM_MOD_RL_pdf_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\modules\coders\IM_MOD_RL_png_.dll"; DestDir: "{app}";Flags: ignoreversion
Source: "C:\Program Files\{#ImageMagickVersion}\modules\coders\IM_MOD_RL_ps_.dll"; DestDir: "{app}";Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{app}\labplot2\labplot2.ico"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
; project file association
Root: HKCR; Subkey: ".lml"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "{#MyAppName}"; ValueType: string; ValueName: ""; ValueData: "MyView"; Flags: uninsdeletekey
Root: HKCR; Subkey: "{#MyAppName}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\labplot2\application-x-labplot2.ico,0"
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
; Root: HKCU; Subkey: "Environment"; ValueType:string; ValueName:"KDEROOT"; ValueData:"{app}" ; Flags: preservestringtype ;
