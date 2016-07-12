
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

In the following description we index as set of `N` (string,weight)-paris. Let `n` be the length of the concatenation of all strings in the set and `m` be the length of the pattern.    

## Solution 1

You can find the implementation of our first index in the file [`index1.hpp`][IDX1]. The index is implemented as class which consists of
* three members (`m_text`, `m_start`,`m_weight`). Each one has SDSL class type.
  - `m_text` contains the concatenation of all strings of our input file. Strings were ordered lexicographically before they were concatenated.
  - `m_start` is a vector of `N` integers and `m_start[i]` equals the start of string `i` in `m_text`. Note that the entries in `m_start` are strictly increasing.
  - `m_weight` contains the weight for each string.
* a constructor, which takes a vector of pairs of strings and unsigned integers (i.e. strings and weights/priorities),
* two methods to `serialize` and `load` a class instance. Note that all SDSL classes implement these methods and we just have to call the methods of the members.
* and three methods which are used to answer top-k queries:
  - `prefix_range(prefix)` determines the range in the sorted list of strings which is prefixed by `prefix`. This is done by binary search. I.e. it takes O(m*log n) time.
  - `heaviest_indexes_in_range(k, range, w)` (defined in [index_common.hpp][IDXC]) determines `k` indexes in the range `[begin, end)`, which are associated with the heaviest weight in weight vector `w`. For a range of size `r=end-begin` this takes O(r*log k) time as we use a min-priority queue of size `k` to maintain the heaviest strings.
  - `top-k(prefix, k)` first determines the range of all strings prefixed by `prefix`, and the uses `heaviest_indexes_in_range` to get the `k` positions of the heaviest strings in the set. The only work left is to replace the positions by the (string,weight) pairs.

### Exercise 1.a

We want to analyze the practical properties of index1. For this we first compiler the index (change into the `build` directory and call `make index1-main` to create the console application). Build the index for the Wikipedia titles (`./index1-main ../data/enwiki-20160601-all-titles `) and report the total space consumption of the index and the space breakdown of the three components.

How fast are query answered for `k=5` and the empty prefix, prefix `E`, prefix `Ex`, `Exw`?

### Exercise 1.b (optional)

How do the numbers change for `k=50`? To answer this question you have to modify the console program (see [index.cpp][MAIN]).

## Solution 2



[SDSL]: https://github.com/simongog/sdsl-lite
[MONGOOSE]: https://github.com/cesanta/mongoose
[JQUERYAUTO]: https://https://github.com/devbridge/jQuery-Autocomplete
[IDX1]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index1.hpp
[IDXC]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index_common.hpp
[MAIN]: https://github.com/simongog/sigir16-topkcomplete/blob/master/src/index.cpp
