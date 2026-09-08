@echo off
setlocal enabledelayedexpansion

rem 在这里修改andes安装的根目录
set "AndesRoot=D:\AndesToolsLite"

set "AndesRootUtils=%AndesRoot%\utils"
set "AndesRootIce=%AndesRoot%\ice"
set "AndesRootCygwin=%AndesRoot%\cygwin\bin"
set "AndesRootCygwin_lib=%AndesRoot%\cygwin\bin\lib"
set "AndesRootGcc=%AndesRoot%\toolchains\nds32le-elf-mculib-v3s\bin"

set PATH=%AndesRoot%;%AndesRootUtils%;%AndesRootCygwin%;%AndesRootCygwin_lib%;%AndesRootGcc%;%PATH%

set "HomeRoot=%cd%"

:menu
cls
echo 请选择一个操作：
echo 1. 编译和下载: 通过SW
echo 2. 仅编译
echo 3. 仅下载: 通过SW
echo 4: 增量编译和下载:通过SW
echo 5: 重启目标板
echo 6: 生成第三方音效内嵌代码
echo Q. 退出

set /p choice=请输入你的选择: 

cd "!HomeRoot!"

if "%choice%"=="" goto menu
if /i "%choice%"=="1" (
    echo 你选择了操作1: 编译和下载:通过SW

	rem 开始编译代码

	rd /s /q cmake_build

	nds_ldsag.exe -t %AndesRootUtils%\nds32_template.txt nds32-ae210p-cm.sag -o nds32-ae210p.ld
	tools\adapt_files_gen.py -i middleware\roboeffect\graphics\ -o main\
	mkdir cmake_build
	cd cmake_build
	cmake ..
	make all -j 8 2>&1 | tee log.log
	python ..\tools\gcc_error_msg.py log.log

	cd "!HomeRoot!"
	
	rem 开始下载代码
	taskkill /f /im ICEman.exe
	
	cd "!AndesRootIce!"
	start "" /b "ICEman.exe" "-b 9900" "-t 9901" "-p 9902" "-A" "-N reset-hold-script.tpl"
	ping 127.0.0.1 -n 3 > nul
	
	cd "!HomeRoot!"
	tools\o26_download_v1.8-4k-crc.exe --image cmake_build\roboeffect_demo.bin --addr 0x0 --port 9900

	taskkill /f /im ICEman.exe


) else if /i "%choice%"=="2" (
    echo 你选择了操作2: 仅编译
    rem 在这里执行操作2的代码

	rd /s /q cmake_build

	nds_ldsag.exe -t %AndesRootUtils%\nds32_template.txt nds32-ae210p-cm.sag -o nds32-ae210p.ld
	tools\adapt_files_gen.py -i middleware\roboeffect\graphics\ -o main\
	mkdir cmake_build
	cd cmake_build
	cmake ..
	make all -j 8 2>&1 | tee log.log
	python ..\tools\gcc_error_msg.py log.log

	cd "!HomeRoot!"

) else if /i "%choice%"=="3" (
    echo 你选择了操作3: 仅下载
    rem 在这里执行操作3的代码

	taskkill /f /im ICEman.exe
	
	set "HomeRoot=%cd%"
	
	cd "!AndesRootIce!"
	start "" /b "ICEman.exe" "-b 9900" "-t 9901" "-p 9902" "-A" "-N reset-hold-script.tpl"
	ping 127.0.0.1 -n 3 > nul
	
	cd "!HomeRoot!"
	tools\o26_download_v1.8-4k-crc.exe --image cmake_build\roboeffect_demo.bin --addr 0x0 --port 9900

	taskkill /f /im ICEman.exe

)else if /i "%choice%"=="4" (
	echo 你选择了操作4: 增量编译和下载:通过SW

	rem 开始编译代码

	set "HomeRoot=%cd%"
	del /s /q cmake_build\roboeffect_demo.bin
	del /s /q cmake_build\roboeffect_demo.mva

	tools\adapt_files_gen.py -i middleware\roboeffect\graphics\ -o main\
	cd cmake_build
	cmake ..
	make all -j 8 2>&1 | tee log.log
	python ..\tools\gcc_error_msg.py log.log

	cd "!HomeRoot!"
	
	rem 开始下载代码
	taskkill /f /im ICEman.exe
	
	set "HomeRoot=%cd%"
	
	cd "!AndesRootIce!"
	start "" /b "ICEman.exe" "-b 9900" "-t 9901" "-p 9902" "-A" "-N reset-hold-script.tpl"
	ping 127.0.0.1 -n 3 > nul
	
	cd "!HomeRoot!"
	tools\o26_download_v1.8-4k-crc.exe --image cmake_build\roboeffect_demo.bin --addr 0x0 --port 9900

	taskkill /f /im ICEman.exe

)else if /i "%choice%"=="5" (
	echo 你选择了操作5: 重启目标板

	rem 开始启动ICEman
	taskkill /f /im ICEman.exe

	set "HomeRoot=%cd%"
	
	cd "!AndesRootIce!"
	start "" /b "ICEman.exe" "-b 9902" "-t 9901" "-p 9903" "-A" "-N reset-hold-script.tpl"
	ping 127.0.0.1 -n 2 > nul
	
	cd "!HomeRoot!"
	nds32le-elf-gdb.exe -x tools\dbg_reset_and_run.txt

	taskkill /f /im ICEman.exe

)else if /i "%choice%"=="6" (
	echo 你选择了操作6: 生成第三方音效内嵌代码
	middleware\roboeffect\third_party_effect\third_party_effects_data_gen.exe -p middleware\roboeffect\third_party_effect\

) else if /i "%choice%"=="Q" (
    echo 退出脚本
    goto :end
) else (
    echo 无效的选择，请重新输入。
    pause
    goto menu
)

pause
goto menu


:end
endlocal
