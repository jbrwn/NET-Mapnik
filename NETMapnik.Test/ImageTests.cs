using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;

namespace NETMapnik.Test
{
    [TestClass]
    public class ImageTests
    {

        [TestMethod]
        public void Image_Init()
        {
            Image i = new Image(256, 256);
            Assert.AreEqual(i.Width(), 256);
            Assert.AreEqual(i.Height(), 256);
            Assert.IsFalse(i.Painted());
        }

        [TestMethod]
        public void Image_GetSetPixel()
        {
            Image i = new Image(1, 1);
            Color c = i.GetPixel(0, 0);
            Assert.AreEqual(c.ToString(), "rgba(0,0,0,0.0)");
            i.SetPixel(0,0, new Color(255, 255, 255));
            Color c1 = i.GetPixel(0, 0);
            Assert.AreEqual(c1.ToString(), "rgb(255,255,255)");
        }

        [TestMethod]
        public void Image_SetGrayScaleToAlpha()
        {
            Map m = new Map(256, 256);
            Image i = new Image(256, 256);
            m.Background = new Color("white");
            m.Render(i);
            i.SetGrayScaleToAlpha();
            Color c = i.GetPixel(0, 0);
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 255);
            Assert.AreEqual(c.B, 255);
            Assert.AreEqual(c.A, 255);

            m.Background = new Color("black");
            m.Render(i);
            i.SetGrayScaleToAlpha();
            Color c1 = i.GetPixel(0, 0);
            Assert.AreEqual(c1.R, 255);
            Assert.AreEqual(c1.G, 255);
            Assert.AreEqual(c1.B, 255);
            Assert.AreEqual(c1.A, 0);

            i.SetGrayScaleToAlpha(new Color("green"));
            Color c2 = i.GetPixel(0, 0);
            Assert.AreEqual(c2.R, 0);
            Assert.AreEqual(c2.G, 128);
            Assert.AreEqual(c2.B, 0);
            Assert.AreEqual(c2.A, 255);
        }

        [TestMethod]
        public void Image_Open()
        {
            Map m = new Map(10, 10);
            Image i1 = new Image(10, 10);
            m.Background = new Color("green");
            m.Render(i1);

            Image i2 = Image.Open(@".\data\10x10green.png");
            Assert.AreEqual(i1.Compare(i2), 0);
        }

        [TestMethod]
        public void Image_FromBytes()
        {
            Map m = new Map(10, 10);
            Image i1 = new Image(10, 10);
            m.Background = new Color("green");
            m.Render(i1);

            byte[] buffer = File.ReadAllBytes(@".\data\10x10green.png");
            Image i2 = Image.FromBytes(buffer);
            Assert.AreEqual(i1.Compare(i2), 0);
        }

        [TestMethod]
        public void Image_Save()
        {
            Map m = new Map(10, 10);
            Image i1 = new Image(10, 10);
            m.Background = new Color("green");
            m.Render(i1);
            string filename = @".\data\tmp\" + Guid.NewGuid().ToString() + ".png";
            i1.Save(filename);

            MD5 md5 = MD5.Create();
            byte[] hash1 = md5.ComputeHash(File.ReadAllBytes(filename));
            byte[] hash2 = md5.ComputeHash(File.ReadAllBytes(@".\data\10x10green.png"));
            CollectionAssert.AreEqual(hash1, hash2);
        }

        [TestMethod]
        public void Image_Encode()
        {
            Map m = new Map(10, 10);
            Image i1 = new Image(10, 10);
            m.Background = new Color("green");
            m.Render(i1);
            byte[] bytes1 = i1.Encode("png");
            byte[] bytes2 = File.ReadAllBytes(@".\data\10x10green.png");
            CollectionAssert.AreEqual(bytes1, bytes2);
        }

        [TestMethod]
        public void Image_Painted()
        {
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "shape.input"));
            Image i = new Image(256, 256);
            Assert.IsFalse(i.Painted());

            Map m = new Map(256,256);
            m.ZoomAll();
            m.Render(i);
            Assert.IsFalse(i.Painted());

            m.Load(@".\data\test.xml");
            m.ZoomAll();
            m.Render(i);
            Assert.IsTrue(i.Painted());
        }

        [TestMethod]
        public void Image_Compare()
        {
            Map m1 = new Map(256, 256);
            Map m2 = new Map(256, 256);
            Image i1 = new Image(256, 256);
            Image i2 = new Image(256, 256);
            Assert.AreEqual(i1.Compare(i2), 0);

            i1.SetPixel(0, 0, new Color("white"));
            Assert.AreEqual(i1.Compare(i2), 1);

            m1.Background = new Color("black");
            m1.Render(i1);
            Assert.AreEqual(i1.Width() * i1.Height(), i1.Compare(i2));

            //test options
            m1.Background= new Color(100, 100, 100, 255);
            m1.Render(i1);
            i2 = new Image(256, 256);
            m2.Background = new Color(100,100,100,100);
            m2.Render(i2);
            Dictionary<string, object> options;
            options = new Dictionary<string, object> { { "Alpha", false } };
            Assert.AreEqual(i1.Compare(i2, options), 0);

            m1.Background = new Color(255, 255, 255);
            m1.Render(i1);
            m2.Background = new Color(255, 255, 255);
            m2.Render(i2);
            i2.SetPixel(0, 0, new Color(250, 250, 250));
            options = new Dictionary<string, object> { { "Threshold", 5 } };
            Assert.AreEqual(i1.Compare(i2, options), 0);
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Image_Compare_InvalidSize()
        {
            Image i1 = new Image(256, 256);
            Image i2 = new Image(512, 512);
            i1.Compare(i2);
            
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Image_Encode_InvalidFormat()
        {
            Image im = new Image(256, 256);
            im.Encode("foo");
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Image_Save_NoExtension()
        {
            Image im = new Image(256, 256);
            im.Save("foo");
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Image_Save_InvalidFormat()
        {
            Image im = new Image(256, 256);
            im.Save(@".\data\tmp\foo", "foo");
        }

        [TestMethod]
        [ExpectedException(typeof(Exception))]
        public void Image_FromBytes_InvalidBuffer()
        {
            Image im = Image.FromBytes(new byte[] { 0 });
        }

        [TestMethod]
        public void Image_Composite()
        {
            string filebase = @".\data\tmp\" + Guid.NewGuid().ToString();
            foreach (CompositeOp compop in Enum.GetValues(typeof(CompositeOp)))
            {
                Image i1 = Image.Open(@".\data\circle1.png");
                i1.Premultiply();
                Image i2 = Image.Open(@".\data\circle2.png");
                i2.Premultiply();
                Dictionary<string, object> options = new Dictionary<string, object>()
                {
                    { "CompOp", compop}
                };
                Image i3 = i1.Composite(i2, options);
                i3.Demultiply();
                i3.Save(filebase + compop.ToString() + ".png");
            }
        }



    }
}
