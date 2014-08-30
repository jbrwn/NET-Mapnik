NET-Mapnik
==========

.NET bindings for [mapnik](https://github.com/mapnik/mapnik). 

Usage
-------
```
using NETMapnik;

    public class MapTests
    {
        public void CreateMap()
        {
            // Register Datasources
            DatasourceCache.RegisterDatasources(@"mapnik\input");

            // Register Fonts
            FreetypeEngine.RegisterFonts(@"mapnik\fonts", false);
            
            // Create map object
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.LoadMap(@"map.xml");
            m.ZoomToBox(-20037508.3428, -20037508.3428, 20037508.3428, 20037508.3428);
            
            // Render an image
            Image i = new Image(256, 256);
            m.Render(i);
            m.Save("tile.png", "png");
            
            // Render a UTFGrid
            Grid g = new Grid(256, 256);
            List<string> gridFields = new List<string>() {"FIPS"};
            m.Render(g, 0, gridFields);
            Dictionary<string, object> UTFGridDict = g.Encode("utf", true, 4);
            
            // Render a vector tile
            VectorTile v = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v);
            byte[] vBytes = v.GetBytes();
        }
    }
```

Build Instrucitons
------------------

Requirements:
  - Visual studio 2013
  - [Build dependencies](lib/readme.md)

Build:
  - Open NETMapnik.sln in Visual Studio 2013
  - Set build to release
  - Build solution
  - Binary and all mapnik dependencies will be copied to .\Release
