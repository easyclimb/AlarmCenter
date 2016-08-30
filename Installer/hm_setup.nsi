﻿; Script generated by the HM NIS Edit Script Wizard.

!include "WinVer.nsh"

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Alarm Center"
!define PROJDIR "C:\dev\AlarmCenter\"
!define GOOGLE_LIB_DIR "D:\dev_libs\google\"
!define /file PRODUCT_VERSION "${PROJDIR}\Release\VersionNo.ini"
!define PRODUCT_PUBLISHER "Hengbo, Inc."
!define PRODUCT_WEB_SITE "http://www.ffddcc.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AlarmCenter.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor lzma
;SetCompressor /SOLID lzma
SetCompressorDictSize 64

; 提升安装程序权限(vista,win7,win8)
RequestExecutionLevel admin

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${PROJDIR}\Installer\eliang-install.ico"
!define MUI_UNICON "${PROJDIR}\Installer\eliang-uninstall.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE $(myLicenseData)
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Alarm Center"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\AlarmCenterDaemon.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ChangeLog.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"

LicenseLangString myLicenseData ${LANG_ENGLISH} "C:\dev\AlarmCenterNet\YourSoftwareLicence_en.txt"
LicenseLangString myLicenseData ${LANG_SIMPCHINESE} "C:\dev\AlarmCenterNet\YourSoftwareLicence.txt"
LicenseLangString myLicenseData ${LANG_TRADCHINESE} "C:\dev\AlarmCenterNet\YourSoftwareLicence_tw.txt"
LicenseData $(myLicenseData)

LangString myLangString ${LANG_ENGLISH} "English"
LangString myLangString ${LANG_SIMPCHINESE} "中文简体"
LangString myLangString ${LANG_TRADCHINESE} "中文繁體"

LangString strAlarmCenter ${LANG_ENGLISH} "AlarmCenter"
LangString strAlarmCenter ${LANG_SIMPCHINESE} "接警中心"
LangString strAlarmCenter ${LANG_TRADCHINESE} "接警中心"

LangString strHisroty ${LANG_ENGLISH} "Hisroty Record"
LangString strHisroty ${LANG_SIMPCHINESE} "历史记录"
LangString strHisroty ${LANG_TRADCHINESE} "歷史記錄"

LangString strVideo ${LANG_ENGLISH} "Video Record"
LangString strVideo ${LANG_SIMPCHINESE} "视频录像"
LangString strVideo ${LANG_TRADCHINESE} "視訊錄影"

LangString strVideoCapture ${LANG_ENGLISH} "Video Capture"
LangString strVideoCapture ${LANG_SIMPCHINESE} "视频截图"
LangString strVideoCapture ${LANG_TRADCHINESE} "視訊截圖"

LangString strVisitHomePage ${LANG_ENGLISH} "Visit Alarm Center Home Page"
LangString strVisitHomePage ${LANG_SIMPCHINESE} "访问接警中心主页"
LangString strVisitHomePage ${LANG_TRADCHINESE} "訪問接警中心主頁"

LangString strUninstall ${LANG_ENGLISH} "Uninstall Alarm Center"
LangString strUninstall ${LANG_SIMPCHINESE} "卸载接警中心"
LangString strUninstall ${LANG_TRADCHINESE} "卸载接警中心"

LangString myAreYouSureToUninstallString ${LANG_ENGLISH} "Are you sure to uninstall"
LangString myAreYouSureToUninstallString ${LANG_SIMPCHINESE} "你确实要完全移除"
LangString myAreYouSureToUninstallString ${LANG_TRADCHINESE} "你確實要完全移除"

LangString myAndAllComponentsString ${LANG_ENGLISH} "and all its components? "
LangString myAndAllComponentsString ${LANG_SIMPCHINESE} "及其所有的组件？"
LangString myAndAllComponentsString ${LANG_TRADCHINESE} "及其所有的元件？"

