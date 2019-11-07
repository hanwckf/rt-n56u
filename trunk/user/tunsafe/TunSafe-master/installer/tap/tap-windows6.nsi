; ****************************************************************************
; * Copyright (C) 2002-2010 OpenVPN Technologies, Inc.                       *
; * Copyright (C)      2012 Alon Bar-Lev <alon.barlev@gmail.com>             *
; *  This program is free software; you can redistribute it and/or modify    *
; *  it under the terms of the GNU General Public License version 2          *
; *  as published by the Free Software Foundation.                           *
; ****************************************************************************

; TAP-Windows install script for Windows, using NSIS

SetCompressor /SOLID lzma

!addplugindir .
!include "MUI.nsh"
!include "StrFunc.nsh"
!include "x64.nsh"
!define MULTIUSER_EXECUTIONLEVEL Admin
!include "MultiUser.nsh"
!include FileFunc.nsh
!insertmacro GetParameters
!insertmacro GetOptions

!define PRODUCT_TAP_WIN_COMPONENT_ID "tap0901"
!define PRODUCT_NAME "TunSafe-TAP"
!define PRODUCT_VERSION "9.21.2"
!define PRODUCT_PUBLISHER "TunSafe"

${StrLoc}

;--------------------------------
;Configuration

;General


OutFile "TunSafe-TAP-${PRODUCT_VERSION}.exe"

BrandingText " "
ShowInstDetails show
ShowUninstDetails show

;--------------------------------
;Modern UI Configuration

Name "${PRODUCT_NAME}"

#!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME}, a kernel driver to provide virtual tap device #functionality on Windows originally written by James Yonan.\r\n\r\nNote that ${PRODUCT_NAME} will only run on Windows Vista or later.\r\n\r\n\r\n"

!define MUI_COMPONENTSPAGE_TEXT_TOP "Select the components to install/upgrade.  Stop any ${PRODUCT_NAME} processes or the ${PRODUCT_NAME} service if it is running."

#!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_ABORTWARNING
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "install-whirl.bmp"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_TEXT_LICENSE_TITLE "Welcome to the TunSafe-TAP installer"

#!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_PRE dirPre
!insertmacro MUI_PAGE_LICENSE "COPYING"
#!insertmacro MUI_PAGE_COMPONENTS
!define MUI_PAGE_CUSTOMFUNCTION_PRE dirPre
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
#!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
#!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Language Strings

LangString DESC_SecTAP ${LANG_ENGLISH} "Install/Upgrade the TAP Virtual Ethernet Adapter from OpenVPN."
LangString DESC_SecTAPUtilities ${LANG_ENGLISH} "Install the TAP Utilities."

Function dirPre
	${GetParameters} $R0
	${GetOptions} "$R0" "/X" $R1
	IfErrors +2 0
	Abort
FunctionEnd

;--------------------------------
;Installer Sections

Section "TAP Virtual Ethernet Adapter" SecTAP
	SetOverwrite on

	${If} ${RunningX64}
		DetailPrint "We are running on a 64-bit system."

		SetOutPath "$INSTDIR"
		File "prebuilt\x64\tapinstall.exe"

		SetOutPath "$INSTDIR\driver"
		File "prebuilt\x64\OemVista.inf"
		File "prebuilt\x64\${PRODUCT_TAP_WIN_COMPONENT_ID}.cat"
		File "prebuilt\x64\${PRODUCT_TAP_WIN_COMPONENT_ID}.sys"
	${Else}
		DetailPrint "We are running on a 32-bit system."

		SetOutPath "$INSTDIR"
		File "prebuilt\x86\tapinstall.exe"

		SetOutPath "$INSTDIR\driver"
		File "prebuilt\x86\OemVista.inf"
		File "prebuilt\x86\${PRODUCT_TAP_WIN_COMPONENT_ID}.cat"
		File "prebuilt\x86\${PRODUCT_TAP_WIN_COMPONENT_ID}.sys"
	${EndIf}
SectionEnd

Section "TAP Utilities" SecTAPUtilities
	SetOverwrite on

	# Delete previous start menu
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

	FileOpen $R0 "$INSTDIR\addtap.bat" w
	FileWrite $R0 "rem Add a new TAP virtual ethernet adapter$\r$\n"
	FileWrite $R0 '"$INSTDIR\tapinstall.exe" install "$INSTDIR\driver\OemVista.inf" ${PRODUCT_TAP_WIN_COMPONENT_ID}$\r$\n'
	FileWrite $R0 "pause$\r$\n"
	FileClose $R0

	FileOpen $R0 "$INSTDIR\deltapall.bat" w
	FileWrite $R0 "echo WARNING: this script will delete ALL TAP virtual adapters (use the device manager to delete adapters one at a time)$\r$\n"
	FileWrite $R0 "pause$\r$\n"
	FileWrite $R0 '"$INSTDIR\tapinstall.exe" remove ${PRODUCT_TAP_WIN_COMPONENT_ID}$\r$\n'
	FileWrite $R0 "pause$\r$\n"
	FileClose $R0

	; Create shortcuts
	CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}\Utilities"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Utilities\Add a new TAP virtual ethernet adapter.lnk" "$INSTDIR\addtap.bat" ""
	; set runas admin flag on the addtap link
	ShellLink::SetRunAsAdministrator "$SMPROGRAMS\${PRODUCT_NAME}\Utilities\Add a new TAP virtual ethernet adapter.lnk"
	Pop $0
	${If} $0 != 0
		DetailPrint "Setting RunAsAdmin flag on addtap failed: status = $0"
	${Endif}
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Utilities\Delete ALL TAP virtual ethernet adapters.lnk" "$INSTDIR\deltapall.bat" ""
	; set runas admin flag on the deltapall link
	ShellLink::SetRunAsAdministrator "$SMPROGRAMS\${PRODUCT_NAME}\Utilities\Delete ALL TAP virtual ethernet adapters.lnk"
	Pop $0
	${If} $0 != 0
		DetailPrint "Setting RunAsAdmin flag on deltapall failed: status = $0"
	${Endif}
