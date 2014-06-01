NET-Mapnik
==========

Minimal .NET bindings for mapnik.  The following features are supported:

  - Create map object
  - Load mapnik xml
  - Render to file
  - Render to bytes
  - Render to UTFGrid

Build Instrucitons
------------------

I'm currently only building from visual studio 2012.

Requirements:
  - Visual studio 2012 IDE
  - Visual studio 2010 platform toolset (mapnik 2.2.0 sdk targets MSVC2010)
  - [Build dependencies](lib/README.md)

Build:
  - Open NETMapnik.sln in Visual Studio
  - Build solution
  - Binary and all mapnik dependencies will be copied to .\Release

Example
-------

```
using NETMapnik;

    public class MapTests
    {
        public void CreateMap()
        {
            Map m = new Map();
            m.LoadMap(@"map.xml");
            m.ZoomToBox(-20037508.3428, -20037508.3428, 20037508.3428, 20037508.3428);
            m.SaveToFile(@"map.png", "png");
        }
    }
```
