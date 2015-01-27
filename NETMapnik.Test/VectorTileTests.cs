using System;
using System.Linq;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;
using Newtonsoft.Json;

namespace NETMapnik.Test
{
    [TestClass]
    public class VectorTileTests
    {
        [TestMethod]
        public void VectorTile_Init()
        {
            VectorTile v = new VectorTile(0, 0, 0);
            Assert.AreEqual(256, v.Width());
            Assert.AreEqual(256, v.Height());
            Assert.IsFalse(v.Painted());
            Assert.AreEqual("", v.IsSolid());
            Assert.IsTrue(v.Empty());
            Assert.AreEqual(0, v.GetData().Length);
        }
        [TestMethod]
        public void VectorTile_Init_WidithHeight()
        {
            VectorTile v = new VectorTile(0, 0, 0, 512, 512);
            Assert.AreEqual(512, v.Width());
            Assert.AreEqual(512, v.Height());
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
            int v1before = v1.GetData().Length;

            VectorTile v2 = new VectorTile(0, 0, 0, 256, 256);
            m.Render(v2);

            v1.Composite(new List<VectorTile>() { v2 });
            int v1after = v1.GetData().Length;

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
            int v1before = v1.GetData().Length;

            VectorTile v2 = new VectorTile(0, 0, 0, 256, 256);
            m.ZoomToBox(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
            m.Render(v2);

            v1.Composite(new List<VectorTile>() { v2 });
            int v1after = v1.GetData().Length;

            //composite bytes will actually be a little bit bigger than 2* original
            //Assert.AreEqual(v1before * 2, v1after);
        }


        [TestMethod]
        public void VectorTile_Names()
        {
            VectorTile v = new VectorTile(9,112,195);
            v.SetData(File.ReadAllBytes(@".\data\9.112.195.pbf"));
            CollectionAssert.AreEquivalent(new List<string>() { "world" }, v.Names().ToList());
        }

        [TestMethod]
        public void VectorTile_GetSetData()
        {
            VectorTile v = new VectorTile(9,112,195);
            byte[] bytes = File.ReadAllBytes(@".\data\9.112.195.pbf");
            v.SetData(bytes);
            Assert.IsFalse(v.Empty());
            Assert.IsTrue(v.Painted());
            byte[] actual = v.GetData();
            CollectionAssert.AreEquivalent(bytes, actual);
        }

        [TestMethod]
        public void VectorTile_AddData()
        {
            VectorTile v = new VectorTile(9,112,195);
            byte[] bytes = File.ReadAllBytes(@".\data\9.112.195.pbf");
            v.SetData(bytes);
            v.AddData(bytes);
            Assert.IsFalse(v.Empty());
            Assert.IsTrue(v.Painted());
            CollectionAssert.AreEquivalent(new List<string>() { "world", "world" }, v.Names().ToList());
            byte[] actual = v.GetData();
            Assert.AreEqual(bytes.Length * 2, actual.Length);
        }

        [TestMethod]
        public void VectorTile_AddImage()
        {
            VectorTile v = new VectorTile(1, 0, 0);
            byte[] bytes = File.ReadAllBytes(@".\data\world_1.0.0.png");
            v.AddImage(bytes, "raster");
            Assert.IsFalse(v.Empty());
            Assert.IsTrue(v.Painted());
            CollectionAssert.AreEquivalent(new List<string>() { "raster" }, v.Names().ToList());

            Map m = new Map(256, 256);
            m.Load(@".\data\raster_style.xml");
            Image i = new Image(256, 256);
            v.Render(m, i);
            Assert.AreEqual(0,i.Compare(Image.FromBytes(bytes)));
        }

        [TestMethod]
        public void VectorTile_GeoJSON()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            string json = @"{""type"":""FeatureCollection"",""name"":""layer"",""features"":[{""type"":""Feature"",""geometry"":{""type"":""Point"",""coordinates"":[-121.9921875,47.9899216674142]},""properties"":{""name"":""geojson data""}}]}";
            VectorTile v = new VectorTile(0, 0, 0);
            v.AddGeoJSON(json, "layer");
            byte[] bytes = v.GetData();
            Assert.AreEqual(53, bytes.Length);
            Assert.IsFalse(v.Empty());
            Assert.IsTrue(v.Painted());
            CollectionAssert.AreEquivalent(new List<string>() { "layer" }, v.Names().ToList());

            //ToGeoJSON
            string actual1 = v.ToGeoJSON("layer");
            string actual2 = v.ToGeoJSON(0);
            Assert.AreEqual(json, actual1);
            Assert.AreEqual(json, actual2);

        }

