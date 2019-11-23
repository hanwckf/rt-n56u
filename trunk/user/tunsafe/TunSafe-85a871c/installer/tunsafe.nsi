; ****************************************************************************
; * Copyright (C)      2018 Ludde                                            *
; ****************************************************************************

SetCompressor /SOLID lzma

!addplugindir .
!include "MUI2.nsh"
!include "x64.nsh"
!define MULTIUSER_EXECUTIONLEVEL Admin
!include "MultiUser.nsh"
!include "servicelib.nsh"
!insertmacro GetParameters
!insertmacro GetOptions

!define PRODUCT_NAME "TunSafe"
!define PRODUCT_PUBLISHER "TunSafe"

OutFile "TunSafe-${PRODUCT_VERSION}.exe"

BrandingText " "
ShowInstDetails show
ShowUninstDetails show

Name "${PRODUCT_NAME}"

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_ABORTWARNING
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "tap\install-whirl.bmp"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

!define MUI_TEXT_LICENSE_TITLE "Welcome to the TunSafe installer"

#!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
#!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
#!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

LangString DESC_SecTAP ${LANG_ENGLISH} "Install the TunSafe client."
LangString DESC_SecTapAdapter ${LANG_ENGLISH} "Install the TunSafe-TAP Virtual Ethernet Adapter (GPL)."

Section "TunSafe Client" SecTunSafe
	SetOverwrite on
	${If} ${RunningX64}
		DetailPrint "Installing 64-bit version of TunSafe."
		SetOutPath "$INSTDIR"
		File "x64\TunSafe.exe"
		File "x64\TunSafe.com"
	${Else}
		DetailPrint "Installing 32-bit version of TunSafe."
		SetOutPath "$INSTDIR"
		File "x86\TunSafe.exe"
		File "x86\TunSafe.com"
	${EndIf}
	File "License.txt"
	File "ChangeLog.txt"
	CreateDirectory "$INSTDIR\Config"
	SetOutPath "$INSTDIR\Config"
	File "TunSafe.conf"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\TunSafe.lnk" "$INSTDIR\TunSafe.exe" ""
SectionEnd

Section "TunSafe-TAP Ethernet Adapter (GPL)" SecTapAdapter
	SetOverwrite on

	SetOutPath "$INSTDIR"

	File "tap\TunSafe-TAP Installer.exe"

 	HideWindow
	# Launch TunSafe-TAP installer
 	ExecWait '"$INSTDIR\TunSafe-TAP Installer.exe" /X /D=$INSTDIR\TAP' $1
	ShowWindow $HWNDPARENT ${SW_SHOW}
	${Unless} $1 = 0
		MessageBox MB_ICONEXCLAMATION "An error occurred while installing the TunSafe-TAP Virtual Ethernet Adapter. The installer will now abort."
		SetErrorLevel 1
	  Quit
	${EndUnless}

	BringToFront
SectionEnd

Function CloseTunsafe
again:
  FindWindow $0 "TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90"
  IntCmp $0 0 done
		MessageBox MB_ICONEXCLAMATION|MB_OKCANCEL "TunSafe is currently started. The installer will close TunSafe and proceed with the installation." IDOK proceed
			Quit
		proceed:
		SendMessage $0 1034 1 0 $1
		IntCmp $1 31337 proceed2
			MessageBox MB_ICONEXCLAMATION|MB_OKCANCEL "Unable to close TunSafe. Please close it and press OK to continue." IDOK again
			Quit
		proceed2:
		Sleep 500
		Goto again
	done:
	!insertmacro SERVICE stop TunSafeService ""
FunctionEnd

Function .onInit
	${GetParameters} $R0
	ClearErrors
${IfNot} ${AtLeastWin7}
	MessageBox MB_OK "TunSafe requires at least Windows 7"
	SetErrorLevel 1
	Quit
${EndIf}
	Call CloseTunsafe

	!insertmacro MULTIUSER_INIT
	SetShellVarContext all

	${If} $INSTDIR == ""
		StrCpy $1 "$PROGRAMFILES\TunSafe"
		${If} ${RunningX64}
			SetRegView 64
			StrCpy $1 "$PROGRAMFILES64\TunSafe"
		${EndIf}
		ReadRegStr $INSTDIR HKLM "SOFTWARE\${PRODUCT_NAME}" ""
		StrCmp $INSTDIR "" 0 +2
		StrCpy $INSTDIR $1
	${EndIf}
FunctionEnd

Section -post
	SetOverwrite on
	SetOutPath $INSTDIR

	WriteRegStr HKLM SOFTWARE\${PRODUCT_NAME} "" $INSTDIR

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Show up in Add/Remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
	WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayIcon" "$INSTDIR\TunSafe.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "HelpLink" "https://tunsafe.com"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "URLInfoAbout" "https://tunsafe.com"

SectionEnd

Function .onInstSuccess
	ExecShell "" "$INSTDIR\TunSafe.exe"
FunctionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecTunSafe} $(DESC_SecTAP)
!insertmacro MUI_DESCRIPTION_TEXT ${SecTapAdapter} $(DESC_SecTapAdapter)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.onInit
	ClearErrors
	!insertmacro MULTIUSER_UNINIT
	SetShellVarContext all
	${If} ${RunningX64}
		SetRegView 64
	${EndIf}
FunctionEnd

Section "Uninstall"
	!insertmacro SERVICE stop "TunSafeService" ""
	!insertmacro SERVICE delete "TunSafeService" ""


	Delete "$INSTDIR\TunSafe.exe"
	Delete "$INSTDIR\ts.exe"
	Delete "$INSTDIR\License.txt"
	Delete "$INSTDIR\ChangeLog.txt"
	Delete "$INSTDIR\Config\TunSafe.conf"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\TunSafe-TAP Installer.exe"

	RMDir "$INSTDIR"
	RMDir "$INSTDIR\Config"
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

	DeleteRegKey HKLM "SOFTWARE\${PRODUCT_NAME}"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
SectionEnd
