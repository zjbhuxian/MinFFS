; // **************************************************************************
; // * Copyright (C) 2014,2015 abcdec @GitHub - All Rights Reserved           *
; // * This file is part of a modified version of FreeFileSync, MinFFS.       *
; // * The original FreeFileSync program and source code are distributed by   *
; // * the FreeFileSync project: http://www.freefilesync.org/                 *
; // * This particular file is created by by abcdec @GitHub as part of the    *
; // * MinFFS project: https://github.com/abcdec/MinFFS                       *
; // *                          --EXPERIMENTAL--                              *
; // * This program is experimental and not recommended for general use.      *
; // * Please consider using the original FreeFileSync program unless there   *
; // * are specific needs to use this experimental MinFFS version.            *
; // *                          --EXPERIMENTAL--                              *
; // * This file is distributed under GNU General Public License:             *
; // * http://www.gnu.org/licenses/gpl-3.0 per the FreeFileSync License.      *
; // * This modified program is distributed in the hope that it will be       *
; // * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of *
; // * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
; // * General Public License for more details.                               *
; // **************************************************************************

;
; MinFFS-Setup.nsi
;

;--------------------------------
; Preparation

; Constants
!define PROGRAMNAME "MinFFS"
!define PROGRAMVERSION "1.7.6.1"
!define PACKAGENAME "${NAME} ${VERSION}"
!define PACKGESOURCEDIR "bin-release"

; Use Modern UI
!include "MUI2.nsh"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"

;--------------------------------
; Program definition

; Request for admin priviledge
RequestExecutionLevel admin

; The name of the installer
Name "${PROGRAMNAME} Setup"
OutFile "${PROGRAMNAME}-${PROGRAMVERSION}-Setup.exe"
InstallDir $PROGRAMFILES\${PROGRAMNAME}

; Use old install directory if registerred before
InstallDirRegKey HKLM "Software\NSIS_MinFFS" "Install_Dir"

;--------------------------------
; Installer "Page" definition

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "bin-release\License.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;--------------------------------
; Uninstaller "Page" definition

;UninstPage uninstConfirm
;UninstPage instfiles
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Kurdish"
!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Uzbek"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "Afrikaans"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Esperanto"
!insertmacro MUI_LANGUAGE "Basque"
;!insertmacro MUI_LANGUAGE "Pashto"
!insertmacro MUI_LANGUAGE "Georgian"
!insertmacro MUI_LANGUAGE "Vietnamese"
!insertmacro MUI_LANGUAGE "Welsh"
!insertmacro MUI_LANGUAGE "Armenian"


;--------------------------------

; First section is a component that must be installed.
Section "MinFFS required components"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Install packaged files
  File "${PACKGESOURCEDIR}\FreeFileSync.chm"
  File "${PACKGESOURCEDIR}\Resources.zip"
  File "${PACKGESOURCEDIR}\License.rtf"
  File /r "${PACKGESOURCEDIR}\Bin"
  File /r "${PACKGESOURCEDIR}\Languages"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_MinFFS "Install_Dir" "$INSTDIR"
  
  ; For uninstallation from Windows Control Panel Program Management
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MinFFS" "DisplayName" "MinFFS Uninstaller"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MinFFS" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MinFFS" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MinFFS" "NoRepair" 1

  ; Create uninstaller in SetOutPath directory
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional components
Section "Desktop Shortcuts"

  CreateShortCut "$DESKTOP\${PROGRAMNAME}.lnk" "$INSTDIR\Bin\MinFFS.exe" "" "$INSTDIR\Bin\MinFFS.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MinFFS"
  DeleteRegKey HKLM SOFTWARE\NSIS_MinFFS

  Delete "$DESKTOP\${PROGRAMNAME}.lnk"

  RMDir /r "$INSTDIR\Bin"
  RMDir /r "$INSTDIR\Languages"
  Delete "$INSTDIR\FreeFileSync.chm"
  Delete "$INSTDIR\Resources.zip"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\License.rtf"

SectionEnd
