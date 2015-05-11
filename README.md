NET-Mapnik
==========
[![Build status](https://ci.appveyor.com/api/projects/status/grs29ai58nr6i34s?svg=true)](https://ci.appveyor.com/project/jbrwn/net-mapnik)

.NET bindings for [mapnik](https://github.com/mapnik/mapnik).

For example usage take a look at the [tests](NETMapnik.Test).

API
---
NET-Mapnik is a port of the [nod-mapnik](https://github.com/mapnik/node-mapnik) bindings for mapnik and conforms to the same API.

Requirements
------------

Visual C++ 2015 Redistributable

https://github.com/jbrwn/vcredist/raw/master/2015RC/vcredist_x86.exe
https://github.com/jbrwn/vcredist/raw/master/2015RC/vcredist_x64.exe

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
  - Visual Studio 2015 RC
  - [Build dependencies](lib/readme.md)

Build:
```MSBuild NETMapnik.sln```
