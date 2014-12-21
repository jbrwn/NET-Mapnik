using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NETMapnik.Test
{
    [TestClass]
    public class PaletteTests
    {
        [TestMethod]
        public void Palette_Init_RGBAString()
        {
            Palette p = new Palette("\x01\x02\x03\x04");
            Assert.AreEqual("[Palette 1 color #01020304]", p.ToString());
        }

        [TestMethod]
        public void Palette_Init_RGBString()
        {
            Palette p = new Palette("\x01\x02\x03", PaletteType.PALETTE_RGB);
            Assert.AreEqual("[Palette 1 color #010203]", p.ToString());
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Palette_Init_RGBString_NoType()
        {
            Palette p = new Palette("\x01\x02\x03");
        }

        [TestMethod]
        public void Palette_Init_RGBABytes()
        {
            byte[] bytes = new byte[4] { 0x01, 0x02, 0x03, 0x04 };
            Palette p = new Palette(bytes);
            Assert.AreEqual("[Palette 1 color #01020304]", p.ToString());
        }

        [TestMethod]
        public void Palette_ToBytes()
        {
            byte[] bytes = new byte[4] { 0x01, 0x02, 0x03, 0x04 };
            Palette p = new Palette("\x01\x02\x03\x04");
            CollectionAssert.AreEqual(bytes, p.ToBytes());
        }

    }
}
