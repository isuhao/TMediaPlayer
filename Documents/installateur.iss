
[Setup]
AppId={{636B2DBF-C6E8-448D-A78E-7894B72281E3}
AppName=TMediaPlayer
AppVersion=1.0.74
AppPublisher=Ted
DefaultDirName={pf}\TMediaPlayer
DefaultGroupName=TMediaPlayer
AllowNoIcons=yes
LicenseFile=..\License.txt
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\TMediaPlayer.exe,0
UninstallDisplayName="TMediaPlayer"
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "..\Bin\Win32\Release\TMediaPlayer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\fmodex.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\tag.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\icudt54.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\icuin54.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\icuuc54.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Multimedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5MultimediaWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Positioning.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5PrintSupport.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Qml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Quick.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Sensors.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Sql.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5WebChannel.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5WebKit.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5WebKitWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\Qt5Xml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Bin\Win32\Release\plugins\*.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\Lang\*.qm"; DestDir: "{app}\Lang"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\TMediaPlayer"; Filename: "{app}\TMediaPlayer.exe"
Name: "{group}\{cm:UninstallProgram,TMediaPlayer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\TMediaPlayer"; Filename: "{app}\TMediaPlayer.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\TMediaPlayer"; Filename: "{app}\TMediaPlayer.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\TMediaPlayer.exe"; Description: "{cm:LaunchProgram,TMediaPlayer}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: ".flac"; ValueType: string; ValueName: ""; ValueData: "TMediaPlayerFLAC"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "TMediaPlayerFLAC"; ValueType: string; ValueName: ""; ValueData: "Fichier musical FLAC"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TMediaPlayerFLAC\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\TMediaPlayer.exe,1"
Root: HKCR; Subkey: "TMediaPlayerFLAC\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\TMediaPlayer.exe"" ""%1"""

Root: HKCR; Subkey: ".mp3"; ValueType: string; ValueName: ""; ValueData: "TMediaPlayerMP3"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "TMediaPlayerMP3"; ValueType: string; ValueName: ""; ValueData: "Fichier musical MP3"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TMediaPlayerMP3\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\TMediaPlayer.exe,2"
Root: HKCR; Subkey: "TMediaPlayerMP3\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\TMediaPlayer.exe"" ""%1"""

Root: HKCR; Subkey: ".wav"; ValueType: string; ValueName: ""; ValueData: "TMediaPlayerWAV"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "TMediaPlayerWAV"; ValueType: string; ValueName: ""; ValueData: "Fichier musical WAV"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TMediaPlayerWAV\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\TMediaPlayer.exe,3"
Root: HKCR; Subkey: "TMediaPlayerWAV\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\TMediaPlayer.exe"" ""%1"""

Root: HKCR; Subkey: ".wma"; ValueType: string; ValueName: ""; ValueData: "TMediaPlayerWMA"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "TMediaPlayerWMA"; ValueType: string; ValueName: ""; ValueData: "Fichier musical WMA"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TMediaPlayerWMA\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\TMediaPlayer.exe,4"
Root: HKCR; Subkey: "TMediaPlayerWMA\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\TMediaPlayer.exe"" ""%1"""
