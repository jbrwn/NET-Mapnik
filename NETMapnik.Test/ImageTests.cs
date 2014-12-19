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
            Assert.IsNull(i.Background);
        }

        [TestMethod]
        public void Image_GetSetBackground()
        {
            Image i = new Image(256, 256);
            Assert.IsNull(i.Background);
            i.Background = new Color("black");
            Assert.AreEqual(i.Background.ToString(), new Color("black").ToString());
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
            Image i = new Image(256, 256);
            i.Background = new Color("white");
            i.SetGrayScaleToAlpha();
            Color c = i.GetPixel(0, 0);
            Assert.AreEqual(c.R, 255);
            Assert.AreEqual(c.G, 255);
            Assert.AreEqual(c.B, 255);
            Assert.AreEqual(c.A, 255);

            i.Background = new Color("black");
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
            Image i1 = new Image(10, 10);
            i1.Background = new Color("green");

            Image i2 = Image.Open(@".\data\10x10green.png");
            Assert.AreEqual(i1.Compare(i2), 0);
        }

        [TestMethod]
        public void Image_FromBytes()
        {
            Image i1 = new Image(10, 10);
            i1.Background = new Color("green");


            byte[] buffer = File.ReadAllBytes(@".\data\10x10green.png");
            Image i2 = Image.FromBytes(buffer);
            Assert.AreEqual(i1.Compare(i2), 0);
        }

        [TestMethod]
        public void Image_Save()
        {
            Image i1 = new Image(10, 10);
            i1.Background = new Color("green");
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
            Image i1 = new Image(10, 10);
            i1.Background = new Color("green");
            byte[] bytes1 = i1.Encode("png");
            byte[] bytes2 = File.ReadAllBytes(@".\data\10x10green.png");
            CollectionAssert.AreEqual(bytes1, bytes2);
        }

        [TestMethod]
        public void Image_Painted()
        {
            DatasourceCache.RegisterDatasources(@".\mapnik\input");
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
            Image i1 = new Image(256, 256);
            Image i2 = new Image(256, 256);
            Assert.AreEqual(i1.Compare(i2), 0);

            i1.SetPixel(0, 0, new Color("white"));
            Assert.AreEqual(i1.Compare(i2), 1);

            i1.Background = new Color("black");
            Assert.AreEqual(i1.Compare(i2), i1.Width() * i1.Height());

            //test options
            Dictionary<string, object> options;
            i1.Background= new Color(255, 255, 255, 255);
            i2.Background = new Color(255, 255, 255, 0);
            options = new Dictionary<string, object> { { "Alpha", false } };
            Assert.AreEqual(i1.Compare(i2, options), 0);

            i1.Background = new Color(255, 255, 255);
            i2.Background = new Color(255, 255, 255);
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
