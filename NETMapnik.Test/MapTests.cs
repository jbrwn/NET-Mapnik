using System;
using System.Collections.Generic;
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
            Map m = new Map();
        }

        [TestMethod]
        public void Map_GetAndSetProperties_Success()
        {
            Map m = new Map();
            m.Height = 256;
            m.Width = 256;
            Assert.AreEqual(m.Height, 256U);
            Assert.AreEqual(m.Width, 256U);
        }

        [TestMethod]
        public void Map_GetParamaters()
        {
            Map m = new Map();
            m.Load(@"..\..\data\params.xml");
            Dictionary<string,object> prms = m.Parameters;
            Assert.AreEqual("wat up",(string)prms["words"]);
            Assert.AreEqual(1, (int)prms["num"]);
            Assert.AreEqual(.123, (double)prms["decimal"]);
        }

        [TestMethod]
        public void Map_SetParamaters()
        {
            Map m = new Map();

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
            Map m = new Map();
            string mapString = @"<Map buffer-size=""20""></Map>";
            m.FromString(mapString);
            Assert.AreEqual(m.BufferSize, 20);
        }

        [TestMethod]
        public void Map_GetBackground()
        {
            Map m = new Map();
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
            Map m = new Map();
            m.Background = c;

            Color c2 = m.Background;
            Assert.AreEqual(c2.Hex(), "#ff0000");
        }

    }
}
