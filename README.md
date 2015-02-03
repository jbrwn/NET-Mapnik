NET-Mapnik
==========
[![Build status](https://ci.appveyor.com/api/projects/status/grs29ai58nr6i34s?svg=true)](https://ci.appveyor.com/project/jbrwn/net-mapnik)

.NET bindings for [mapnik](https://github.com/mapnik/mapnik).

For example usage take a look at the [tests](NETMapnik.Test).

Requirements
------------

Visual C++ 2015 Redistributable

https://github.com/jbrwn/vcredist/raw/master/2015Preview/vcredist_x86.exe
https://github.com/jbrwn/vcredist/raw/master/2015Preview/vcredist_x64.exe

Install
-------
Install the nuget package:

```
PM> Install-Package mapnik
```
or
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
