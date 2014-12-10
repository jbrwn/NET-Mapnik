NET-Mapnik
==========

.NET bindings for [mapnik](https://github.com/mapnik/mapnik).

For example usage take a look at the [tests](NETMapnik.Test).

Requirements
------------

Visual C++ 2015 Redistributable

https://github.com/kernelsanders/vcredist/raw/master/2015Preview/vcredist_x86.exe
https://github.com/kernelsanders/vcredist/raw/master/2015Preview/vcredist_x64.exe

Install
-------
Install the nuget package:

[mapnik (x86)](https://www.nuget.org/packages/mapnik)
```
PM> Install-Package mapnik
```

[mapnik (x64)](https://www.nuget.org/packages/mapnik.x64)
```
PM> Install-Package mapnik.x64
```

Build Instrucitons
------------------

Requirements:
  - Visual Studio 2015
  - [Build dependencies](lib/readme.md)

Build:
  - Open NETMapnik.sln in Visual Studio 2015
  - Set build to release
  - Build solution
