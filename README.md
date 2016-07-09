This is an example project of the [SIGIR 2016][SIGIR16] tutorial
[Succinct Data Structures in Information Retrieval: Theory and Practice][TUTPAGE]
presented by [Simon Gog][SIMON] and [Rossano Venturini][ROSSANO].


The example shows how the [Succinct Data Structure Library][SDSL] can be used
to implement a space-efficient top-k query completion system. The final result
is an almost state-of-the-art system which is implemented in less than 300 lines
of code.



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
    
    * Thanks to Sascha Witt for preparing the example input file
      which contains the pairs of Dutch train stations and number
      of daily train stops.

    * Thanks to all contributers of the SDSL.

[TUTPAGE]: http://pages.di.unipi.it/rossano/succinct-data-structures-in-information-retrieval-theory-and-practice/
[SIGIR16]: http://sigir.org/sigir2016/
[ROSSANO]: http://pages.di.unipi.it/rossano/
[SIMON]: http://algo2.iti.kit.edu/gog/
[SDSL]: https://github.com/simongog/sdsl-lite
[SDSLCONTRIB]: https://github.com/simongog/sdsl-lite/graphs/contributors
