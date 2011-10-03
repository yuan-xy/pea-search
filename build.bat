@echo off
if "%1" == "clean" goto do_clean
if "%1" == "gigaso" goto do_gigaso
if "%1" == "gigaso_install" goto do_build
if "%1" == "install" goto do_install
if "%1" == "sign" goto do_sign

:do_build
	cd ReleaseWin
	cmake -G "NMake Makefiles" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..  
	cmake -G "NMake Makefiles" -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release ..  
	cd ..
:do_gigaso
	cd ReleaseWin
	nmake
	cd ..
	if "%1" == "gigaso" goto end	
	if "%1" == "gigaso_install" goto do_install	
:do_install
	cd ReleaseWin
	signtool sign /v /f ../cmake/wandouss-exe.pfx browser/peasrch.exe
	signtool sign /v /f ../cmake/wandouss-exe.pfx filesearch/peadaemo.exe
	signtool sign /v /f ../cmake/wandouss-exe.pfx filesearch/pealn.exe
	cpack
	cd ..
:do_sign
	for %%f in (ReleaseWin/PeaSearch*.exe) do signtool sign /v /f cmake/wandouss-exe.pfx ReleaseWin/%%f 
	goto end

:do_clean
	rmdir /S /Q ReleaseWin
	mkdir ReleaseWin
:end

