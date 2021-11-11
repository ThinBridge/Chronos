all:
	msbuild /p:Configuration=D64_CSG /p:Platform=Win32

clean:
	msbuild /t:clean

test: all
	.\D32\ChronosN.exe
