# This is a test code project

## Building

1. Build cmake project:
```cmake -B build```
```cmake --build build```

2. Start Server app:
```./build/src/server/server```

3. Start clint app and pass file paths of files you want to send as an argument:
```./build/src/client/client <file_path> <another_file_path> ...```

## Alternative

1. Use script to build sources:
```./build all```

2. Use script to run client and server with test files
```./run```