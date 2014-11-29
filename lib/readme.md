# Build dependencies

### mapnik windows sdk
https://mapnik.s3.amazonaws.com/dist/dev/mapnik-v2.3.0-WinSDK-32bit-7c7da1a2fd.7z

### libprotobuf
https://code.google.com/p/protobuf/downloads/detail?name=protobuf-2.5.0.zip

Extract includes
```
protobuf-2.5.0\vsprojects\extract_includes.bat
```
Upgrade protobuf solution to VS2013
```
protobuf-2.5.0\vsprojects\protobuf.sln
```
Patch solution projects for VS2013
* https://code.google.com/p/protobuf/issues/detail?id=531

Build solution in release configuration

### mapnik-vector-tile 
https://github.com/mapbox/mapnik-vector-tile @5394eed

Build
```
protobuf-2.5.0\vsprojects\Release\protoc.exe -I mapnik-vector-tile\proto --cpp_out=mapnik-vector-tile\src mapnik-vector-tile\proto\vector_tile.proto
```
