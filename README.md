# Symbol-Table-in-C
An implementaton of static scoped symbol tables for semantic analysis in C

### Building
First, download the dependencies:
```bash
./download_dependencies.sh
```

Then, to build the static library, run the following commands from the terminal:
```bash
mkdir build ; cd build && cmake .. && make ; cd ..
```
This will build ```libSymbolEnv.a``` in ```./lib``` directory.

### Usage
See ```include/SymbolEnv.h``` for information about functionality provided by this module
