using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class UTFGridTests
    {
        [TestMethod]
        public void UTFGrid_Creation()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.Load(@"..\..\data\test.xml");
            m.ZoomAll();
            Grid g = new Grid(256, 256);
            List<string> gridFields = new List<string>() {"FIPS"};
            m.Render(g, 0, gridFields);
            Dictionary<string, object> UTFGridDict = g.Encode("utf", true, 4);

            Assert.AreEqual(UTFGridDict.Keys.Count, 3);

            //Test for keys
            List<string> keyList= (List<string>)UTFGridDict["keys"];
            Assert.AreNotEqual(keyList.Count, 0);

            //Test for data
            Dictionary<string, object> dataDict = (Dictionary<string, object>)UTFGridDict["data"];
            Assert.AreNotEqual(dataDict.Count, 0);
            
            //data count should equal keys + 1
            Assert.AreEqual(keyList.Count, dataDict.Count + 1);
        }
    }
}
