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
            VectorTile v = new VectorTile();
        }

        //[TestMethod]
        //[DeploymentItem(@"data\test.xml")]
        //public void VectorTile_Output()
        //{
        //    DatasourceCache.RegisterDatasources(@"C:\mapnik-v2.3.0\lib\mapnik\input");
        //    Map m = new Map();
        //    m.Width = 256;
        //    m.Height = 256;

        //    m.LoadMap(Path.Combine(Path.GetFullPath("."),"test.xml"));
        //    m.ZoomAll();

        //    VectorTile v = new VectorTile();
        //    m.Render(v);
        //    byte[] b = v.GetBytes();
        //    int i = b.Length;
        //}
    }
}
