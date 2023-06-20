## MSVC使用笔记



### CL

编译C

```
cl file1.c file2.c file3.c
cl file1.c file2.c file3.c /link /out:program1.exe
```

编译C++

```c
cl /EHsc file1.cpp file2.cpp file3.cpp
cl /EHsc file1.cpp file2.cpp file3.cpp /link /out:program1.exe
```



```bat
cl 
/nologo 
/Zi 
/MD 
/utf-8 
/I..\.. 
/I..\..\backends 
/I "C:\Program Files (x86)\Windows Kits\10\Include\um" 
/I "C:\Program Files (x86)\Windows Kits\10\Include\shared"
/I "Include" 
/D UNICODE 
/D _UNICODE 
main.cpp 
..\..\backends\imgui_impl_dx11.cpp 
..\..\backends\imgui_impl_win32.cpp 
..\..\imgui*.cpp 
/FeDebug/example_win32_directx11.exe 
/FoDebug/ 
/link 
/LIBPATH:"/Lib/x86" d3d11.lib d3dcompiler.lib
```





### MSBuild

[演练：使用 MSBuild 创建 Visual Studio C++ 项目 | Microsoft Learn](https://learn.microsoft.com/zh-cn/cpp/build/walkthrough-using-msbuild-to-create-a-visual-cpp-project?view=msvc-170)