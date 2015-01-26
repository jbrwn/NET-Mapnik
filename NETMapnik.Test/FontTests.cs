using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class FontTests
    {
        [TestMethod]
        public void Font_GlobalFonts()
        {
            Mapnik.RegisterDefaultFonts();
            Mapnik.RegisterSystemFonts();
            IDictionary<string, string> files = Mapnik.FontFiles();
            IEnumerable<string> fonts = Mapnik.Fonts();
            Assert.IsTrue(fonts.Count() > 0);
            Assert.IsTrue(files.Count() > 0);
            CollectionAssert.AreEquivalent(files.Keys.ToList(), fonts.ToList());

            // Registering system fonts should always fail on retry
            Assert.IsFalse(Mapnik.RegisterSystemFonts());
        }

        [TestMethod]
        public void Font_MapFonts()
        {
            Map m = new Map(1, 1);
            Assert.AreEqual(0, m.FontFiles().Count());
            Assert.AreEqual(0, m.Fonts().Count());
            Assert.AreEqual(0, m.MemoryFonts().Count());

            m.RegisterFonts(@".\data\font");
            Assert.AreEqual(1, m.FontFiles().Count());
            Assert.AreEqual(1, m.Fonts().Count());
            string fontFile = m.FontFiles()["DejaVu Sans Mono Bold Oblique"];
            Assert.AreEqual(@".\data\font\DejaVuSansMono-BoldOblique.ttf", fontFile);

            Assert.AreEqual(0, m.MemoryFonts().Count());
            m.LoadFonts();
            Assert.AreEqual(1, m.MemoryFonts().Count());
        }
    }
}
