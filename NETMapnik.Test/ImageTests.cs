using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NETMapnik.Test
{
    [TestClass]
    public class ImageTests
    {
        [TestMethod]
        public void Image_Creation()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.Load(@".\data\test.xml");
            m.ZoomAll();
            Image i = new Image(256, 256);
            m.Render(i);
            Assert.AreNotEqual(i.Encode("jpeg").Length, 0);
        }
    }
}
