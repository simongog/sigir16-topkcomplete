
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
  - `prefix_range(prefix)` determines the range in the sorted list of strings which is prefixed by `prefix`. This is done by binary search. I.e. it takes O(m&times;log n) time.
  - `heaviest_indexes_in_range(k, range, w)` (defined in [index_common.hpp][IDXC]) determines `k` indexes in the range `[begin, end)`, which are associated with the heaviest weight in weight vector `w`. For a range of size `r=end-begin` this takes O(r&times;log k) time as we use a min-priority queue of size `k` to maintain the heaviest strings.
  - `top-k(prefix, k)` first determines the range of all strings prefixed by `prefix`, and the uses `heaviest_indexes_in_range` to get the `k` positions of the heaviest strings in the set. The only work left is to replace the positions by the (string,weight) pairs.

### Exercise 1.a

We want to analyze the practical properties of index1. For this we first compiler the index (change into the `build` directory and call `make index1-main` to create the console application). Build the index for the Wikipedia titles (`./index1-main ../data/enwiki-20160601-all-titles `) and report the total space consumption of the index and the space breakdown of the three components.

How fast are query answered for `k=5` and the empty prefix, prefix `E`, prefix `Ex`, `Exw`?

What happens if you search for a lower case prefix: `a`, `b`,`c`,...?

### Exercise 1.b (optional)

How do the numbers change for `k=50`? To answer this question you have to modify the console program (see [index.cpp][MAIN]).

## Top-k completion system #2

In our second solution (in [index2.hpp][IDX2]) we keep the algorithmic framework of our first solution. We just alter the representation of `m_start`. While we were
using  n &lceil;log n&rceil; bits for `m_start` by using an `int_vector<>` we now use a bit vector and select structure to represent it. So we mark in `m_start_bv` every each position at which a string starts in the concatenation and construct a select structure `m_start_sel` for `m_start_bv`.

Instead of accessing `m_start[i]` we can get the start of the i-th string by querying `m_start_sel(i+1)` (note that we have to add one, since the indexing is 1-based in all SDSL select support structures but we start from zero when numbering the strings). The support structure has to be initialized in the constructor and assigned the supported member (via method `set_vector`) again in the `load` method.

In order to compile index2, we have to remove the comment symbol `#` in front of entry `index2;index2<>` in the [index.config][IDXCFG] file. Again call `cmake .. && make index2-main` to generate the executable.


### Exercise 2.a

How much space is used by the bit vector and select structure which mark the start of the strings in the concatentation? Explore the effect on query speed.


### Exercise 2.b

Examine how much space can be saved by using the Elias-Fano representation (i.e. using the `sd_vector<>` class instead of the plain bit vector class `bit_vector`)?
You can plug-in the Elias-Fano representation by adjusting the template parameter of `index2`.

Explore the effect on query speed.


## Top-k completion system #3

We have seen that the marking the start of strings is now only a small fraction of the total memory consumption of the index. We now try to improve the space of the remaining parts, namely the concatenated text and the weights. For this we build a trie of the keys and store the trie topology as a balanced parentheses sequence in succinct space.
Our first trie based implementation is contained in [index3.hpp][IDX3].

* The construction of the succinct trie representation is done directly in method `build_tree`. The member `m_bp` which will hold the balanced parentheses is initialized to the maximal size of the representation which is 4N bits (as we have N leave nodes, at most N-1 inner nodes, and we take write two bits per node). The character vector `m_labels` which will keep the labels is also initialized to its maximal size n. We also initialize `start_bv` which will be used to store the length of the edge labels to its maximum size 2N+n+2 (we have at most 2N nodes and the total length of the labels is at most n). We then built the trie recursively as follows: We start with the range [0,N-1] and determine the length of the longest common prefix `l` of all strings by comparing `string[0]` and `string[N-1]`. We append an opening parenthesis to `m_bp`, append `l` 0-bits and a 1-bit to `start_bv` and append the `l` common prefix characters to `m_labels`. Then we recurse by determine all subranges which share a common prefix of length > `l`. After the recursive calls we append a closing parenthesis to `m_bp`. Last, we resize the structures to their actual size.
* On top of `m_bp` we have two additional support structures. A rank structure for bit-pattern `10` (remember that `()` or `10` corresponds to a leaf in the tree represented by the balanced parentheses sequence) and a select structure for `10`.
* We add methods to navigate and access our trie:
  - `node_id(v)` maps a node identifier `v` (=position of corresponing opening parenthesis in `m_bp`) to a unique id in [0,N-1]. This can be done by counting the number of opening parentheses in `m_bp[0,v-1]`.
  - `edge(v_id)` for a given node id `v_id` we get the `begin` and `end` of its label by two selects in `m_start_bv` and the return a random access container to `m_labels[begin,end-1]`.
  - `is_leaf(v)` determines if a node is a leaf node.
  - `is_root(v)` determines if the node is the root.
  - `parent(v)` return the parent node of `v`.
  - `label(idx)` return the string formed by the concatenation of all edge labels from the root to the `idx`-th leaf in the trie.
  - `children(v)` return a vector a children of `v` (ordered according to the first character of the edge labels).
* With this operations we can now implement the `prefix_range` method:
  - TODO
