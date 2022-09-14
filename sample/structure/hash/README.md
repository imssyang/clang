# Hash Function 哈希函数

```
map[lies] = J:
                                   hash  value
                                    6      G
                                    7      H
              l   i   e   s         8      I
   'lies' -> 108+105+101+115=429 -> 9      J
                 429%30 = 9         10     K
    KEY          HASH_FUNC          11     L
                                    12     M
                                    13     N
                                    14     O
                                    15     P
```

# Hash Collisions 哈希碰撞

```
map[lies] = J:
map[joes] = JJ:

                                   hash  value
                                    6     *p
                                    7     *p
              l   i   e   s         8     *p     lies      foes
   'lies' -> 108+105+101+115=429 -> 9     *p ->   J         JJ
   'foes' -> 108+105+101+115=429    10    *p    *next ->  *next
              f   o   e   s         11    *p
                 429%30 = 9         12    *p
    KEY        HASH_COLLISIONS      13    *p
                                    14    *p
                                    15    *p
```

# HashMap vs TreeMap

HashMap使用hashtable存储，访问O(1)但是乱序存储。
TreeMap使用binary-search-tree存储，访问O(logN)但是有序存储。

```
Python:
dict  -> Hashtable

Java:
HashMap
TreeMap

C++:
std::unordered_map -> Hashtable
std::map           -> Binary-search-tree

C#:
Dictionary<TKey, TValue>  -> Hashtable
StringDictionary
SortedDictionary
```

# HashSet vs TreeSet

类似HashMap与TreeMap;