SectionEnd

Function .onInit
	${GetParameters} $R0
	ClearErrors

${IfNot} ${AtLeastWin7}
	MessageBox MB_OK "TunSafe-TAP requires at least Windows 7"
	SetErrorLevel 1
	Quit
${EndIf}

	!insertmacro MULTIUSER_INIT
	SetShellVarContext all

	${If} $INSTDIR == ""
		StrCpy $1 "$PROGRAMFILES\TunSafe\TAP"
		${If} ${RunningX64}
			SetRegView 64
			StrCpy $1 "$PROGRAMFILES64\TunSafe\TAP"
		${EndIf}
		ReadRegStr $INSTDIR HKLM "SOFTWARE\${PRODUCT_NAME}" ""
		StrCmp $INSTDIR "" 0 +2
		StrCpy $INSTDIR $1
	${EndIf}
FunctionEnd

;--------------------------------
;Dependencies

Function .onSelChange
#	${If} ${SectionIsSelected} ${SecTAPUtilities}
#		!insertmacro SelectSection ${SecTAP}
#	${EndIf}
FunctionEnd

;--------------------
;Post-install section

Section -post

	; Store README, license, icon
	SetOverwrite on
	SetOutPath $INSTDIR
	File "COPYING"

	${If} ${SectionIsSelected} ${SecTAP}
		;
		; install/upgrade TAP driver if selected, using devcon
		;
		; TAP install/update was selected.
		; Should we install or update?
		; If tapinstall error occurred, $R5 will
		; be nonzero.
		IntOp $R5 0 & 0
		nsExec::ExecToStack '"$INSTDIR\tapinstall.exe" hwids ${PRODUCT_TAP_WIN_COMPONENT_ID}'
		Pop $R0 # return value/error/timeout
		IntOp $R5 $R5 | $R0
		DetailPrint "tapinstall.exe hwids returned: $R0"

		; If tapinstall output string contains "${PRODUCT_TAP_WIN_COMPONENT_ID}" we assume
		; that TAP device has been previously installed,
		; therefore we will update, not install.
		Push "${PRODUCT_TAP_WIN_COMPONENT_ID}"
		Push ">"
		Call StrLoc
		Pop $R0

		${If} $R5 == 0
			${If} $R0 == ""
				StrCpy $R1 "install"
			${Else}
				StrCpy $R1 "update"
			${EndIf}
			DetailPrint "TAP $R1 (${PRODUCT_TAP_WIN_COMPONENT_ID}) (May require confirmation)"
			nsExec::ExecToLog '"$INSTDIR\tapinstall.exe" $R1 "$INSTDIR\driver\OemVista.inf" ${PRODUCT_TAP_WIN_COMPONENT_ID}'
			Pop $R0 # return value/error/timeout
			${If} $R0 == ""
				IntOp $R0 0 & 0
				SetRebootFlag true
				DetailPrint "REBOOT flag set"
			${EndIf}
			IntOp $R5 $R5 | $R0
			DetailPrint "tapinstall.exe returned: $R0"
		${EndIf}

		DetailPrint "tapinstall.exe cumulative status: $R5"
		${If} $R5 != 0
			MessageBox MB_OK "An error occurred installing the TAP device driver."
		${EndIf}

		; Store install folder in registry
		WriteRegStr HKLM SOFTWARE\${PRODUCT_NAME} "" $INSTDIR
	${EndIf}

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Show up in Add/Remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
	WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayIcon" "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "HelpLink" "https://tunsafe.com/open-source"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "URLInfoAbout" "https://tunsafe.com"

	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "EstimatedSize" "$0"

	${GetParameters} $R0
	${GetOptions} "$R0" "/X" $R1
	IfErrors +3 0
	SetErrorLevel 0
	Quit

SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecTAP} $(DESC_SecTAP)
!insertmacro MUI_DESCRIPTION_TEXT ${SecTAPUtilities} $(DESC_SecTAPUtilities)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Function un.onInit
	ClearErrors
	!insertmacro MULTIUSER_UNINIT
	SetShellVarContext all
	${If} ${RunningX64}
		SetRegView 64
	${EndIf}
FunctionEnd

Section "Uninstall"
	DetailPrint "TAP REMOVE"
	nsExec::ExecToLog '"$INSTDIR\tapinstall.exe" remove ${PRODUCT_TAP_WIN_COMPONENT_ID}'
	Pop $R0 # return value/error/timeout
	DetailPrint "tapinstall.exe remove returned: $R0"

	Delete "$INSTDIR\tapinstall.exe"
	Delete "$INSTDIR\addtap.bat"
	Delete "$INSTDIR\deltapall.bat"

	Delete "$INSTDIR\driver\OemVista.inf"
	Delete "$INSTDIR\driver\${PRODUCT_TAP_WIN_COMPONENT_ID}.cat"
	Delete "$INSTDIR\driver\${PRODUCT_TAP_WIN_COMPONENT_ID}.sys"

	Delete "$INSTDIR\COPYING"
	Delete "$INSTDIR\Uninstall.exe"

	RMDir "$INSTDIR"
	RMDir "$INSTDIR\driver"
	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR"
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

	DeleteRegKey HKLM "SOFTWARE\${PRODUCT_NAME}"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
SectionEnd
