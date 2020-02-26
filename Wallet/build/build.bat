@echo OFF
setlocal enabledelayedexpansion
set "FullScriptPath=%~dp0"
set "FullExecPath=%cd%"

if not exist "%FullScriptPath%..\..\..\DesktopPrivate" (
  echo.
  echo This script is for building the autoupdating version of Gram Wallet.
  echo.
  echo For building custom versions please visit the build instructions page at:
  echo https://github.com/ton-blockchain/wallet-desktop/#build-instructions
  exit /b
)

FOR /F "tokens=1* delims= " %%i in (%FullScriptPath%target) do set "BuildTarget=%%i"

FOR /F "tokens=1,2* delims= " %%i in (%FullScriptPath%version) do set "%%i=%%j"

set "VersionForPacker=%AppVersion%"
set "AlphaBetaParam="
set "AppVersionStrFull=%AppVersionStr%"

echo.
echo Building version %AppVersionStrFull% for Windows..
echo.

set "HomePath=%FullScriptPath%.."
set "SolutionPath=%HomePath%\..\out"
set "UpdateFile=wupdate-win-%AppVersion%"
set "SetupFile=wsetup.%AppVersionStrFull%.exe"
set "PortableFile=wportable.%AppVersionStrFull%.zip"
set "ReleasePath=%HomePath%\..\out\Release"
set "DeployPath=%ReleasePath%\deploy\%AppVersionStrMajor%\%AppVersionStrFull%"
set "SignPath=%HomePath%\..\..\DesktopPrivate\Sign.bat"
set "BinaryName=Wallet"
set "FinalReleasePath=Z:\Projects\backup\wallet"

if not exist %FinalReleasePath% (
  echo Release path %FinalReleasePath% not found!
  exit /b 1
)
if exist %ReleasePath%\deploy\%AppVersionStrMajor%\%AppVersionStr%\ (
  echo Deploy folder for version %AppVersionStr% already exists!
  exit /b 1
)

cd "%HomePath%"

call configure.bat
if %errorlevel% neq 0 goto error

cd "%SolutionPath%"
call cmake --build . --config Release --target Wallet
if %errorlevel% neq 0 goto error

echo.
echo Version %AppVersionStrFull% build successfull. Preparing..
echo.

set "PATH=%PATH%;C:\Program Files\7-Zip"

cd "%ReleasePath%"

:sign1
call "%SignPath%" "%BinaryName%.exe"
if %errorlevel% neq 0 (
  timeout /t 3
  goto sign1
)

iscc /dMyAppVersion=%AppVersionStrSmall% /dMyAppVersionZero=%AppVersionStr% /dMyAppVersionFull=%AppVersionStrFull% "/dReleasePath=%ReleasePath%" "%FullScriptPath%setup.iss"
if %errorlevel% neq 0 goto error
if not exist "%SetupFile%" goto error
:sign2
call "%SignPath%" "%SetupFile%"
if %errorlevel% neq 0 (
  timeout /t 3
  goto sign2
)

call update_packer.exe --version %VersionForPacker% --path %BinaryName%.exe
if %errorlevel% neq 0 goto error
move "%ReleasePath%\packed_update%AppVersion%" "%ReleasePath%\%UpdateFile%"
if %errorlevel% neq 0 goto error

if not exist "%ReleasePath%\deploy" mkdir "%ReleasePath%\deploy"
if not exist "%ReleasePath%\deploy\%AppVersionStrMajor%" mkdir "%ReleasePath%\deploy\%AppVersionStrMajor%"
mkdir "%DeployPath%"
mkdir "%DeployPath%\%BinaryName%"
if %errorlevel% neq 0 goto error

move "%ReleasePath%\%BinaryName%.exe" "%DeployPath%\%BinaryName%\"
xcopy "%ReleasePath%\%BinaryName%.pdb" "%DeployPath%\"
move "%ReleasePath%\%BinaryName%.exe.pdb" "%DeployPath%\"
move "%ReleasePath%\%SetupFile%" "%DeployPath%\"
move "%ReleasePath%\%UpdateFile%" "%DeployPath%\"
if %errorlevel% neq 0 goto error

cd "%DeployPath%"
7z a -mx9 %PortableFile% %BinaryName%\
if %errorlevel% neq 0 goto error

move "%DeployPath%\%BinaryName%\%BinaryName%.exe" "%DeployPath%\"
rmdir "%DeployPath%\%BinaryName%"
if %errorlevel% neq 0 goto error

set "FinalDeployPath=%FinalReleasePath%\%AppVersionStrMajor%\%AppVersionStrFull%\win"

echo.
echo Version %AppVersionStrFull% is ready for deploy!
echo.

if not exist "%DeployPath%\%UpdateFile%" goto error
if not exist "%DeployPath%\%PortableFile%" goto error
if not exist "%DeployPath%\%SetupFile%" goto error
if not exist "%DeployPath%\%BinaryName%.pdb" goto error
md "%FinalDeployPath%"

xcopy "%DeployPath%\%UpdateFile%" "%FinalDeployPath%\" /Y
xcopy "%DeployPath%\%PortableFile%" "%FinalDeployPath%\" /Y
xcopy "%DeployPath%\%SetupFile%" "%FinalDeployPath%\" /Y

echo Version %AppVersionStrFull% is ready!

cd "%FullExecPath%"
exit /b

:error
(
  set ErrorCode=%errorlevel%
  if !ErrorCode! neq 0 (
    echo Error !ErrorCode!
  ) else (
    echo Error 666
    set ErrorCode=666
  )
  cd "%FullExecPath%"
  exit /b !ErrorCode!
)
