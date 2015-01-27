using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class GridTests
    {
        [TestMethod]
        public void Grid_Encode()
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
        }

        [TestMethod]
        public void Grid_Encode_Resolution()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Map m = new Map(256, 256);
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            Grid g1 = new Grid(256, 256);
            m.Render(g1);
            List<string> data = (List<string>) g1.Encode()["grid"];
            Assert.AreEqual(64, data.Count());
            Assert.AreEqual(64, data[0].Length);

            Grid g2 = new Grid(256, 256);
            m.Render(g2);
            var options = new Dictionary<string, object>()
            {
                { "Resolution", 1U } 
            };
            data = (List<string>)g1.Encode(options)["grid"];
            Assert.AreEqual(256, data.Count());
            Assert.AreEqual(256, data[0].Length);
        }

        [TestMethod]
        public void Grid_WidthHeight()
        {
            Grid g = new Grid(256, 256);
            Assert.AreEqual(256, g.Width());
            Assert.AreEqual(256, g.Height());
        }

        [TestMethod]
        public void Grid_Key()
        {
            Grid g = new Grid(256, 256);
            Assert.AreEqual("__id__", g.Key);
            g.Key = "key";
            Assert.AreEqual("key", g.Key);
        }

        [TestMethod]
        public void Grid_Painted()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Grid g = new Grid(256, 256);
            Assert.IsFalse(g.Painted());

            Map m = new Map(256, 256);
            Layer l = new Layer("test");
            m.AddLayer(l);
            m.Render(g);
            Assert.IsFalse(g.Painted());

            m.Clear();
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            var options = new Dictionary<string, object>()
            {
                {"Fields", new List<string>() { "FIPS" } },
                {"Layer", "world" }
            };
            m.Render(g);
            Assert.IsTrue(g.Painted());
        }

        [TestMethod]
        public void Grid_Fields()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Map m = new Map(256, 256);
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            Grid g = new Grid(256, 256);
            List<string> expected = new List<string>() { "FIPS" };
            var options = new Dictionary<string, object>()
            {
                {"Fields", expected }
            };
            m.Render(g, options);

            CollectionAssert.AreEquivalent(expected, g.Fields().ToList());
        }
    }
}
