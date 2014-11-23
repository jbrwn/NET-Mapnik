#include "stdafx.h"
#include "NET_box_utils.h"

namespace NETMapnik
{
	mapnik::box2d<double> ArrayToBox2D(array<System::Double>^ boxArray)
	{
		if (boxArray->Length != 4)
			throw gcnew System::Exception("Bbox array must contain 4 values: minx,miny,maxx,maxy");

		return mapnik::box2d<double>(boxArray[0], boxArray[1], boxArray[2], boxArray[3]);
	}

	array<System::Double>^ Box2DToArray(mapnik::box2d<double> bbox)
	{
		array<System::Double>^ boxArray = gcnew array<System::Double>(4);
		boxArray[0] = bbox.minx();
		boxArray[1] = bbox.miny();
		boxArray[2] = bbox.maxx();
		boxArray[3] = bbox.maxy();

		return boxArray;
	}
}
