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
            CollectionAssert.AreEqual(l.Styles.ToList(), new List<string>());

        }

        [TestMethod]
        public void Layer_Describe()
        {
            Layer l = new Layer("layer", "+init=epsg:4326");
            Dictionary<string, object> d = l.Describe();

            Assert.AreEqual((string)d["name"], "layer");
            Assert.AreEqual((string)d["srs"], "+init=epsg:4326");
            CollectionAssert.AreEqual(((IEnumerable<string>)d["styles"]).ToList(), new List<string>());
        }
    }
}
