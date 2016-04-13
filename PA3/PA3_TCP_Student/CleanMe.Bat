rmdir /S /Q PA3_Client\Debug
rmdir /S /Q PA3_Client\ipch
rmdir /S /Q PA3_Client\Release
del /F /S PA3_Client\*.sdf
del /F /S /A:H PA3_Client\*.suo
rmdir /S /Q PA3_Client\PA3_Client\Debug
rmdir /S /Q PA3_Client\PA3_Client\Release

rmdir /S /Q PA3_Server\Debug
rmdir /S /Q PA3_Server\ipch
rmdir /S /Q PA3_Server\Release
del /F /S PA3_Server\*.sdf
del /F /S /A:H PA3_Server\*.suo
rmdir /S /Q PA3_Server\PA3_Server\Debug
rmdir /S /Q PA3_Server\PA3_Server\Release
