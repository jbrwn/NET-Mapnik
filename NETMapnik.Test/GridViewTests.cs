using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;

namespace NETMapnik.Test
{
    [TestClass]
    public class GridViewTests
    {
        [TestMethod]
        public void GridView_Init()
        {
            Grid g = new Grid(256, 256);
            GridView v = g.View(0, 0, 128, 128);
            Assert.IsTrue(v.IsSolid());
            long pixel = v.GetPixel(0, 0);
            Assert.AreEqual(Grid.BaseMask, pixel);
            Assert.AreEqual(128, v.Width());
            Assert.AreEqual(128, v.Height());
        }

        [TestMethod]
        public void GridView_IsSolid()
        {
            Map m = new Map(256, 256);
            m.AddLayer(new Layer("test"));
            m.ZoomAll();
            Grid g = new Grid(256, 256);
            m.Render(g);
            GridView v1 = g.View(0, 0, 256, 256);
            Assert.IsTrue(v1.IsSolid());

            m.Clear();
            g.Clear();
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            m.Render(g);
            GridView v2 = g.View(0, 0, 256, 256);
            Assert.IsFalse(v2.IsSolid());
        }

        [TestMethod]
        public void GridView_GetPixel()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Map m = new Map(256, 256);
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            Grid g = new Grid(256, 256);
            m.Render(g);
            GridView v = g.View(0, 0, 256, 256);
            long pixel = v.GetPixel(25, 100);
            Assert.AreEqual(207, pixel);
        }

        [TestMethod]
        public void GridView_Encode()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Map m = new Map(256, 256);
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            Grid g = new Grid(256, 256);
            var options = new Dictionary<string, object>()
            {
                {"Fields", new List<string>() { "FIPS" } },
                {"Layer", "world" }
            };

            m.Render(g, options);
            Dictionary<string, object> UTFGridDict = g.Encode();

            Assert.AreEqual(UTFGridDict.Keys.Count, 3);

            //Test for keys
            List<string> keyList = (List<string>)UTFGridDict["keys"];
            Assert.AreNotEqual(keyList.Count, 0);

            //Test for data
            Dictionary<string, object> dataDict = (Dictionary<string, object>)UTFGridDict["data"];
            Assert.AreNotEqual(dataDict.Count, 0);

            //data count should equal keys + 1
            Assert.AreEqual(keyList.Count, dataDict.Count + 1);

            //Test grid dimensions
            List<string> grid = (List<string>)UTFGridDict["grid"];
            Assert.AreEqual(64, grid.Count);
            Assert.AreEqual(64, grid[0].Length);
        }
    }
}
