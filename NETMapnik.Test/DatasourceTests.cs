using System;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NETMapnik.Test
{
    [TestClass]
    public class DatasourceTests
    {
        [TestMethod]
        public void Datasource_Creation()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Dictionary<string, object> options = new Dictionary<string, object>() 
            { 
                { "type","shape"},
                { "file", @".\data\world_merc.shp" }
            };

            Datasource d = new Datasource(options);

            string expectedType = "vector";
            string expectedEncoding = "utf-8";
            string expectedGeomType = "polygon";
            Dictionary<string, string> expectedFields = new Dictionary<string, string>()
            {
                {"FIPS", "String"},
                {"ISO2", "String"},
                {"ISO3", "String"},
                {"UN", "Number"},
                {"NAME", "String"},
                {"AREA", "Number"},
                {"POP2005", "Number"},
                {"REGION", "Number"},
                {"SUBREGION", "Number"},
                {"LON", "Number"},
                {"LAT", "Number"}
            };
            double[] expectedExtent = new double[] 
            { 
                -20037508.342789248,
                -8283343.6938826973,
                20037508.342789244,
                18365151.363070473 
            };
            
            Dictionary<string, object> p = new Dictionary<string,object>(d.Paramemters());
            CollectionAssert.AreEquivalent(p, options);

            double[] e = d.Extent();
            CollectionAssert.AreEqual(e, expectedExtent);

            IDictionary<string, object> desc = d.Describe();
            Assert.AreEqual(desc["type"], expectedType);
            Assert.AreEqual(desc["encoding"], expectedEncoding);
            Assert.AreEqual(desc["geometry_type"], expectedGeomType);
            CollectionAssert.AreEquivalent((Dictionary<string, string>)desc["fields"], expectedFields);
        }
    }
}
