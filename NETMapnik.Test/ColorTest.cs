using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class ColorTest
    {
        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Color_Throws()
        {
            Color c = new Color("foo");
        }

        [TestMethod]
        public void Color_FromText()
        {
            Color c = new Color("red");
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 0);
            Assert.AreEqual(c.B, 0);
            Assert.AreEqual(c.A, 255);
            Assert.IsFalse(c.Premultiplied);
            Assert.AreEqual(c.Hex(), "#ff0000");
            Assert.AreEqual(c.ToString(), "rgb(255,0,0)");
        }

        [TestMethod]
        public void Color_FromRGB()
        {
            Color c = new Color(255,0,0);
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 0);
            Assert.AreEqual(c.B, 0);
            Assert.AreEqual(c.A, 255);
            Assert.IsFalse(c.Premultiplied);
            Assert.AreEqual(c.Hex(), "#ff0000");
            Assert.AreEqual(c.ToString(), "rgb(255,0,0)");
        }

        [TestMethod]
        public void Color_FromRGBA()
        {
            Color c = new Color(255,0,0,255);
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 0);
            Assert.AreEqual(c.B, 0);
            Assert.AreEqual(c.A, 255);
            Assert.IsFalse(c.Premultiplied);
            Assert.AreEqual(c.Hex(), "#ff0000");
            Assert.AreEqual(c.ToString(), "rgb(255,0,0)");
        }

        [TestMethod]
        public void Color_FromRGBAPercent()
        {
            Color c = new Color("rgba(100%,0%,0%,1)");
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 0);
            Assert.AreEqual(c.B, 0);
            Assert.AreEqual(c.A, 255);
            Assert.AreEqual(c.Hex(), "#ff0000");
            Assert.IsFalse(c.Premultiplied);
            Assert.AreEqual(c.ToString(), "rgb(255,0,0)");
        }

        [TestMethod]
        public void Color_Premultiply()
        {
            Color c = new Color(128,128,128,true);
            Assert.AreEqual(c.R, 128);
            Assert.AreEqual(c.G, 128);
            Assert.AreEqual(c.B, 128);
            Assert.AreEqual(c.A, 255);
            Assert.IsTrue(c.Premultiplied);
            Assert.AreEqual(c.ToString(), "rgb(128,128,128)");
        }
    }
}
