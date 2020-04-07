set -e
FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

python $FullScriptPath/updates.py $1 $2 $3 $4 $5 $6

exit
