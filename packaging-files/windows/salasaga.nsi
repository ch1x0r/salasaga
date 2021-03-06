# Customised from auto-generated EclipseNSIS script

Name "salasaga"
SetCompressor /SOLID lzma

# Defines
!define REGKEY "SOFTWARE\$(^Name)"
!define VERSION 0.8.0.0
!define COMPANY "Justin Clift"
!define URL http://www.salasaga.org

# MUI defines
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_LICENSEPAGE_CHECKBOX
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Salasaga"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULT_FOLDER "Salasaga"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

# Included files
!include Sections.nsh
!include MUI.nsh

# Reserved Files

# Variables
Var StartMenuGroup

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE C:\eclipse\workspace\salasaga\LICENSE.TXT
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!insertmacro MUI_LANGUAGE English

# Installer attributes
OutFile "salasaga_setup_${VERSION}_2008mmdd.exe"
InstallDir $PROGRAMFILES\Salasaga
CRCCheck on
XPStyle on
ShowInstDetails show
VIProductVersion ${VERSION}
VIAddVersionKey ProductName "Salasaga"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey CompanyWebsite "${URL}"
VIAddVersionKey FileVersion ""
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""
InstallDirRegKey HKLM "${REGKEY}" Path
ShowUninstDetails show

# Installer sections
Section -Main SEC0000
    SetOutPath $INSTDIR
    SetOverwrite on
    File C:\eclipse\workspace\salasaga_ide\Release\salasaga.exe
    File C:\eclipse\workspace\salasaga_screencapture\Release\salasaga_screencapture.exe
    File C:\eclipse\workspace\salasaga_keycapture\Release\libsalasaga_keycapture.dll
    File C:\install\compiled\Salasaga\libungif4.dll
    File C:\install\compiled\Salasaga\libxml2.dll
    File C:\install\compiled\Salasaga\AUTHORS
    File C:\install\compiled\Salasaga\LICENSE.TXT
    File C:\eclipse\workspace\sf-packaging-files\windows\included_licenses\GIFLIB_license.txt
    SetOutPath $INSTDIR\fonts
    File /r C:\install\compiled\Salasaga\fonts\*
    SetOutPath $INSTDIR\icons
    File /r C:\install\compiled\Salasaga\icons\*
    SetOutPath $INSTDIR\sounds
    File /r C:\install\compiled\Salasaga\sounds\*
    WriteRegStr HKLM "${REGKEY}\Components" Main 1
SectionEnd

Section -post SEC0001
    WriteRegStr HKLM "${REGKEY}" Path $INSTDIR
    WriteUninstaller $INSTDIR\uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuGroup"
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Salasaga.lnk" "$INSTDIR\salasaga.exe" "" "$INSTDIR\icons\salasaga-icon.ico"
    CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\uninstall.exe
    !insertmacro MUI_STARTMENU_WRITE_END
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\uninstall.exe
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\uninstall.exe
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoModify 1
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoRepair 1
SectionEnd

# Macro for selecting uninstaller sections
!macro SELECT_UNSECTION SECTION_NAME UNSECTION_ID
    Push $R0
    ReadRegStr $R0 HKLM "${REGKEY}\Components" "${SECTION_NAME}"
    StrCmp $R0 1 0 next${UNSECTION_ID}
    !insertmacro SelectSection "${UNSECTION_ID}"
    GoTo done${UNSECTION_ID}
next${UNSECTION_ID}:
    !insertmacro UnselectSection "${UNSECTION_ID}"
done${UNSECTION_ID}:
    Pop $R0
!macroend

# Uninstaller sections
Section /o un.Main UNSEC0000
    RmDir /r /REBOOTOK $INSTDIR\sounds
    RmDir /r /REBOOTOK $INSTDIR\icons
    RmDir /r /REBOOTOK $INSTDIR\fonts
    Delete /REBOOTOK $INSTDIR\LICENSE.TXT
    Delete /REBOOTOK $INSTDIR\GIFLIB_license.txt
    Delete /REBOOTOK $INSTDIR\AUTHORS
    Delete /REBOOTOK $INSTDIR\salasaga_screencapture.exe
    Delete /REBOOTOK $INSTDIR\salasaga.exe
    Delete /REBOOTOK $INSTDIR\libsalasaga_keycapture.dll
    Delete /REBOOTOK $INSTDIR\libungif4.dll
    Delete /REBOOTOK $INSTDIR\libxml2.dll
    DeleteRegValue HKLM "${REGKEY}\Components" Main
SectionEnd

Section un.post UNSEC0001
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Salasaga.lnk"
    Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
    Delete /REBOOTOK $INSTDIR\uninstall.exe
    DeleteRegValue HKLM "${REGKEY}" StartMenuGroup
    DeleteRegValue HKLM "${REGKEY}" Path
    DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
    DeleteRegKey /IfEmpty HKLM "${REGKEY}"
    RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
    RmDir /REBOOTOK $INSTDIR
SectionEnd

# Installer functions
Function .onInit
    InitPluginsDir
FunctionEnd

# Uninstaller functions
Function un.onInit
    ReadRegStr $INSTDIR HKLM "${REGKEY}" Path
    ReadRegStr $StartMenuGroup HKLM "${REGKEY}" StartMenuGroup
    !insertmacro SELECT_UNSECTION Main ${UNSEC0000}
FunctionEnd
