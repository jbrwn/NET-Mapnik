using System;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

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

            Dictionary<string, object> p = new Dictionary<string, object>(d.Paramemters());
            CollectionAssert.AreEquivalent(p, options);

            double[] e = d.Extent();
            CollectionAssert.AreEqual(e, expectedExtent);

            IDictionary<string, object> desc = d.Describe();
            Assert.AreEqual(desc["type"], expectedType);
            Assert.AreEqual(desc["encoding"], expectedEncoding);
            Assert.AreEqual(desc["geometry_type"], expectedGeomType);
            CollectionAssert.AreEquivalent((Dictionary<string, string>)desc["fields"], expectedFields);
        }

        [TestMethod]
        public void MemoryDatasource_Init()
        {
            var options = new Dictionary<string, object>();
            MemoryDatasource d = new MemoryDatasource(options);

            IDictionary<string, object> prms = d.Paramemters();
            Assert.AreEqual(1, prms.Count);
            Assert.AreEqual("memory", prms["type"]);

            IDictionary<string, object> desc = d.Describe();
            Assert.AreEqual("vector", desc["type"]);
            Assert.AreEqual("utf-8", desc["encoding"]);
            Assert.AreEqual("collection", desc["geometry_type"]);
            var fields = (Dictionary<string, string>)desc["fields"];
            Assert.AreEqual(0, fields.Count);
        }


        [TestMethod]
        public void MemoryDatasource_Add()
        {
            var options = new Dictionary<string, object>();
            MemoryDatasource d = new MemoryDatasource(options);
            d.Add(1, 2);
            Featureset fs = d.Featureset();
            Feature f = fs.Next();
            string expected = @"{""type"":""Feature"",""id"":1,""geometry"":{""type"":""Point"",""coordinates"":[1,2]},""properties"":{}}";
            string json = f.ToJSON();
            Assert.AreEqual(expected, json);
            // should only have 1 feature
            f = fs.Next();
            Assert.IsNull(f);
        }

        [TestMethod]
        public void MemoryDatasource_AddWithProps()
        {
            var options = new Dictionary<string, object>();
            MemoryDatasource d = new MemoryDatasource(options);
            var props = new Dictionary<string, object>()
            {
                {"string_field", "text" },
                {"int_field", 1 },
                {"double_field", .124 }
            };
            d.Add(1, 2, props);
            Featureset fs = d.Featureset();
            Feature f = fs.Next();
            string expected = @"{""type"":""Feature"",""id"":1,""geometry"":{""type"":""Point"",""coordinates"":[1,2]},""properties"":{""double_field"":0.124,""int_field"":1,""string_field"":""text""}}";
            string json = f.ToJSON();
            Assert.AreEqual(expected, json);
            // should only have 1 feature
            f = fs.Next();
            Assert.IsNull(f);
        }
    }
}
