using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class MapTests
    {
        [TestMethod]
        public void Map_Creation_Success()
        {
            Map m = new Map(256,256);
        }

        [TestMethod]
        public void Map_GetAndSetProperties_Success()
        {
            Map m = new Map(0,0);
            m.Height = 256;
            m.Width = 256;
            Assert.AreEqual(m.Height, 256);
            Assert.AreEqual(m.Width, 256);

            m.Resize(512, 512);
            Assert.AreEqual(m.Height, 512);
            Assert.AreEqual(m.Width, 512);

            m.Width = 256;
            m.Height = 256;
            Assert.AreEqual(m.Height, 256);
            Assert.AreEqual(m.Width, 256);

            // Aspect fix mode
            Assert.AreEqual(m.AspectFixMode, AspectFixMode.ASPECT_GROW_BBOX);

            double[] world = { -180, -85, 180, 85 };
            m.Extent = world;
            // will have been made square
            CollectionAssert.AreEqual(m.Extent, new double[] { -180, -180, 180, 180 });

            // now try again after disabling the "fixing"
            m.AspectFixMode = AspectFixMode.ASPECT_RESPECT;
            Assert.AreEqual(m.AspectFixMode, AspectFixMode.ASPECT_RESPECT);
            m.Extent = world;
            CollectionAssert.AreEqual(m.Extent, world);
        }

        [TestMethod]
        public void Map_GetParamaters()
        {
            Map m = new Map(256,256);
            m.Load(@".\data\params.xml");
            Dictionary<string, object> prms = m.Parameters;
            Assert.AreEqual("wat up", (string)prms["words"]);
            Assert.AreEqual(1, (int)prms["num"]);
            Assert.AreEqual(.123, (double)prms["decimal"]);
        }

        [TestMethod]
        public void Map_SetParamaters()
        {
            Map m = new Map(256,256);

            Dictionary<string, object> prms = new Dictionary<string, object>()
            {
                {"words", "wat up"},
                {"num", 1},
                {"decimal", .123}
            };

            m.Parameters = prms;

            Dictionary<string, object> rtnPrms = m.Parameters;
            Assert.AreEqual("wat up", (string)rtnPrms["words"]);
            Assert.AreEqual(1, (int)rtnPrms["num"]);
            Assert.AreEqual(.123, (double)rtnPrms["decimal"]);
        }

        [TestMethod]
        public void Map_FromString()
        {
            Map m = new Map(256,256);
            string mapString = @"<Map buffer-size=""20""></Map>";
            m.FromString(mapString);
            Assert.AreEqual(m.BufferSize, 20);
        }

        [TestMethod]
        public void Map_GetBackground()
        {
            Map m = new Map(256, 256);
            Color c = m.Background;
            Assert.IsNull(c);

            string mapString = @"<Map background-color=""red""></Map>";
            m.FromString(mapString);
            c = m.Background;
            Assert.AreEqual(c.Hex(), "#ff0000");
        }

        [TestMethod]
        public void Map_SetBackground()
        {
            Color c = new Color("red");
            Map m = new Map(256, 256);
            m.Background = c;

            Color c2 = m.Background;
            Assert.AreEqual(c2.Hex(), "#ff0000");
        }

        [TestMethod]
        public void Map_GetSetLayer()
        {
            Layer l = new Layer("layer", "+init=epsg:4326");
            Map m = new Map(256, 256);
            m.AddLayer(l);

            IEnumerable<Layer> layers = m.Layers();
            Assert.AreEqual(layers.Count(), 1);

            Layer l1 = m.GetLayer("layer");
            Assert.AreEqual(l1.Name, "layer");
        }

        [TestMethod]
        public void Map_Clone()
        {
            Map m = new Map(256, 256);
            m.AddLayer(new Layer("layer"));
            Map m2 = m.Clone();
            Assert.AreEqual(m.Layers().Count(), 1);
            Assert.AreEqual(m2.Layers().Count(), 1);

            m.Clear();
            Assert.AreEqual(m.Layers().Count(), 0);
            Assert.AreEqual(m2.Layers().Count(), 1);
        }

        [TestMethod]
        public void Map_ToXML()
        {
            string mapString = @"<?xml version=""1.0"" encoding=""utf-8""?>" 
                + (char)10 
                + @"<Map srs=""+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs""/>"
                + (char)10;
            Map m = new Map(256, 256);
            m.FromString(mapString);
            string xml = m.ToXML();
            Assert.AreEqual(mapString, xml);
        }
    }
}

