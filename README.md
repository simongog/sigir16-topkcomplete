This is an example project of the [SIGIR 2016][SIGIR16] tutorial
[Succinct Data Structures in Information Retrieval: Theory and Practice][TUTPAGE]
presented by [Simon Gog][SIMON] and [Rossano Venturini][ROSSANO].


The example shows how the [Succinct Data Structure Library][SDSL] can be used
to implement a space-efficient top-k query completion system. The final result
is an almost state-of-the-art system which is implemented in less than 300 lines
of code.

Here is an example of our final system. The index is built over 
[titles and click counts of Wikipedia pages][WIKICOUNT].

<p align="center">
  <img src="https://github.com/simongog/sigir16-autocomplete/blob/master/web/demo.gif?raw=true" alt="Searching Wikipedia titles"/>
</p>

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
    ./index1-main ../data/stops_nl.txt
```
The binary will generate an index and wait for user input
and answer queries (one per line) interactively. The
index is stored in `../data/stops_nl.txt.index1.sdsl` and
a visualization of its memory consumption is available
at `stops_nl.txt.index1.html`. In general, each
executable `IDX-*`  will store the generated index
at `file.IDX.sdsl` and its space visualization at
`file.IDX.html`.


### Running the webserver version

```bash
    ./index1-webserver ../data/stops_nl.txt 8000
```

The binary will generate an index and start a webserver
which will listen to the specified port.

### Running the demo application

1. Change into the `build` directory
2. Download the Wikipedia titles by calling `make download` 
3. Build the executable by calling `make index4ci-webserver`
4. Generate the index and start the webserver by calling `./index4ci-webserver ../data/enwiki-20160601-all-titles`
5. You can access the demo at [http://127.0.0.1:8000](http://127.0.0.1:8000)


### Credits
    
  * Thanks to Sascha Witt for preparing the example input file
    which contains the pairs of Dutch train stations and number
    of daily train stops.

  * Thanks to [all contributers to the SDSL project][SDSLCONTRIB].

[TUTPAGE]: http://pages.di.unipi.it/rossano/succinct-data-structures-in-information-retrieval-theory-and-practice/
[SIGIR16]: http://sigir.org/sigir2016/
[ROSSANO]: http://pages.di.unipi.it/rossano/
[SIMON]: http://algo2.iti.kit.edu/gog/
[SDSL]: https://github.com/simongog/sdsl-lite
[SDSLCONTRIB]: https://github.com/simongog/sdsl-lite/graphs/contributors
[WIKICOUNT]: https://dumps.wikimedia.org/other/pagecounts-raw/
