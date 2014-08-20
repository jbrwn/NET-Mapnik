using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class VectorTileTests
    {
        [TestMethod]
        public void VectorTile_Creation()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.LoadMap(@"..\..\data\test.xml");
            m.ZoomAll();
            VectorTile v = new VectorTile();
            m.Render(v);
            Assert.AreNotEqual(v.GetBytes().Length, 0);

        }

        [TestMethod]
        public void VectorTile_Render()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.LoadMap(@"..\..\data\test.xml");
            m.ZoomAll();
            VectorTile v = new VectorTile();
            m.Render(v);

            Map m2 = new Map();
            m2.Width = 256;
            m2.Height = 256;
            Image i = new Image(256, 256);
            v.Render(m2, i);
            i.Save(@"d:\tile.png", "png");
        }
    }
}
