#!/bin/bash
echo "begin to build roboeffect_demo: $#"
workdir=$(cd $(dirname $0); pwd)
echo "$workdir"
if [ $# -ge 1 ]
then
	if [ "$1" = "build" ]
	then
		echo "build"
		mkdir cmake_build
		chmod +x tools/adapt_files_gen.py
		nds_ldsag -t $ANDES_COMPILE_UTILS/nds32_template.txt nds32-ae210p-cm.sag -o nds32-ae210p.ld
		python3 tools/adapt_files_gen.py -i middleware/roboeffect/graphics/ -o main/
		cd ./cmake_build
		cmake ..
		make all 2>&1 | tee log.log
		python3 ../tools/gcc_error_msg.py log.log
		cd $workdir 
	elif [ "$1" = "clean" ]
	then
		echo "clean"
		cd ./cmake_build
		make clean
		cd $workdir
	elif [ "$1" = "release" ]
	then
		:
	else
		:
	fi
else
	echo "parameters error! [build][clean][copy]"
fi