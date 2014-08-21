using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NETMapnik;

namespace NETMapnik.Test
{
    [TestClass]
    public class VectorTileTests
    {
        [TestMethod]
        public void VectorTile_Creation()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
            Map m = new Map();
            m.Width = 256;
            m.Height = 256;
            m.LoadMap(@"..\..\data\layer.xml");
            //m.ZoomAll();
            m.ZoomToBox(-10037508, -10037508, 10037508, 10037508);
            VectorTile v = new VectorTile(0,0,0,256,256);
            m.Render(v);
            int byteCount = v.GetBytes().Length;
            Assert.AreNotEqual(byteCount, 0);

        }

        //[TestMethod]
        //public void VectorTile_Render()
        //{
        //    DatasourceCache.RegisterDatasources(@".\mapnik\input");
        //    Map m = new Map();
        //    m.Width = 256;
        //    m.Height = 256;
        //    m.LoadMap(@"..\..\data\layer.xml");
        //    //m.ZoomAll();
        //    m.ZoomToBox(-10037508, -10037508, 10037508, 10037508);
        //    VectorTile v = new VectorTile(0, 0, 0, 256, 256);
        //    m.Render(v);

        //    VectorTile v2 = new VectorTile(5, 6, 0, 256, 256);
        //    v2.SetBytes(v.GetBytes());
        //    Map m2 = new Map();
        //    //m2.Width = 256;
        //    //m2.Height = 256;
        //    m2.LoadMap(@"..\..\data\style.xml");
        //    //m2.ZoomToBox(-200000, -200000, 200000, 200000);
        //    Image i = new Image(512, 512);
        //    v2.Render(m2, i);
        //    i.Save(@"c:\tile.png", "png");
        //}
    }
}
