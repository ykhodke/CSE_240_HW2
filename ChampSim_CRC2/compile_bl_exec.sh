# # LRU baseline
# g++ -Wall --std=c++11 -o lru-config1 example/lru.cc lib/config1.a
# g++ -Wall --std=c++11 -o lru-config2 example/lru.cc lib/config2.a

# # Ship++ baseline
# g++ -Wall --std=c++11 -o ship_pp-config1 src/ship++.cc lib/config1.a
# g++ -Wall --std=c++11 -o ship_pp-config2 src/ship++.cc lib/config2.a

# # Multiperspective baseline
# g++ -Wall --std=c++11 -o multp-config1 src/multiperseptive.cc lib/config1.a
# g++ -Wall --std=c++11 -o multp-config2 src/multiperseptive.cc lib/config2.a

# # Lime baseline
# g++ -Wall --std=c++11 -o lime-config1 src/lime.cc lib/config1.a
# g++ -Wall --std=c++11 -o lime-config2 src/lime.cc lib/config2.a

# RED baseline
g++ -Wall --std=c++11 -o red-config1 src/red.cc lib/config1.a
g++ -Wall --std=c++11 -o red-config2 src/red.cc lib/config2.a