LangString myUninstallOKString ${LANG_ENGLISH} "has been uninstalled from your computer."
LangString myUninstallOKString ${LANG_SIMPCHINESE} "已成功地从你的计算机移除。"
LangString myUninstallOKString ${LANG_TRADCHINESE} "已成功地從你的計算機移除。"

LangString myUnsupportSystem ${LANG_ENGLISH} "Operating System not support! Alarm Center need at least Windows Vista or higher version Windows Operating systems."
LangString myUnsupportSystem ${LANG_SIMPCHINESE} "不支持的操作系统！接警中心可以运行在 Window Vista 及更新的操作系统版本上。"
LangString myUnsupportSystem ${LANG_TRADCHINESE} "不支持的操作系統！接警中心可以運行在 Window Vista 及更新的操作系統版本上。"


; Reserve files
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "AlarmCenter_Setup_V${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\AlarmCenter"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
BrandingText " "

;安装包版本号格式必须为x.x.x.x的4组整数,每组整数范围0~65535,如:2.0.1.2
;若使用易量统计,版本号将用于区分不同版本的安装情况,此时建议用户务必填写正确的版本号
!define INSTALL_VERSION "${PRODUCT_VERSION}"

VIProductVersion "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_English} "ProductName"      "AlarmCenter"
VIAddVersionKey /LANG=${LANG_English} "Comments"         "AlarmCenter(Hengbo Security, Inc.)"
VIAddVersionKey /LANG=${LANG_English} "CompanyName"      "Hengbo Security, Inc."
VIAddVersionKey /LANG=${LANG_English} "LegalCopyright"   "Hengbo Security, Inc.(http://www.ffddcc.com)"
VIAddVersionKey /LANG=${LANG_English} "FileDescription"  "AlarmCenter"
VIAddVersionKey /LANG=${LANG_English} "ProductVersion"   "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_English} "FileVersion"      "${INSTALL_VERSION}"

VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductName"      "接警中心"
VIAddVersionKey /LANG=${LANG_SimpChinese} "Comments"         "接警中心(Hengbo Security, Inc.)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "CompanyName"      "Hengbo Security, Inc."
VIAddVersionKey /LANG=${LANG_SimpChinese} "LegalCopyright"   "Hengbo Security, Inc.(http://www.ffddcc.com)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileDescription"  "接警中心"
VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductVersion"   "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileVersion"      "${INSTALL_VERSION}"

