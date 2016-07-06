This is an example project presented at the Succinct Data Structure Tutorial at SIGIR 2016.

### Input format

Each line of the input file should consists of a (entry, priority)-pair.
Entry is a string followed by a tab `\t` followed by an integer which
represents the priority.

### Installation

```bash
    ./install.sh
```

### Execution

```bash
    cd build
    cmake ..
    make
    ./index4-main ../data/stops_nl.txt
    ./index4-webserver ../data-stops_nl.txt 8000
```

The webserver will listen to the specified port. In the example the port
is 8080 and you will get top-k query suggestions at 
[http://127.0.0.1:8000](http://127.0.0.1:8000).


### Credits

Thanks to Sascha Witt for preparing the file which contains the pairs of
Dutch train stations and number of daily train stops.
