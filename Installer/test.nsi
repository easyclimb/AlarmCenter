; 该脚本使用 易量安装(az.eliang.com) 向导生成
; 安装程序初始定义常量
!define PRODUCT_NAME "接警中心"
!define PROJDIR "C:\dev\AlarmCenter\"
!define /file PRODUCT_VERSION "${PROJDIR}\Release\VersionNo.ini"
!define PRODUCT_PUBLISHER "Hengbo Security, Inc."
!define PRODUCT_WEB_SITE "http://www.ffddcc.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AlarmCenter.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

;SetCompressor lzma
SetCompressor /SOLID lzma
SetCompressorDictSize 32

; 提升安装程序权限(vista,win7,win8)
RequestExecutionLevel admin

; ------ MUI 现代界面定义 ------
!include "MUI2.nsh"
!include "x64.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\eliang-install.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\eliang-uninstall.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\eliang-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\eliang-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\eliang-install.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\eliang-uninstall.bmp"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 开始菜单设置页面
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "接警中心"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
!define MUI_FINISHPAGE_RUN "$INSTDIR\AlarmCenter.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ChangeLog.txt"
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "AlarmCenter_Setup_V${PRODUCT_VERSION}.exe"
;ELiangID 统计编号     /*  安装统计项名称：【接警中心】  */
InstallDir "$PROGRAMFILES\AlarmCenter"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUninstDetails show
BrandingText " "

;安装包版本号格式必须为x.x.x.x的4组整数,每组整数范围0~65535,如:2.0.1.2
;若使用易量统计,版本号将用于区分不同版本的安装情况,此时建议用户务必填写正确的版本号
!define INSTALL_VERSION "${PRODUCT_VERSION}"

VIProductVersion "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductName"      "接警中心"
VIAddVersionKey /LANG=${LANG_SimpChinese} "Comments"         "接警中心(Hengbo Security, Inc.)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "CompanyName"      "Hengbo Security, Inc."
VIAddVersionKey /LANG=${LANG_SimpChinese} "LegalCopyright"   "Hengbo Security, Inc.(http://www.ffddcc.com)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileDescription"  "接警中心"
VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductVersion"   "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileVersion"      "${INSTALL_VERSION}"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  SetOutPath "$INSTDIR"

push $1
push $0

SetRegView 32

StrCpy $0 "asdf"
DetailPrint "NSIS is installed at: $0"

ReadRegStr $0 HKLM "SOFTWARE\7-Zip\Path" "asdfff"
MessageBox MB_OK $0


	ReadRegStr $1 HKLM "SOFTWARE\Classes\Installer\Dependencies\{23daf363-3020-4059-b3ae-dc4ad39fed19}\DisplayName" ""

    MessageBox MB_OK $1
	StrCmp $1 0 installed

pop $0
pop $1



;not installed, so run the installer

ExecWait '$INSTDIR/vcredist_x86.exe /q:a /c:”msiexec /i vcredist.msi /qb! /l*v %temp%\vcredist_x86.log”'



installed:

;we are done
  ;ExecWait 'vcredist_x86.exe /q:a /c:”msiexec /i vcredist.msi /qb! /l*v %temp%\vcredist_x86.log”'

SetOutPath "$INSTDIR"








; 创建开始菜单快捷方式
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\接警中心.lnk" "$INSTDIR\AlarmCenterDaemon.exe"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\历史记录.lnk" "$INSTDIR\data\history" "" "$INSTDIR\bk.ico"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\视频录像.lnk" "$INSTDIR\data\video_record" "" "$INSTDIR\video_record.ico"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\视频截图.lnk" "$INSTDIR\data\video_capture" "" "$INSTDIR\video_capture.ico"
  CreateShortCut "$DESKTOP\接警中心.lnk" "$INSTDIR\AlarmCenterDaemon.exe"
  CreateShortCut "$DESKTOP\历史记录.lnk" "$INSTDIR\data\history" "" "$INSTDIR\bk.ico"
  CreateShortCut "$DESKTOP\视频录像.lnk" "$INSTDIR\data\video_record" "" "$INSTDIR\video_record.ico"
  CreateShortCut "$DESKTOP\视频截图.lnk" "$INSTDIR\data\video_capture" "" "$INSTDIR\video_capture.ico"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteINIStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\访问接警中心主页.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\卸载接警中心.lnk" "$INSTDIR\uninst.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\AlarmCenter.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\AlarmCenter.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

/******************************
*  以下是安装程序的卸载部分  *
******************************/

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\AlarmCenter.exe"
  Delete "$INSTDIR\AlarmCenterDaemon.exe"
  Delete "$INSTDIR\python27.dll"
  Delete "$INSTDIR\VersionNo.ini"
  Delete "$INSTDIR\ChangeLog.txt"

  Delete "$INSTDIR\Log\*.*"
  ;Delete "$INSTDIR\Config\*.*"
  Delete "$INSTDIR\MapLib\*.*"
  ;Delete "$INSTDIR\Maps\*.*"
  Delete "$INSTDIR\*.*"

  Delete "$SMPROGRAMS\$ICONS_GROUP\访问接警中心主页.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\卸载接警中心.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\接警中心.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\历史记录.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\视频录像.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\视频截图.lnk"
  Delete "$DESKTOP\接警中心.lnk"
  Delete "$DESKTOP\历史记录.lnk"
  Delete "$DESKTOP\视频录像.lnk"
  Delete "$DESKTOP\视频截图.lnk"
  RMDir "$SMPROGRAMS\$ICONS_GROUP"

  ;RMDir "$INSTDIR\Log"
  ;RMDir "$INSTDIR\Config"
  RMDir "$INSTDIR\MapLib"
  ;RMDir "$INSTDIR\Maps"
  RMDir "$INSTDIR\*.*"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
SectionEnd

/* 根据 NSIS 脚本编辑规则,所有 Function 区段必须放置在 Section 区段之后编写,以避免安装程序出现未可预知的问题. */

Function un.onInit
FunctionEnd

Function un.onUninstSuccess
FunctionEnd
