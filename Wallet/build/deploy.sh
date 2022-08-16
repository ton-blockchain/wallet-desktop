set -e
FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

if [ ! -d "$FullScriptPath/../../../DesktopPrivate" ]; then
  echo ""
  echo "This script is for building the autoupdating version of TON Wallet."
  echo ""
  echo "For building custom versions please visit the build instructions page at:"
  echo "https://github.com/newton-blockchain/wallet-desktop/#build-instructions"
  exit
fi

Error () {
  cd $FullExecPath
  echo "$1"
  exit 1
}

DeployTarget="$1"

if [ ! -f "$FullScriptPath/target" ]; then
  Error "Build target not found!"
fi

while IFS='' read -r line || [[ -n "$line" ]]; do
  BuildTarget="$line"
done < "$FullScriptPath/target"

while IFS='' read -r line || [[ -n "$line" ]]; do
  set $line
  eval $1="$2"
done < "$FullScriptPath/version"

AppVersionStrFull="$AppVersionStr"

echo ""
HomePath="$FullScriptPath/.."
DeployMac="0"
DeployWin="0"
DeployLinux="0"
DeployLinux32="0"
if [ "$DeployTarget" == "mac" ]; then
  DeployMac="1"
  echo "Deploying version $AppVersionStrFull for OS X 10.12+.."
elif [ "$DeployTarget" == "win" ]; then
  DeployWin="1"
  echo "Deploying version $AppVersionStrFull for Windows.."
elif [ "$DeployTarget" == "linux" ]; then
  DeployLinux="1"
  echo "Deploying version $AppVersionStrFull for Linux 64 bit.."
elif [ "$DeployTarget" == "linux32" ]; then
  DeployLinux32="1"
  echo "Deploying version $AppVersionStrFull for Linux 32 bit.."
else
  DeployMac="1"
  DeployWin="1"
  DeployLinux="1"
  echo "Deploying three versions of $AppVersionStrFull: for Windows, OS X 10.12+ and Linux 64 bit.."
fi
if [ "$BuildTarget" == "mac" ]; then
  BackupPath="$HOME/Projects/backup/wallet"
elif [ "$BuildTarget" == "linux" ]; then
  BackupPath="/media/psf/Home/Projects/backup/wallet"
else
  Error "Can't deploy here"
fi
MacDeployPath="$BackupPath/$AppVersionStrMajor/$AppVersionStrFull/mac"
MacUpdateFile="wupdate-mac-$AppVersion"
MacSetupFile="wsetup.$AppVersionStrFull.dmg"
MacRemoteFolder="mac"
WinDeployPath="$BackupPath/$AppVersionStrMajor/$AppVersionStrFull/win"
WinUpdateFile="wupdate-win-$AppVersion"
WinSetupFile="wsetup.$AppVersionStrFull.exe"
WinPortableFile="wportable.$AppVersionStrFull.zip"
WinRemoteFolder="win"
LinuxDeployPath="$BackupPath/$AppVersionStrMajor/$AppVersionStrFull/linux"
LinuxUpdateFile="wupdate-linux-$AppVersion"
LinuxSetupFile="wsetup.$AppVersionStrFull.tar.xz"
LinuxRemoteFolder="linux"
Linux32DeployPath="$BackupPath/$AppVersionStrMajor/$AppVersionStrFull/linux32"
Linux32UpdateFile="wupdate-linux32-$AppVersion"
Linux32SetupFile="wsetup32.$AppVersionStrFull.tar.xz"
Linux32RemoteFolder="linux32"
DeployPath="$BackupPath/$AppVersionStrMajor/$AppVersionStrFull"

if [ "$DeployMac" == "1" ]; then
  if [ ! -f "$MacDeployPath/$MacUpdateFile" ]; then
    Error "$MacDeployPath/$MacUpdateFile not found!";
  fi
  if [ ! -f "$MacDeployPath/$MacSetupFile" ]; then
    Error "$MacDeployPath/$MacSetupFile not found!"
  fi
fi
if [ "$DeployWin" == "1" ]; then
  if [ ! -f "$WinDeployPath/$WinUpdateFile" ]; then
    Error "$WinUpdateFile not found!"
  fi
  if [ "$AlphaVersion" == "0" ]; then
    if [ ! -f "$WinDeployPath/$WinSetupFile" ]; then
      Error "$WinSetupFile not found!"
    fi
  fi
  if [ ! -f "$WinDeployPath/$WinPortableFile" ]; then
    Error "$WinPortableFile not found!"
  fi
fi
if [ "$DeployLinux" == "1" ]; then
  if [ ! -f "$LinuxDeployPath/$LinuxUpdateFile" ]; then
    Error "$LinuxDeployPath/$LinuxUpdateFile not found!"
  fi
  if [ ! -f "$LinuxDeployPath/$LinuxSetupFile" ]; then
    Error "$LinuxDeployPath/$LinuxSetupFile not found!"
  fi
fi
if [ "$DeployLinux32" == "1" ]; then
  if [ ! -f "$Linux32DeployPath/$Linux32UpdateFile" ]; then
    Error "$Linux32DeployPath/$Linux32UpdateFile not found!"
  fi
  if [ ! -f "$Linux32DeployPath/$Linux32SetupFile" ]; then
    Error "$Linux32DeployPath/$Linux32SetupFile not found!"
  fi
fi

declare -a Files
if [ "$DeployMac" == "1" ]; then
  Files+=("mac/$MacUpdateFile" "mac/$MacSetupFile")
fi
if [ "$DeployWin" == "1" ]; then
  Files+=("win/$WinUpdateFile" "win/$WinPortableFile" "win/$WinSetupFile")
fi
if [ "$DeployLinux" == "1" ]; then
  Files+=("linux/$LinuxUpdateFile" "linux/$LinuxSetupFile")
fi
if [ "$DeployLinux32" == "1" ]; then
  Files+=("linux32/$Linux32UpdateFile" "linux32/$Linux32SetupFile")
fi
cd $DeployPath
rsync -avR --progress ${Files[@]} "wallet:wallet-updates"

echo "Version $AppVersionStrFull was deployed!"
cd $FullExecPath

