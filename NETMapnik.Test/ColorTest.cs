using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class ColorTest
    {
        [TestMethod]
        public void Color_FromText()
        {
            Color c = new Color("red");
            Assert.AreEqual(c.R, 255U);
            Assert.AreEqual(c.G, 0U);
            Assert.AreEqual(c.B, 0U);
            Assert.AreEqual(c.A, 255U);
            Assert.AreEqual(c.Hex(), "#ff0000");
            Assert.AreEqual(c.ToString(), "rgb(255,0,0)");
        }
    }
}