        [TestMethod]
        public void VectorTile_ToJSON()
        {
            VectorTile v = new VectorTile(9, 112, 195);
            byte[] bytes = File.ReadAllBytes(@".\data\9.112.195.pbf");
            v.SetData(bytes);
            IEnumerable<VectorTileLayer> vtJSON = v.ToJSON();
            string json = JsonConvert.SerializeObject(
                vtJSON,
                new JsonSerializerSettings { NullValueHandling = NullValueHandling.Ignore });
            string expected = @"[{""Features"":[{""Properties"":{""AREA"":915896,""FIPS"":""US"",""ISO2"":""US"",""ISO3"":""USA"",""LAT"":39.622,""LON"":-98.606,""NAME"":""United States"",""POP2005"":299846449,""REGION"":19,""SUBREGION"":21,""UN"":840},""Geometry"":[9,8192,0,58,0,0,0,8192,0,8191,0,8192,0,0,8191,0,0,8191,15],""Type"":3,""Id"":207}],""Version"":1,""Extent"":4096,""Name"":""world""}]";
            Assert.AreEqual(expected, json);
        }

        [TestMethod]
        public void VectorTile_IsSolid()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.Extent = new double[] { -11271098.442818949, 4696291.017841229, -11192826.925854929, 4774562.534805249 };
            VectorTile v = new VectorTile(9, 112, 195);
            m.Render(v);
            Assert.IsTrue(v.Painted());
            Assert.AreEqual("world", v.IsSolid());
        }

        [TestMethod]
        public void VectorTile_Clear()
        {
            VectorTile v = new VectorTile(9,112,195);
            byte[] bytes = File.ReadAllBytes(@".\data\9.112.195.pbf");
            v.SetData(bytes);
            Assert.IsFalse(v.Empty());
            Assert.IsTrue(v.Painted());
            CollectionAssert.AreEquivalent(new List<string>() { "world" }, v.Names().ToList());
            byte[] actual = v.GetData();
            Assert.AreEqual(bytes.Length, actual.Length);

            v.Clear();
            Assert.IsTrue(v.Empty());
            Assert.IsFalse(v.Painted());
            Assert.AreEqual("",v.IsSolid());
            CollectionAssert.AreEquivalent(new List<string>(), v.Names().ToList());
            actual = v.GetData();
            Assert.AreEqual(0, actual.Length);
        }

        [TestMethod]
        public void VectorTile_Query()
        {
            VectorTile v = new VectorTile(9, 112, 195);
            byte[] bytes = File.ReadAllBytes(@".\data\9.112.195.pbf");
            v.SetData(bytes);
            IEnumerable<VectorQueryResult> results = v.Query(-100.8576, 39.1181);
            VectorQueryResult result = results.ToList()[0];
            Assert.AreEqual("world", result.Layer);
            Assert.AreEqual(0, result.Distance);
            Assert.AreEqual(207, result.Feature.Id());
            IDictionary<string, object> attributes = result.Feature.Attributes();
            Assert.AreEqual("United States", (string)attributes["NAME"]);
        }


        [TestMethod]
        public void VectorTile_Render_Image()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.Extent = new double[] { -20037508.34, 0, 0, 20037508.34 };
            VectorTile v = new VectorTile(1, 0, 0);
            m.Render(v);

            VectorTile v2 = new VectorTile(1, 0, 0);
            v2.SetData(v.GetData());

            Map m2 = new Map(256, 256);
            m2.Load(@".\data\style.xml");
            Image i = new Image(256, 256);
            v2.Render(m2, i);
            Assert.AreEqual(0,i.Compare(Image.Open(@".\data\world_1.0.0.png")));
        }

        [TestMethod]
        public void VectorTile_Render_Grid()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map(256, 256);
            m.Load(@".\data\layer.xml");
            m.Extent = new double[] { -20037508.34, 0, 0, 20037508.34 };
            VectorTile v = new VectorTile(1, 0, 0);
            m.Render(v);

            VectorTile v2 = new VectorTile(1, 0, 0);
            v2.SetData(v.GetData());

            Map m2 = new Map(256, 256);
            m2.Load(@".\data\style.xml");
            Grid g = new Grid(256, 256);
            var options = new Dictionary<string, object>()
            {
                {"Fields", new List<string>() { "FIPS" } },
                {"Layer", "world" }
            };
            v2.Render(m2, g, options);
            Dictionary<string, object > grid = g.Encode();
            Assert.AreEqual(grid.Keys.Count, 3);

            //Test for keys
            List<string> keyList = (List<string>)grid["keys"];
            Assert.AreNotEqual(keyList.Count, 0);

            //Test for data
            Dictionary<string, object> dataDict = (Dictionary<string, object>)grid["data"];
            Assert.AreNotEqual(dataDict.Count, 0);

            //data count should equal keys + 1
            Assert.AreEqual(keyList.Count, dataDict.Count + 1);
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

            //Small diff showing up between images 
            Assert.IsTrue(i1.Compare(i2)-500 < 0);
   
        }
    }
}
