using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace NETMapnik.Test
{
    [TestClass]
    public class LayerTest
    {
        [TestMethod]
        public void Layer_Creation()
        {
            Layer l = new Layer("layer", "+init=epsg:4326");
            
            Assert.AreEqual(l.Name, "layer");
            Assert.AreEqual(l.SRS, "+init=epsg:4326");
            CollectionAssert.AreEquivalent(l.Styles.ToList(), new List<string>());
            Assert.IsNull(l.Datasource);
        }

        [TestMethod]
        public void Layer_Describe()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Dictionary<string, object> options = new Dictionary<string, object>() 
            { 
                { "type","shape"},
                { "file", @".\data\world_merc.shp" }
            };
            Datasource ds = new Datasource(options);

            Layer l = new Layer("layer", "+init=epsg:4326");
            l.Datasource = ds;
            
            IDictionary<string, object> d = l.Describe();
            Assert.AreEqual((string)d["name"], "layer");
            Assert.AreEqual((string)d["srs"], "+init=epsg:4326");
            CollectionAssert.AreEquivalent(((List<string>)d["styles"]), new List<string>());
            CollectionAssert.AreEquivalent((Dictionary<string, object>)d["datasource"], options);
        }
    }
}
