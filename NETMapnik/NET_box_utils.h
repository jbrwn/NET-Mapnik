#include <mapnik\box2d.hpp>

namespace NETMapnik
{
	mapnik::box2d<double> ArrayToBox2D(array<System::Double>^ boxArray);
	array<System::Double>^ Box2DToArray(mapnik::box2d<double> bbox);
}
