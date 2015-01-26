using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace NETMapnik.Test
{
    [TestClass]
    public class ImageViewTests
    {
        [TestMethod]
        public void ImageView_Init()
        {
            Image i = new Image(256, 256);
            ImageView iv = i.View(0, 0, 256, 256);
            Assert.IsTrue(iv.IsSolid());
            Color pixel = iv.GetPixel(0, 0);
            Assert.AreEqual(pixel.ToString(), "rgba(0,0,0,0.0)");

            i.Background = new Color(255, 255, 255);
            iv = i.View(0, 0, 256, 256);
            Assert.IsTrue(iv.IsSolid());
            pixel = iv.GetPixel(0, 0);
            Assert.AreEqual(pixel.ToString(), "rgb(255,255,255)");
        }

        [TestMethod]
        public void ImageView_IsSolid()
        {
            Image i = new Image(256, 256);
            ImageView iv = i.View(0, 0, 256, 256);
            Assert.IsTrue(iv.IsSolid());

            i.SetPixel(0, 0, new Color("red"));
            iv = i.View(0, 0, 256, 256);
            Assert.IsFalse(iv.IsSolid());
        }

        [TestMethod]
        public void ImageView_Save()
        {
            Image i = new Image(100, 100);
            i.Background = new Color("green");
            ImageView iv = i.View(0,0,10,10);
            string filename = @".\data\tmp\" + Guid.NewGuid().ToString() + ".png";
            iv.Save(filename);
            byte[] bytes1 = File.ReadAllBytes(filename);
            byte[] bytes2 = File.ReadAllBytes(@".\data\10x10green.png");
            CollectionAssert.AreEqual(bytes1, bytes2);
        }

        [TestMethod]
        public void ImageView_Encode()
        {
            Image i = new Image(100, 100);
            i.Background = new Color("green");
            ImageView iv = i.View(0, 0, 10, 10);
            byte[] bytes1 = iv.Encode("png");
            byte[] bytes2 = File.ReadAllBytes(@".\data\10x10green.png");
            CollectionAssert.AreEqual(bytes1, bytes2);
        }
    }
}