VIAddVersionKey /LANG=${LANG_TradChinese} "ProductName"      "接警中心"
VIAddVersionKey /LANG=${LANG_TradChinese} "Comments"         "接警中心(Hengbo Security, Inc.)"
VIAddVersionKey /LANG=${LANG_TradChinese} "CompanyName"      "Hengbo Security, Inc."
VIAddVersionKey /LANG=${LANG_TRADCHINESE} "LegalCopyright"   "Hengbo Security, Inc.(http://www.ffddcc.com)"
VIAddVersionKey /LANG=${LANG_TradChinese} "FileDescription"  "接警中心"
VIAddVersionKey /LANG=${LANG_TradChinese} "ProductVersion"   "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_TradChinese} "FileVersion"      "${INSTALL_VERSION}"


Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "${PROJDIR}\Release\AlarmCenter.exe"
  File "${PROJDIR}\Release\AlarmCenterDaemon.exe"
  File "${PROJDIR}\Release\AlarmCenterMap.exe"
  File "${PROJDIR}\Release\AlarmCenterVideo.exe"
  File "${PROJDIR}\Release\AlarmCenter.pdb" 
  File "${PROJDIR}\Release\VersionNo.ini"
  File "${PROJDIR}\Installer\ChangeLog.txt"
  File "${PROJDIR}\Installer\crashrpt_lang.ini"
  File "${PROJDIR}\Installer\CrashRpt1403.dll"
  File "${PROJDIR}\Installer\CrashSender1403.exe"
  ; vc
  File "${PROJDIR}\Installer\msvcp140.dll"
  File "${PROJDIR}\Installer\vcruntime140.dll"
  File "${PROJDIR}\Installer\win_kits_x86\*.dll"
  SetOutPath "$INSTDIR\update"
  SetOutPath "$INSTDIR\lang"
  File "${PROJDIR}\Installer\lang\*.txt"
  SetOutPath "$INSTDIR"

  

  ; CEF
  File "${GOOGLE_LIB_DIR}cef\cef_binary_3.2171.1902_windows32\out\Release\*.dll"
  File "${GOOGLE_LIB_DIR}cef\cef_binary_3.2171.1902_windows32\out\Release\*.pak"
  File "${GOOGLE_LIB_DIR}cef\cef_binary_3.2171.1902_windows32\out\Release\icudtl.dat"
  SetOutPath "$INSTDIR\locales"
  File "${GOOGLE_LIB_DIR}cef\cef_binary_3.2171.1902_windows32\out\Release\locales\*.*"
  SetOutPath "$INSTDIR"
  ; CEF END

  File "${PROJDIR}\Installer\bk.ico"
  File "${PROJDIR}\Installer\video_record.ico"
  File "${PROJDIR}\Installer\video_capture.ico"

  ; EZVIZ
  SetOutPath "$INSTDIR\contrib\ezviz"
  File "${PROJDIR}\Installer\contrib\ezviz\*.dll"
  SetOutPath "$INSTDIR\contrib\ezviz\Microsoft.VC90.CRT"
  File "${PROJDIR}\Installer\contrib\ezviz\Microsoft.VC90.CRT\*.*"
  SetOutPath "$INSTDIR\contrib\ezviz\Microsoft.VC90.MFC"
  File "${PROJDIR}\Installer\contrib\ezviz\Microsoft.VC90.MFC\*.*"
  ; EZVIZ END
  
  ; JOVISION
  SetOutPath "$INSTDIR\contrib\jovision"
  File "${PROJDIR}\Installer\contrib\jovision\*.*"
  ; JOVISION END


  SetOutPath "$INSTDIR\SoundFiles\zh-cn"
  File "${PROJDIR}\Installer\SoundFiles\zh-cn\*.wav"
  SetOutPath "$INSTDIR\SoundFiles\zh-tw"
  File "${PROJDIR}\Installer\SoundFiles\zh-tw\*.wav"
  SetOutPath "$INSTDIR\SoundFiles\en-us"
  File "${PROJDIR}\Installer\SoundFiles\en-us\*.wav"
  SetOutPath "$INSTDIR\Detectors"
  File "${PROJDIR}\Installer\Detectors\*.bmp"
  SetOutPath "$INSTDIR\Log"
  SetOutPath "$INSTDIR\Data"
  SetOutPath "$INSTDIR\data\Config"
  File "${PROJDIR}\Installer\AlarmCenter.mdb"
  File "${PROJDIR}\Installer\HistoryRecord.mdb"
  File "${PROJDIR}\Installer\user_info.mdb"
  File "${PROJDIR}\Installer\sms.mdb"
  File "${PROJDIR}\Installer\video.mdb"
  File "${PROJDIR}\Installer\BaiduMapDlg_zh-cn.htm"
  File "${PROJDIR}\Installer\BaiduMapDlg_zh-tw.htm"
  File "${PROJDIR}\Installer\BaiduMapDlg_en-us.htm"

  StrCmp "$(myLangString)" 'English' 0 label_chinese
  File "${PROJDIR}\Installer\config_en-us\lang.json"
  File "${PROJDIR}\Installer\config_en-us\service.mdb"
  Goto labal_lang_end
label_chinese:
  StrCmp "$(myLangString)" '中文简体' 0 label_taiwaness
  File "${PROJDIR}\Installer\config_zh-cn\lang.json"
  File "${PROJDIR}\Installer\config_zh-cn\service.mdb"
  Goto labal_lang_end
