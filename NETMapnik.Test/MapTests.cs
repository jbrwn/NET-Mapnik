using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class MapTests
    {
        [TestMethod]
        public void Map_Creation_Success()
        {
            Map m = new Map();
        }

        [TestMethod]
        public void Map_GetAndSetProperties_Success()
        {
            Map m = new Map();
            m.Height = 256;
            m.Width = 256;
            Assert.AreEqual(m.Height, 256U);
            Assert.AreEqual(m.Width, 256U);
        }




    }
}
