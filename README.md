This is an example project presented at the Succinct Data Structure Tutorial at SIGIR 2016.

### Input format

Each line of the input file should consists of a (entry, priority)-pair.
Entry is a string followed by a tab `\t` followed by an integer which
represents the priority.

### Installation

```bash
    ./install.sh
```

### Building the project

```bash
    cd build
    cmake ..
    make
```

CMake will parse the `index.config` file and generate
binaries for each index. The index name will be the prefix
of the corresponding executables.

### Running the command line version


```bash
    ./index4-main ../data/stops_nl.txt
```
The binary will generate an index and wait for user input
and answer queries (one per line) interactively.


### Running the web server version

```bash
    ./index4-webserver ../data/stops_nl.txt 8000
```

TODO: The binary will generate an index and start a web server
which will listen to the specified port.
In the example the port
is 8080 and you will get top-10 query suggestions at 
[http://127.0.0.1:8000](http://127.0.0.1:8000).


### Credits

Thanks to Sascha Witt for preparing the file which contains the pairs of
Dutch train stations and number of daily train stops.
