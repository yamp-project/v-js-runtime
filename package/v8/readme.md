# Use prebuilt binaries (Default)
xmake config --v8_build_mode=prebuilt
xmake build v8

# Build from source 
xmake config --v8_build_mode=source
xmake build v8_from_source

# Use local V8 installation
xmake config --v8_build_mode=local --v8_local_path=path-to-v8-source
xmake build v8