label_taiwaness:
  File "${PROJDIR}\Installer\config_zh-tw\lang.json"
  File "${PROJDIR}\Installer\config_zh-tw\service.mdb"
  
labal_lang_end:

  SetOutPath "$INSTDIR\data\Maps"
  SetOutPath "$INSTDIR\MapLib"
  SetOutPath "$INSTDIR\data\history"
  SetOutPath "$INSTDIR\data\video_capture"
  SetOutPath "$INSTDIR\data\video_record"
  SetOutPath "$INSTDIR\Resource"
  File "${PROJDIR}\Installer\Resource\*.bmp"
  SetOutPath "$INSTDIR"
  File "${PROJDIR}\Installer\sqlite3.dll"


; 1.4.0.xxxx 测试版专用
  SetOutPath "$INSTDIR\7-Zip"
  File "${PROJDIR}\Installer\7-Zip\*.dll"
  File "${PROJDIR}\Installer\7-Zip\7z.exe"

  SetOutPath "$INSTDIR"
  File "${PROJDIR}\Release\AlarmCenterUpdateTool.exe"
  ExecWait "$INSTDIR\AlarmCenterUpdateTool.exe"

  SetOutPath "$INSTDIR"
  ;File "${PROJDIR}\Release\vcredist_x86.exe"
  ;ExecWait 'vcredist_x86.exe /q:a /c:”msiexec /i vcredist.msi /qb! /l*v %temp%\vcredist_x86.log”'



; 创建开始菜单快捷方式
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\$(strAlarmCenter).lnk" "$INSTDIR\AlarmCenterDaemon.exe"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\$(strHisroty).lnk" "$INSTDIR\data\history" "" "$INSTDIR\bk.ico"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\$(strVideo).lnk" "$INSTDIR\data\video_record" "" "$INSTDIR\video_record.ico"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\$(strVideoCapture).lnk" "$INSTDIR\data\video_capture" "" "$INSTDIR\video_capture.ico"
  CreateShortCut "$DESKTOP\$(strAlarmCenter).lnk" "$INSTDIR\AlarmCenterDaemon.exe"
  CreateShortCut "$DESKTOP\$(strHisroty).lnk" "$INSTDIR\data\history" "" "$INSTDIR\bk.ico"
  CreateShortCut "$DESKTOP\$(strVideo).lnk" "$INSTDIR\data\video_record" "" "$INSTDIR\video_record.ico"
  CreateShortCut "$DESKTOP\$(strVideoCapture).lnk" "$INSTDIR\data\video_capture" "" "$INSTDIR\video_capture.ico"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\$(strUninstall).lnk" "$INSTDIR\uninst.exe"
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


Function .onInit
  ${Unless} ${AtLeastWinVista}
  MessageBox MB_ICONSTOP|MB_OK "$(myUnsupportSystem)"
  Abort
  ${EndIf}

  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) $(myUninstallOKString)"
FunctionEnd

Function un.onInit
!insertmacro MUI_UNGETLANGUAGE
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(myAreYouSureToUninstallString) $(^Name) $(myAndAllComponentsString)" IDYES +2
  Abort
FunctionEnd

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

  Delete "$SMPROGRAMS\$ICONS_GROUP\Website.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\$(strUninstall).lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\$(strAlarmCenter).lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\$(strHisroty).lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\$(strVideo).lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\$(strVideoCapture).lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\*.lnk"
  Delete "$DESKTOP\$(strAlarmCenter).lnk"
  Delete "$DESKTOP\$(strHisroty).lnk"
  Delete "$DESKTOP\$(strVideo).lnk"
  Delete "$DESKTOP\$(strVideoCapture).lnk"
  RMDir "$SMPROGRAMS\$ICONS_GROUP"

  ;RMDir "$INSTDIR\Log"
  ;RMDir "$INSTDIR\Config"
  RMDir "$INSTDIR\MapLib"
  ;RMDir "$INSTDIR\Maps"
  RMDir "$INSTDIR\*.*"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd