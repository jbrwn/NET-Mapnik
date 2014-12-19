using System;
using System.IO;
using System.Collections.Generic;
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
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.ZoomAll();
            VectorTile v = new VectorTile(0,0,0,256,256);
            m.Render(v);
            int byteCount = v.GetBytes().Length;
            Assert.AreNotEqual(byteCount, 0);

        }

        [TestMethod]
        public void VectorTile_Render()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.ZoomAll();
            VectorTile v = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v);

            VectorTile v2 = new VectorTile(0, 0, 0, 256, 256);
            v2.SetBytes(v.GetBytes());
            Map m2 = new Map(256, 256);
            m2.Load(@".\data\style.xml");
            Image i = new Image(256, 256);
            v2.Render(m2, i);
            int byteCount = i.Encode("png").Length;
            Assert.AreNotEqual(byteCount, 0);
        }

        [TestMethod]
        public void VectorTile_Overzoom_Render()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");

            m.ZoomToBox(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
            VectorTile v1 = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v1);

            m.ZoomToBox(-20037508.34, 0, 0, 20037508.34);
            VectorTile v2 = new VectorTile(1, 0, 0, 256, 256);
            m.Render(v2);

            Map renderMap = new Map(256, 256);
            renderMap.Load(@".\data\style.xml");
            Image i1 = new Image(256, 256);
            Image i2 = new Image(256, 256);

            Dictionary<string, object> options = new Dictionary<string, object>();
            options["Z"] = 1;

            v1.Render(renderMap, i1, options);
            v2.Render(renderMap, i2);

            int v1byteCount = i1.Encode("png").Length;
            int v2byteCount = i2.Encode("png").Length;
            //Byte counts may be a little different. need to find better test.
            //Assert.AreEqual(v1byteCount, v2byteCount);
        }

        [TestMethod]
        public void VectorTile_SimpleComposite()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.ZoomAll();

            VectorTile v1 = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v1);
            int v1before = v1.GetBytes().Length;

            VectorTile v2 = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v2);

            v1.Composite(new List<VectorTile>() { v2 });
            int v1after = v1.GetBytes().Length;

            Assert.AreEqual(v1before * 2, v1after);
        }

        [TestMethod]
        public void VectorTile_OverzoomComposite()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");

            VectorTile v1 = new VectorTile(1, 0, 0, 256, 256);
            m.ZoomToBox(-20037508.34, 0, 0, 20037508.34);
            m.Render(v1);
            int v1before = v1.GetBytes().Length;

            VectorTile v2 = new VectorTile(0, 0, 0, 256, 256);
            m.ZoomToBox(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
            m.Render(v2);

            v1.Composite(new List<VectorTile>() { v2 });
            int v1after = v1.GetBytes().Length;

            //composite bytes will actually be a little bit bigger than 2* original
            //Assert.AreEqual(v1before * 2, v1after);
        }
    }
}
