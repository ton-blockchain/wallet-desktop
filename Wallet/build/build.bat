@echo OFF
setlocal enabledelayedexpansion
set "FullScriptPath=%~dp0"
set "FullExecPath=%cd%"

FOR /F "tokens=1* delims= " %%i in (%FullScriptPath%target) do set "BuildTarget=%%i"

FOR /F "tokens=1,2* delims= " %%i in (%FullScriptPath%version) do set "%%i=%%j"

set "VersionForPacker=%AppVersion%"
set "AlphaBetaParam="
set "AppVersionStrFull=%AppVersionStr%"

echo.
echo Building version %AppVersionStrFull% for Windows..
echo.

set "HomePath=%FullScriptPath%.."
set "SolutionPath=%HomePath%\.."
set "PortableFile=wallet.%AppVersionStrFull%.win.zip"
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

call gyp\refresh.bat
if %errorlevel% neq 0 goto error

cd "%SolutionPath%"
call ninja -C out/Release Wallet
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
if not exist "%ReleasePath%\deploy" mkdir "%ReleasePath%\deploy"
if not exist "%ReleasePath%\deploy\%AppVersionStrMajor%" mkdir "%ReleasePath%\deploy\%AppVersionStrMajor%"
mkdir "%DeployPath%"
mkdir "%DeployPath%\%BinaryName%"
if %errorlevel% neq 0 goto error

move "%ReleasePath%\%BinaryName%.exe" "%DeployPath%\%BinaryName%\"
xcopy "%ReleasePath%\%BinaryName%.pdb" "%DeployPath%\"
move "%ReleasePath%\%BinaryName%.exe.pdb" "%DeployPath%\"
if %errorlevel% neq 0 goto error

cd "%DeployPath%"
7z a -mx9 %PortableFile% %BinaryName%\
if %errorlevel% neq 0 goto error

move "%DeployPath%\%BinaryName%\%BinaryName%.exe" "%DeployPath%\"
rmdir "%DeployPath%\%BinaryName%"
if %errorlevel% neq 0 goto error

set "FinalDeployPath=%FinalReleasePath%\%AppVersionStrMajor%\%AppVersionStrFull%\tsetup"

echo.
echo Version %AppVersionStrFull% is ready for deploy!
echo.

if not exist "%DeployPath%\%PortableFile%" goto error
if not exist "%DeployPath%\%BinaryName%.pdb" goto error
if not exist "%DeployPath%\%BinaryName%.exe.pdb" goto error
md "%FinalDeployPath%"

xcopy "%DeployPath%\%PortableFile%" "%FinalDeployPath%\" /Y

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
