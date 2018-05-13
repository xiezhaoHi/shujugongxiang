; 该脚本使用 HM VNISEdit 脚本编辑器向导产生

; 安装程序初始定义常量
!define PRODUCT_NAME "数据共享"
!define PRODUCT_VERSION "1.0.1.8"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "E:\润泰茂成工作整理\1.pc与android手机互传文件\WPD读写手机SD卡\WPD读写手机SD卡\1.WPD_MTP_data\WPD_MTP_data\WPD_MTP_data\res\WPD_MTP_data.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_INSTFILES

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\数据共享"
ShowInstDetails show
ShowUnInstDetails show
BrandingText " "

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "E:\润泰茂成工作整理\1.pc与android手机互传文件\WPD读写手机SD卡\WPD读写手机SD卡\1.WPD_MTP_data\WPD_MTP_data\打包\*.*"
  CreateShortCut "$DESKTOP\数据共享.lnk" "$INSTDIR\WPD_MTP_data.exe"
SectionEnd

Section -AdditionalIcons
  CreateDirectory "$SMPROGRAMS\数据共享"
  CreateShortCut "$SMPROGRAMS\数据共享\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
SectionEnd

/******************************
 *  以下是安装程序的卸载部分  *
 ******************************/

Section Uninstall
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\libmysql.dll"
  Delete "$INSTDIR\msvcp120.dll"
  Delete "$INSTDIR\msvcr120.dll"
  Delete "$INSTDIR\sqlite3.dll"
  Delete "$INSTDIR\WPD_MTP_data.exe"
  Delete "$INSTDIR\KLD_license"

  Delete "$SMPROGRAMS\数据共享\Uninstall.lnk"
  Delete "$DESKTOP\数据共享.lnk"

  RMDir "$SMPROGRAMS\数据共享"
  RMDir ""

  RMDir /r "$INSTDIR\logfiles"
  RMDir /r "$INSTDIR\config"
  RMDir /r "$INSTDIR\backUp"

  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "您确实要完全移除 $(^Name) ，及其所有的组件？" IDYES +2
  Abort
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) 已成功地从您的计算机移除。"
FunctionEnd
