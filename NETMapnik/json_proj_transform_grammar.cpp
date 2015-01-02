#include "stdafx.h"
#include <mapnik/json/geometry_generator_grammar_impl.hpp>
#include "proj_transform_adapter.h"
#include <string>

using sink_type = std::back_insert_iterator<std::string>;
template struct mapnik::json::multi_geometry_generator_grammar<sink_type, NETMapnik::proj_transform_container>;