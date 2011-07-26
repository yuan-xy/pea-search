@echo off
if "%1" == "clean" goto do_clean
if "%1" == "plugin" goto do_build_plugin
if "%1" == "gigaso" goto do_build

:do_build
	cd Release
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..  
	nmake
	cd ..
	if "%1" == "gigaso" goto end	
:do_build_plugin
	cd ..\firebreath-1.4\Release
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
	nmake
	cd ..\..\gigaso
	if "%1" == "plugin" goto end	
	cd Release
	cpack
	cd ..
	goto end

:do_clean
	rm -rf Release
	mkdir Release 
	cd ..\firebreath-1.4
	rm -rf Release
	mkdir Release
	cd ..\..\gigaso
:end

