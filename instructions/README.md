
# Tutorial instructions

For this tutorial we have generated an example project which implements a top-k query completion system. The project depends on three resources (which are included as submodules and will be installed automatically):
* The [Succinct Data Structure Library](SDSL) will provide basic succinct data structures.
* The [Mongoose Embedded Web Server Library][MONGOOSE] was used to implement the server of our web application.
* On the client side of your web application [jQuery-Autocomplete][JQUERYAUTO] is used to fetch and display results

The project has the following software requirements:
* `git` version control  
* `cmake` build system
* a recent C++ compiler (`clang++` or `g++`)


You can clone the tutorial with the following command:

```bash
git clone https://github.com/simongog/sigir16-topkcomplete
```

Then change into the `sigir16-topkcomplete` directory and follow the instructions of the README file. After doing this you have already compiled two indexes. We will start to discuss the first one.  

## Solution 1

You can find the implementation of our first index in the file `include/topkcomp/index1.hpp`. The index is implemented as class which consists of
* three members (`m_text`, `m_start`,`m_weight`). Each one has SDSL class type.
  - `m_text` contains the concatenation of all strings of our input file. Strings were ordered lexicographically before they were concatenated.
  - `m_start` is a vector of `N` integers and `m_start[i]` equals the start of string `i` in `m_text`. Note that the entries in `m_start` are strictly increasing.
  - `m_weight` contains the weight for each string.
* a constructor, which takes a vector of pairs of strings and unsigned integers (i.e. strings and weights/priorities),
* two methods to `serialize` and `load` a class instance. Note that all SDSL classes implement these methods and we just have to call the methods of the members.
* and three methods which answer top-k queries:
  - `prefix_range(prefix)` determines the range in the sorted list of strings which is prefixed by `prefix`. This is done by binary search.
  - `heaviest_indexes_in_range()`  
  - `top-k(prefix, k)` determines the range of prefixes


## Solution

We have seen that `index1` takes

[SDSL]: https://github.com/simongog/sdsl-lite
[MONGOOSE]: https://github.com/cesanta/mongoose
[JQUERYAUTO]: https://https://github.com/devbridge/jQuery-Autocomplete
