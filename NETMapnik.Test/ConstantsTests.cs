using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class ConstantsTests
    {
        [TestMethod]
        public void Constants_Get()
        {
            Assert.IsInstanceOfType(Mapnik.Supports["svg"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["grid"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["cairo"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["cairo_svg"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["cairo_pdf"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["png"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["jpeg"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["tiff"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["webp"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["threadsafe"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Supports["proj4"], typeof(Boolean));
            Assert.IsInstanceOfType(Mapnik.Versions["CLR"], typeof(String));
            Assert.IsInstanceOfType(Mapnik.Versions["Mapnik"], typeof(String));
            Assert.IsInstanceOfType(Mapnik.Versions["Boost"], typeof(String));
            Assert.IsInstanceOfType(Mapnik.Paths["Fonts"], typeof(String));
            Assert.IsInstanceOfType(Mapnik.Paths["InputPlugins"], typeof(String));

            //Ignore key case
            Assert.IsInstanceOfType(Mapnik.Versions["MaPnIk"], typeof(String));
            Assert.IsInstanceOfType(Mapnik.Supports["CAIRO_SVG"], typeof(Boolean));
        }
    }
}
