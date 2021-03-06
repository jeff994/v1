; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Antena Controller"
#define MyAppVersion "1.0"
#define MyAppPublisher "HOPE Technik, Inc."
#define MyAppURL "http://hopetechnik.com/"
#define MyAppExeName "VI_GUI.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{79A39AE2-1663-4E58-94F9-C364325CE497}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf64}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=setup_resource\license.txt
OutputBaseFilename=Antena Conroller Installer
Compression=lzma
SolidCompression=yes

[Code]

function GetProgramFiles(Param: string): string;
begin
  if IsWin64 then Result := ExpandConstant('{pf64}')
    else Result := ExpandConstant('{pf32}')
end;

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "..\x64\Release\VI_GUI.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\concrt140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\config.json"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140chs.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140cht.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140deu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140enu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140esn.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140fra.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140ita.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140jpn.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140kor.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140rus.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfc140u.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\mfcm140u.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\msvcp140_2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\vcamp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\vccorlib140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\vcomp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "setup_resource\*.ico"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

