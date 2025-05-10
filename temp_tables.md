teensy avg 512 no bit mask - 788.3139 teensy avg 512 with & - 788.0976

same for 64

no bit mask - 13.1564 with & - 13.1000

with bitshifting:

64 - min: 16 max: 17 avg: 16.468939329294308 512 - min: 1006 max: 1009 avg:
1006.6255972696246 1024 - min: 3996 max: 3997 avg: 3996.5972696245735

with batching:

64/64: min: 43 max: 44 avg: 43.07607304377507 512/64: min: 2776 max: 2779 avg:
2776.0440273037543 512/256: min: 2663 max: 2667 avg: 2663.8293515358364 512/512:
min: 2663 max: 2664 avg: 2663.819453924915

1024/512: min: 10573 max: 10581 avg: 10573.757679180888 1024/1024: min: 10513
max: 10520 avg: 10513.176791808874

pico with batching:

1024/1024: min: 80801 max: 80823 avg: 80806.00068259386

iter5 some crazy, cursed code 4 unrolled bits:

64: min: 9 max: 12 avg: 9.701706484641639 512: min: 551 max: 555 avg:
551.6358361774744 1024: min: 2195 max: 2203 avg: 2195.19795221843

pico:

64: min: 68 max: 123 avg: 70.74020820889154 512: min: 3969 max: 4053 avg:
3978.192832764505 1024: min: 15826 max: 15918 avg: 15830.878498293516

teensy unroll x8:

1024: min: 2305 max: 2312 avg: 2305.086689419795

teensy unroll x16: 1024: min: 2303 max: 2311 avg: 2303.897610921502

teensy: 1 loop 64x unroll: min: 6 max: 7 avg: 6.6626845293967065

teensy unroll x64 no loops: min: 0 max: 1 avg: 0.04919361720283301

teensy pragma unroll: min: 0 max: 1 avg: 0.05013226384503797

add something to vector: min: 5 max: 6 avg: 5.72813379981227

add/set something to vector: min: 13 max: 14 avg: 13.404684700059732

pico no vector: min: 0 max: 9 avg: 0.2175767918088737

===== how about different datastructures? teensy

baseline vector: avg: 13.152146087550133

array: 0.42840686065363937

vector<uint8_t>: same as vector

unordered_map: avg: 31.509002474613876

bitset: 0.4136018431606792

with bitset and 128bit: avg: 179.87123474699206

with bitset and bitshifting optimization: 179.87149074153083
