; �ýű�ʹ�� HM VNISEdit �ű��༭���򵼲���

; ��װ�����ʼ���峣��
!define PRODUCT_NAME "���ݹ���"
!define PRODUCT_VERSION "1.0.1.8"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "E:\��̩ï�ɹ�������\1.pc��android�ֻ������ļ�\WPD��д�ֻ�SD��\WPD��д�ֻ�SD��\1.WPD_MTP_data\WPD_MTP_data\WPD_MTP_data\res\WPD_MTP_data.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; ��ӭҳ��
!insertmacro MUI_PAGE_WELCOME
; ��װĿ¼ѡ��ҳ��
!insertmacro MUI_PAGE_DIRECTORY
; ��װ����ҳ��
!insertmacro MUI_PAGE_INSTFILES
; ��װ���ҳ��
!insertmacro MUI_PAGE_FINISH

; ��װж�ع���ҳ��
!insertmacro MUI_UNPAGE_INSTFILES

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

; ��װԤ�ͷ��ļ�
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI �ִ����涨����� ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\���ݹ���"
ShowInstDetails show
ShowUnInstDetails show
BrandingText " "

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "E:\��̩ï�ɹ�������\1.pc��android�ֻ������ļ�\WPD��д�ֻ�SD��\WPD��д�ֻ�SD��\1.WPD_MTP_data\WPD_MTP_data\���\*.*"
  CreateShortCut "$DESKTOP\���ݹ���.lnk" "$INSTDIR\WPD_MTP_data.exe"
SectionEnd

Section -AdditionalIcons
  CreateDirectory "$SMPROGRAMS\���ݹ���"
  CreateShortCut "$SMPROGRAMS\���ݹ���\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
SectionEnd

/******************************
 *  �����ǰ�װ�����ж�ز���  *
 ******************************/

Section Uninstall
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\libmysql.dll"
  Delete "$INSTDIR\msvcp120.dll"
  Delete "$INSTDIR\msvcr120.dll"
  Delete "$INSTDIR\sqlite3.dll"
  Delete "$INSTDIR\WPD_MTP_data.exe"
  Delete "$INSTDIR\KLD_license"

  Delete "$SMPROGRAMS\���ݹ���\Uninstall.lnk"
  Delete "$DESKTOP\���ݹ���.lnk"

  RMDir "$SMPROGRAMS\���ݹ���"
  RMDir ""

  RMDir /r "$INSTDIR\logfiles"
  RMDir /r "$INSTDIR\config"
  RMDir /r "$INSTDIR\backUp"

  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

#-- ���� NSIS �ű��༭�������� Function ���α�������� Section ����֮���д���Ա��ⰲװ�������δ��Ԥ֪�����⡣--#

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "��ȷʵҪ��ȫ�Ƴ� $(^Name) ���������е������" IDYES +2
  Abort
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) �ѳɹ��ش����ļ�����Ƴ���"
FunctionEnd
