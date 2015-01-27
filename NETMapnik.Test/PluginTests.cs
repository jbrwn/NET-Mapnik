using System;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NETMapnik.Test
{
    [TestClass]
    public class PluginTests
    {
        [TestMethod]
        public void Plugins_Init()
        {
            Mapnik.RegisterDefaultInputPlugins();
            List<string> ds = Mapnik.Datasources().ToList();
            Assert.IsTrue(ds.Count > 0);

            //Should return false if we try to re-register plugins
            Assert.IsFalse(Mapnik.RegisterDatasources(Mapnik.Paths["InputPlugins"]));
        }
    }
}
