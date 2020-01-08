#!/bin/bash 

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
echo $SCRIPTPATH

if [ -d $SCRIPTPATH/build ]; then
   echo "build directory already exists."
   echo "If you want to reconfigure the project, remove the build directory."
   exit 1
fi

cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..
echo "**** All done! ****"
echo "**** Do a make in build/ dir ****"

exit 
