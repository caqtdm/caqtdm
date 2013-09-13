echo Setting up environment for Qt usage...

qmake plugins.pro CONFIG+=configA
nmake.exe clean
qmake plugins.pro CONFIG+=configA
nmake

qmake plugins.pro CONFIG+=configB
nmake.exe clean
qmake plugins.pro CONFIG+=configB
nmake

qmake plugins.pro CONFIG+=configC
nmake.exe clean
qmake plugins.pro CONFIG+=configC
nmake
