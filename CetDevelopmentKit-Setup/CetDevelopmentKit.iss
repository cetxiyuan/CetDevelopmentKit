; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

#define MyAppName "CetDevelopmentKit"
#define MyAppVersion "2.0.1"
#define MyAppPublisher "CetXiyuan"
#define MyAppURL "https://blog.csdn.net/xiyuan255"
#define MyAppExeName "CetDevelopmentKit.exe"
#define MyAppIcon  "F:\sharefolder\cetqtlearn\CetDevelopmentKit\CetDevelopmentKit-Setup\favorite.ico"
;注： Release-正式版  Patch-补丁包
#define MyVersionTip "Release"

[Code]
function GetCustomInstallPath(DefaultPath: string): string;
begin
  // 在这里编写自定义逻辑来确定安装路径
  // 可以使用 Pascal 语言来实现各种判断和计算逻辑
  Result := 'D:\Program Files (x86)\CetXiyuan';
end;

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (若要生成新的 GUID，可在菜单中点击 "工具|生成 GUID"。)
AppId={{F0E929DF-B74B-4F3D-A131-E73D5CA54D42}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={code:GetCustomInstallPath}\{#MyAppName}
DisableProgramGroupPage=yes
; 以下行取消注释，以在非管理安装模式下运行（仅为当前用户安装）。
;PrivilegesRequired=lowest
OutputBaseFilename={#MyAppName}-Setup-{#MyVersionTip}-V{#MyAppVersion}
SetupIconFile={#MyAppIcon}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={#MyAppIcon}

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "F:\sharefolder\cetqtlearn\CetDevelopmentKit\CetDevelopmentKit-{#MyVersionTip}\CetDevelopmentKit.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "F:\sharefolder\cetqtlearn\CetDevelopmentKit\CetDevelopmentKit-{#MyVersionTip}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; 注意: 不要在任何共享系统文件上使用“Flags: ignoreversion”

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

