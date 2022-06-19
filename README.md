## Qt Quick overlay for Skyrim Special Edition.  

This is a proof of concept and not a finished application.  
Wasn't tested too much.  
There is also no input handling.

Developed with Qt version 6.3.0.

To compile, run CMake:
```
cmake --preset vs2022-windows -DCMAKE_PREFIX_PATH="<PATH_TO_QT_6.3.0>/msvc2019_64/lib/cmake" -DSKSE_SUPPORT_XBYAK=ON
cmake --build build --config Release
```

After you compile it, put the `qml` folder to Skyrims root directory.  
Put the .dll to the `Data/SKSE/Plugins` folder.  
Run `windeployqt . --qmlpath <PATH_TO_SKYRIM>/qml` from the same folder where the mods dll is.

Put the resulting dll files and folders (excluding the mod itself) to Skyrims root directory.  

