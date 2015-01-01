using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace NETMapnik.Test
{
    [TestClass]
    public class ProjectionTests
    {
        [TestMethod]
        public void Projection_Init()
        {
            Projection wgs84 = new Projection("+init=epsg:4326");
            Projection merc = new Projection("+init=epsg:3857");
        }

        [TestMethod]
        public void Projection_Forward()
        {
            Projection merc = new Projection("+init=epsg:3857");
            double[] latlong = new double[] { -122.33517, 47.63752 };
            double[] expected = new double[] { -13618288.8305, 6046761.54747 };
            double[] actual = merc.Forward(latlong[0], latlong[1]);
            Assert.AreEqual(expected[0], actual[0], .0001);
            Assert.AreEqual(expected[1], actual[1], .0001);
        }

        [TestMethod]
        public void Projection_Inverse()
        {
            Projection merc = new Projection("+init=epsg:3857");
            double[] mercCoords = new double[] { -13627804.8659, 6041391.68077, -13608084.1728, 6053392.19471 };
            double[] expected = new double[] { -122.420654, 47.605006, -122.2435, 47.67764 };
            double[] actual = merc.Inverse(mercCoords[0], mercCoords[1], mercCoords[2], mercCoords[3]);
            Assert.AreEqual(expected[0], actual[0], .0001);
            Assert.AreEqual(expected[1], actual[1], .0001);
            Assert.AreEqual(expected[2], actual[2], .0001);
            Assert.AreEqual(expected[3], actual[3], .0001);
        }

        [TestMethod]
        public void ProjTransform_Init()
        {
            Projection src = new Projection("+init=epsg:4326");
            Projection dest = new Projection("+init=epsg:3857");
            ProjTransform tran = new ProjTransform(src, dest);
        }

        [TestMethod]
        public void ProjTransform_NoChange()
        {
            Projection src = new Projection("+init=epsg:4326");
            Projection dest = new Projection("+init=epsg:4326");
            ProjTransform tran = new ProjTransform(src, dest);
            double[] expected = new double[] { -122.33517, 47.63752 };

            //forward
            double[] actual = tran.Forward(expected[0], expected[1]);
            Assert.AreEqual(expected[0], actual[0]);
            Assert.AreEqual(expected[1], actual[1]);

            //backward
            actual = tran.Backward(expected[0], expected[1]);
            Assert.AreEqual(expected[0], actual[0]);
            Assert.AreEqual(expected[1], actual[1]);
        }

        [TestMethod]
        public void ProjTransform_Forward()
        {
            Projection src = new Projection("+init=epsg:4326");
            Projection dest = new Projection("+init=epsg:3857");
            ProjTransform tran = new ProjTransform(src, dest);
            double x = -122.33517;
            double y = 47.63752;
            double[] expected = new double[] { -13618288.8305, 6046761.54747 };

            //forward
            double[] actual = tran.Forward(x, y);
            Assert.AreEqual(expected[0], actual[0], .0001);
            Assert.AreEqual(expected[1], actual[1], .0001);
        }

        [TestMethod]
        public void ProjTransform_Backward()
        {
            Projection src = new Projection("+init=epsg:4326");
            Projection dest = new Projection("+init=epsg:3857");
            ProjTransform tran = new ProjTransform(src, dest);
            double x =  - 13618288.8305;
            double y = 6046761.54747;
            double[] expected = new double[] {-122.33517, 47.63752 };

            //forward
            double[] actual = tran.Backward(x, y);
            Assert.AreEqual(expected[0], actual[0], .0001);
            Assert.AreEqual(expected[1], actual[1], .0001);
        }
    }
}