* discuss who subtree is mapped back to lexicographic range

[TODO: more description here, discuss introduction of `m_bp_rnk10` and `m_bp_sel10`,`m_bp_support`]

### Exercise 3.a

Again we want to analyze the practical properties of index3. You can remove again the `#` sign in front of index3 in the [index.config][IDXCFG] file and call `cmake .. && make index3-main`.
Build the index for the Wikipedia titles (`./index3-main ../data/enwiki-20160601-all-titles `) and report the total space consumption of the index and the space breakdown of the three components.

What is the effect on the query times (`printf "\nE\nEx\n" | ./index3-main ../data/enwiki-20160601-all-titles`)?

### Exercise 3.b

We have used a plain bit vector (`bit_vector`) and its select structure to compress the starting points of the labels. How much space can be saved by using the Elias-Fano representation (`sd_vector<>`)? Implement a version which uses Elias-Fano and call it `index3a`.

### Exercise 3.c

We assume that there a a lot of Wikipedia pages which are not clicked often which means that there are many small values in our `weight` vector. Investigate how much you can reduce the space of this component by replacing the `int_vector<>` data structure by alternatives presented in the tutorial slides of SDSL (e.g. variants of `dac_vector<>` and `vlc_vector<>`).


How does the significant space reduction influence query time?


### Exercise 3.d (optional)

The implemented tree construction is quite inefficient in the worst case, as we determine the child ranges of an range by scanning the whole range. Suppose that we can afford to store O(n) extra bits during construction. Can you devise a solution which has O(n) time complexity?

### Exercise 3.e (optional)

What would be the advantage of using the DFUDS representation in this application?


## Top-k completion system #4

We have seen that the space for the `weight` component can be reduced significantly but at the same time query speed drops dramatically. We can use a range maximum query (RMQ) structure built on top of `weight` to reduce the number of accesses to the `weight` vector. The algorithmic change happens in the `haviest_index_in_range` method. We use the RMQ to get the position `p` of the maximum in the initial range `[begin,end)`. We then extract `weight[p]` put the `(weight[p],[begin,end))`-pair into a max-priority queue. From this queue we extract the top element and recurse into ranges `[begin,p)` and `[p+1,end)` if they are not empty. The extracted element is added to our top-k list.
Now the query time is independent from the range size and only O(k*log k), as we extract at most k elements from the queue and produce at most two new elements per extracted elements.

Note that the code of `index4` (see [index4.hpp][IDX4]) hardly changes compared to `index3`. We just
* added the member m_rmq
* and wrote a new version of `heaviest_indexes_in_range` (see [index_common.hpp][IDXC]).

### Exercise 4.a

Compare the `index4` to `index3a` in time and space.
Create two versions of `index4`. One which uses `dac_vector<4>` (named `index3a`) and one which uses `vlc_vector<>` (named `index3b`) for the `weight` array. Compare those variants to `index3b`. Are `index4a` and `index4b` practical solutions?


## Top-k completion system #4 (case-insensitive version)

We have to add two more bit vectors to generate a system which is capable of matching case-insensitive but returns the original (case-sensitive) strings for our query.
This index is implemented in [index4ci][IDX4ci].

Compared to [index4][IDX4] we added three members to the class:
 * `m_str_start` is a bit vector which marks the starting positions of each string in the concatenation
 * `m_str_sel` is a select structure for `m_str_start` so that we access the start of the i-th string efficiently
 * `m_bv_uc` is a bit vector which indicates for each position in the concatenation if the character was in uppercase

 Apart from the reorganization of the construction (strings are first sorted case-insensitively, then concatenated, `m_bv_uc` constructed and the trie built over the lowercased concatenation) there are only minor changes of the code:
  * the pattern is lowercased before the matching
  * for each top-k entry, say the `i`-th string, we use `p=m_str_sel(i+1)` to go the the start of the `i`-th string in the concatenation and then check for each character  at position `j` of the string if it was written in uppercase in the original input by checking if `m_bv_uc[p+j]` was set. If yes, we uppercase the character at position `j` in the string.

### Exercise 4.b

With the default parametrization of the class `index4ci<>` takes about 427 MiB for the Wikipedia titles input. Try to optimze the space of the index by choosing better parameters. Keep in mind that the query times. They should not exceed 5 milliseconds. Call you new index `demo` and try the `demo-webserver` application.
A good solution should not require more than 330 MiB ;)

## Top-k completion system #5

[TODO: add description]




[SDSL]: https://github.com/simongog/sdsl-lite
[MONGOOSE]: https://github.com/cesanta/mongoose
[JQUERYAUTO]: https://https://github.com/devbridge/jQuery-Autocomplete
[IDXC]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index_common.hpp
[IDX1]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index1.hpp
[IDX2]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index2.hpp
[IDX3]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index3.hpp
[IDX4]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index4.hpp
[IDX4ci]: https://github.com/simongog/sigir16-topkcomplete/blob/master/include/topkcomp/index4ci.hpp
[MAIN]: https://github.com/simongog/sigir16-topkcomplete/blob/master/src/index.cpp
[IDXCFG]: https://github.com/simongog/sigir16-topkcomplete/blob/master/index.config
