using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace NETMapnik.Test
{
    [TestClass]
    public class FeatureTests
    {
        [TestMethod]
        public void Featureset_Creation()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Dictionary<string, object> options = new Dictionary<string, object>()
            {
                { "type","shape"},
                { "file", @".\data\world_merc.shp" }
            };

            Datasource d = new Datasource(options);
            Featureset fs = d.Featureset();
            Feature feature = fs.Next();
            Dictionary<string, object> attr = new Dictionary<string, object>(feature.Attributes());
            Dictionary<string, object> expectedAttr = new Dictionary<string, object>()
            {
                {"AREA", 44L},
                {"FIPS", "AC"},
                {"ISO2", "AG"},
                {"ISO3", "ATG"},
                {"LAT", 17.078},
                {"LON", -61.783},
                {"NAME", "Antigua and Barbuda"},
                {"POP2005", 83039L},
                {"REGION", 19L},
                {"SUBREGION", 29L},
                {"UN", 28L}
            };
            CollectionAssert.AreEquivalent(attr, expectedAttr);

            int count = 1;
            while (fs.Next() != null)
            {
                count++;
            }
            Assert.AreEqual(count, 245);
        }

        [TestMethod]
        public void Feature_ToJSON()
        {
            string input = @"{
                type: ""Feature"",
                properties: {},
                geometry: {
                    type: ""Polygon"",
                    coordinates: [[[1,1],[1,2],[2,2],[2,1],[1,1]]]
                }
            }";
            JObject expected = JObject.Parse(input);

            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "csv.input"));
            Dictionary<string, object> options = new Dictionary<string, object>()
            {
                { "type","csv"},
                { "inline", "geojson\n'" + expected["geometry"].ToString(Formatting.None) + "'" }
            };
            Datasource ds = new Datasource(options);

            Feature f = ds.Featureset().Next();
            JObject feature = JObject.Parse(f.ToJSON());

            Assert.AreEqual(expected["type"], feature["type"]);
            Assert.AreEqual(((JObject)expected["properties"]).Count, ((JObject)feature["properties"]).Count);
            Assert.AreEqual(expected["geometry"]["type"], feature["geometry"]["type"]);

            JArray coords = (JArray)expected["geometry"]["coordinates"][0];
            for (int i = 0; i < coords.Count; i++)
            {
                JArray coord1 = (JArray)expected["geometry"]["coordinates"][0][i];
                JArray coord2 = (JArray)feature["geometry"]["coordinates"][0][i];
                CollectionAssert.AreEquivalent(coord1.ToObject<List<double>>(), coord2.ToObject<List<double>>());
            }
        }

        [TestMethod]
        public void Feature_FromJSON()
        {
            string input = @"{
                type: ""Feature"",
                properties: {},
                geometry: {
                    type: ""Polygon"",
                    coordinates: [[[1,1],[1,2],[2,2],[2,1],[1,1]]]
                }
            }";
            JObject expected = JObject.Parse(input);
            Feature f = Feature.FromJSON(expected.ToString(Formatting.None));
            JObject feature = JObject.Parse(f.ToJSON());

            Assert.AreEqual(expected["type"], feature["type"]);
            Assert.AreEqual(((JObject)expected["properties"]).Count, ((JObject)feature["properties"]).Count);
            Assert.AreEqual(expected["geometry"]["type"], feature["geometry"]["type"]);

            JArray coords = (JArray)expected["geometry"]["coordinates"][0];
            for (int i = 0; i < coords.Count; i++)
            {
                JArray coord1 = (JArray)expected["geometry"]["coordinates"][0][i];
                JArray coord2 = (JArray)feature["geometry"]["coordinates"][0][i];
                CollectionAssert.AreEquivalent(coord1.ToObject<List<double>>(), coord2.ToObject<List<double>>());
            }
        }
    }
}
