
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

## Top-k completion system #1

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

What happens if you search for a lower case prefix: `a`, `b`,`c`,...?

### Exercise 1.b (optional)

How do the numbers change for `k=50`? To answer this question you have to modify the console program (see [index.cpp][MAIN]).

## Top-k completion system #2

In our second solution we keep the algorithmic framework of our first solution. We just alter the representation of `m_start`. While we were
using  n &lceil;log n&rceil; bits for `m_start` by using an `int_vector<>` we now use a bit vector and select structure to represent it. So we mark in `m_start_bv` every each position at which a string starts in the concatenation and construct a select structure `m_start_sel` for `m_start_bv`.

Instead of accessing `m_start[i]` we can get the start of the i-th string by querying `m_start_sel(i+1)` (not that we have to add one, since the indexing is 1-based in all SDSL select structures).

[TODO: more on select structures here?]


In order to compile index2, we have to remove the comment symbol `#` in front of entry `index2;index2<>` in the [index.config][IDXCFG] file. Again call `cmake .. && make index2-main` to generate the executable.


### Exercise 2.a

How much space is used by the bit vector and select structure which mark the start of the strings in the concatentation? Explore the effect on query speed.


### Exercise 2.b

Examine how much space can be saved by using the Elias-Fano representation (i.e. using the `sd_vector<>` class instead of the plain bit vector class `bit_vector`)?
You can plug-in the Elias-Fano representation by adjusting the template parameter of `index2`.

Explore the effect on query speed.


## Top-k completion system #3

We have seen that the marking the start of strings is now only a small fraction of the total memory consumption of the index. We now try to improve the space of the remaining parts, namely the concatenated text and the weights.






[SDSL]: https://github.com/simongog/sdsl-lite
[MONGOOSE]: https://github.com/cesanta/mongoose
[JQUERYAUTO]: https://https://github.com/devbridge/jQuery-Autocomplete
[IDX1]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index1.hpp
[IDXC]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index_common.hpp
[MAIN]: https://github.com/simongog/sigir16-topkcomplete/blob/master/src/index.cpp
[IDXCFG]: https://github.com/simongog/sigir16-topkcomplete/blob/master/index.config
