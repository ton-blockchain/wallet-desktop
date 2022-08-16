set -e
FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

Param1="$1"
Param2="$2"
Param3="$3"
Param4="$4"

if [ ! -d "$FullScriptPath/../../../DesktopPrivate" ]; then
  echo ""
  echo "This script is for building the offical version of TON Wallet."
  echo ""
  echo "For building custom versions please visit the build instructions page at:"
  echo "https://github.com/ton-blockchain/wallet-desktop/#build-instructions"
  exit
fi

Error () {
  cd $FullExecPath
  echo "$1"
  exit 1
}

while IFS='' read -r line || [[ -n "$line" ]]; do
  set $line
  eval $1="$2"
done < "$FullScriptPath/version"

cd "$FullScriptPath"
python3 release.py $AppVersionStr $Param1 $Param2 $Param3 $Param4
