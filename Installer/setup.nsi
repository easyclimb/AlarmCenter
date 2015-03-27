; �ýű�ʹ�� ������װ(az.eliang.com) ������
; ��װ�����ʼ���峣��
!define PRODUCT_NAME "�Ӿ�����"
!define /file PRODUCT_VERSION "C:\AlarmCenter\Release\versionno.ini"
!define PRODUCT_PUBLISHER "Hengbo Security, Inc."
!define PRODUCT_WEB_SITE "http://www.ffddcc.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AlarmCenter.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor /SOLID lzma
SetCompressorDictSize 32

; ������װ����Ȩ��(vista,win7,win8)
RequestExecutionLevel admin

; ------ MUI �ִ����涨�� ------
!include "MUI2.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\eliang-install.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\eliang-uninstall.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\eliang-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\eliang-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\eliang-install.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\eliang-uninstall.bmp"

; ��ӭҳ��
!insertmacro MUI_PAGE_WELCOME
; ��װĿ¼ѡ��ҳ��
!insertmacro MUI_PAGE_DIRECTORY
; ��ʼ�˵�����ҳ��
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "�Ӿ�����"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; ��װ����ҳ��
!insertmacro MUI_PAGE_INSTFILES
; ��װ���ҳ��
!define MUI_FINISHPAGE_RUN "$INSTDIR\AlarmCenter.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ChangeLog.txt"
!insertmacro MUI_PAGE_FINISH

; ��װж�ع���ҳ��
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

; ------ MUI �ִ����涨����� ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "AlarmCenter_Setup_V${PRODUCT_VERSION}.exe"
;ELiangID ͳ�Ʊ��     /*  ��װͳ�������ƣ����Ӿ����ġ�  */
InstallDir "$PROGRAMFILES\�Ӿ�����"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUninstDetails show
BrandingText " "

;��װ���汾�Ÿ�ʽ����Ϊx.x.x.x��4������,ÿ��������Χ0~65535,��:2.0.1.2
;��ʹ������ͳ��,�汾�Ž��������ֲ�ͬ�汾�İ�װ���,��ʱ�����û������д��ȷ�İ汾��
!define INSTALL_VERSION "0.0.2.1597"

VIProductVersion "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductName"      "�Ӿ�����"
VIAddVersionKey /LANG=${LANG_SimpChinese} "Comments"         "�Ӿ�����(Hengbo Security, Inc.)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "CompanyName"      "Hengbo Security, Inc."
VIAddVersionKey /LANG=${LANG_SimpChinese} "LegalCopyright"   "Hengbo Security, Inc.(http://www.ffddcc.com)"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileDescription"  "�Ӿ�����"
VIAddVersionKey /LANG=${LANG_SimpChinese} "ProductVersion"   "${INSTALL_VERSION}"
VIAddVersionKey /LANG=${LANG_SimpChinese} "FileVersion"      "${INSTALL_VERSION}"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "C:\AlarmCenter\Release\AlarmCenter.exe"
  File "C:\AlarmCenter\Installer\AlarmCenter.mdb"
  File "C:\AlarmCenter\Installer\HistoryRecord.mdb"
  File "C:\AlarmCenter\Installer\user_info.mdb"
  File "C:\AlarmCenter\Release\QrCode.dll"
  File "C:\AlarmCenter\Release\VersionNo.ini"
  File "C:\AlarmCenter\ChangeLog.txt"
  SetOutPath "$INSTDIR\SoundFiles"
  File "C:\AlarmCenter\Installer\SoundFiles\*.wav"
  SetOutPath "$INSTDIR\Detector"
  File "C:\AlarmCenter\Installer\Detectors\*.bmp"
  SetOutPath "$INSTDIR\Log"
  SetOutPath "$INSTDIR\Config"
  SetOutPath "$INSTDIR\Maps"
  SetOutPath "$INSTDIR\MapLib"

; ������ʼ�˵���ݷ�ʽ
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\�Ӿ�����.lnk" "$INSTDIR\AlarmCenter.exe"
  CreateShortCut "$DESKTOP\�Ӿ�����.lnk" "$INSTDIR\AlarmCenter.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteINIStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\���ʽӾ�������ҳ.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\ж�ؽӾ�����.lnk" "$INSTDIR\uninst.exe"
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
*  �����ǰ�װ�����ж�ز���  *
******************************/

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\AlarmCenter.exe"
  Delete "$INSTDIR\AlarmCenter.mdb"
  Delete "$INSTDIR\HistoryRecord.mdb"
  Delete "$INSTDIR\user_info.mdb"
  Delete "$INSTDIR\QrCode.dll"
  Delete "$INSTDIR\VersionNo.ini"
  Delete "$INSTDIR\ChangeLog.txt"

  Delete "$INSTDIR\Log\*.*"
  Delete "$INSTDIR\Config\*.*"
  Delete "$INSTDIR\MapLib\*.*"
  Delete "$INSTDIR\Maps\*.*"

  Delete "$SMPROGRAMS\$ICONS_GROUP\���ʽӾ�������ҳ.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\ж�ؽӾ�����.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\�Ӿ�����.lnk"
  Delete "$DESKTOP\�Ӿ�����.lnk"

  RMDir "$SMPROGRAMS\$ICONS_GROUP"

  RMDir "$INSTDIR\Log"
  RMDir "$INSTDIR\Config"
  RMDir "$INSTDIR\MapLib"
  RMDir "$INSTDIR\Maps"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
SectionEnd

/* ���� NSIS �ű��༭����,���� Function ���α�������� Section ����֮���д,�Ա��ⰲװ�������δ��Ԥ֪������. */

Function un.onInit
FunctionEnd

Function un.onUninstSuccess
FunctionEnd
