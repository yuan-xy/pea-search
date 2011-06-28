@echo off
if "%1" == "clean" goto do_clean

:do_build
	cd Release
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release .. 
	nmake
	cd ..\..\firebreath-1.4\Release
	cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
	nmake
	cd ..\..\gigaso\Release
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
	goto do_build
:end

