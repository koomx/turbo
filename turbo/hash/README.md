# hash


## bm on mac
```text

M_Int_StdHash                0.292 ns        0.292 ns   2375272732
BM_Int_TurboHash              0.317 ns        0.317 ns   2195190025
BM_Int_XxHash64                2.33 ns         2.33 ns    300050580
BM_Int_Xxh3_64                 3.13 ns         3.13 ns    222868186
BM_String_StdHash/8            1.48 ns         1.48 ns    471647262 bytes_per_second=5.02403Gi/s
BM_String_StdHash/16           1.47 ns         1.47 ns    478429657 bytes_per_second=10.1596Gi/s
BM_String_StdHash/64           2.96 ns         2.96 ns    230887465 bytes_per_second=20.1413Gi/s
BM_String_StdHash/256          14.7 ns         14.7 ns     47473076 bytes_per_second=16.236Gi/s
BM_String_StdHash/1024         55.6 ns         55.5 ns     12615568 bytes_per_second=17.1769Gi/s
BM_String_StdHash/4096          218 ns          217 ns      3231137 bytes_per_second=17.5447Gi/s
BM_String_TurboHash/8          1.28 ns         1.28 ns    550465930 bytes_per_second=5.84322Gi/s
BM_String_TurboHash/16         1.49 ns         1.49 ns    468983445 bytes_per_second=10.0144Gi/s
BM_String_TurboHash/64         2.38 ns         2.38 ns    294610315 bytes_per_second=25.071Gi/s
BM_String_TurboHash/256        4.75 ns         4.75 ns    147425840 bytes_per_second=50.2215Gi/s
BM_String_TurboHash/1024       12.5 ns         12.5 ns     55923497 bytes_per_second=76.3466Gi/s
BM_String_TurboHash/4096       61.6 ns         61.5 ns     11396011 bytes_per_second=61.9974Gi/s
BM_String_XxHash64/8           2.48 ns         2.48 ns    281027436 bytes_per_second=3.00019Gi/s
BM_String_XxHash64/16          3.06 ns         3.06 ns    228057601 bytes_per_second=4.87027Gi/s
BM_String_XxHash64/64          6.42 ns         6.41 ns    108802089 bytes_per_second=9.29626Gi/s
BM_String_XxHash64/256         16.8 ns         16.8 ns     41869785 bytes_per_second=14.1648Gi/s
BM_String_XxHash64/1024        66.1 ns         66.0 ns     10573220 bytes_per_second=14.4412Gi/s
BM_String_XxHash64/4096         263 ns          263 ns      2648345 bytes_per_second=14.4962Gi/s
BM_String_Xxh3_64/8            3.29 ns         3.29 ns    213233905 bytes_per_second=2.26729Gi/s
BM_String_Xxh3_64/16           3.47 ns         3.46 ns    204153652 bytes_per_second=4.30195Gi/s
BM_String_Xxh3_64/64           5.74 ns         5.73 ns    121031883 bytes_per_second=10.4063Gi/s
BM_String_Xxh3_64/256          12.5 ns         12.5 ns     55896703 bytes_per_second=19.075Gi/s
BM_String_Xxh3_64/1024         49.4 ns         49.4 ns     14123439 bytes_per_second=19.3079Gi/s
BM_String_Xxh3_64/4096          239 ns          239 ns      2933080 bytes_per_second=15.9813Gi/s
```
