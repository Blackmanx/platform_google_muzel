# JavaHeapProfTest

The JavaHeapProfTest is an aspirational test for the Java heap profiling
implementation. The current implementation of Java heap profiling passes the
test most of the time, but not every time due to randomness and minor biases
in the sampling.

This test is not part of ART run-tests because it depends on the real version
of `heapprofd_client_api.so` to run. It is not run under continuous
integration because it is too flaky at the moment. To make use of this test,
you can run it manually when making significant changes to the Java heap
profiling code.

To run the JavaHeapProfTest manually, for example:
```sh
atest --iterations 10 JavaHeapProfTest
```

The test should pass on a decent number of the iterations. The test logs a
summary of its findings to device logcat when it runs. Here's an example
passing case:

```
<name>:   expected     actual     normal        Δ abs    Δ rel Δ stddev
 total: 1652326400 1652356160 1652356160        29760     0.00     0.01
    S0:     102400      98304      98302        -4098    -0.04    -0.20
    S1:     102400      69632      69630       -32770    -0.32    -1.60
    S2:     102400      94208      94206        -8194    -0.08    -0.40
    S3:     102400     122880     122877        20477     0.20     1.00
    M0:  102400000  101766144  101764311      -635689    -0.01    -0.98
    M1:  102400000  102368256  102366412       -33588    -0.00    -0.05
    M2:  102400000  102005760  102003922      -396078    -0.00    -0.61
    M3:  102400000  103330816  103328954       928954     0.01     1.43
    N0:  307200000  306942248  306936719      -263281    -0.00    -0.23
    N1:  307200000  306694488  306688964      -511036    -0.00    -0.46
    N2:  307200000  308225808  308220256      1020256     0.00     0.91
    N3:  307200000  307124912  307119380       -80620    -0.00    -0.07
    B0:   13107200   13107200   13106963         -237    -0.00    -0.00
    E0:     102400      81920      81918       -20482    -0.20    -1.00
    E1:     102400     110592     110590         8190     0.08     0.40
    E2:     102400     102400     102398           -2    -0.00    -0.00
    E3:     102400     110592     110590         8190     0.08     0.40
```

The first column corresponds to allocation sites in `JavaHeapProfTest.java`.
The `expected` column is the number of bytes actually allocated at those call
sites. The `actual` column is the number of bytes randomly sampled by the
profiler. Small differences between `expected` and `actual` are expected,
because the profiler uses random sampling.

The `normal` column scales the `actual` results to match the `expected` total
number of sampled bytes. This makes it easier to see if profiling is off in
magnitude but still produces reasonable relative counts for different call
sites.

The `Δ abs`, `Δ rel`, and `Δ stddev` columns show how far off the `normal`
results are from the `expected` results, in terms of absolute difference,
relative differences, and standard deviation from what's expected given the
randomness of sampling. The test is marked as failing if anything is more than
3 standard deviations from what is expected. If the implementation of Java
heap profiling was perfect (it's not), we would expect the test to pass 99.7%
of the time.

Here's an example failure you might get that's not too worrying, because the
only issue is N0 is slightly out of bounds in terms of `Δ stddev`:
```
STACKTRACE:
java.lang.AssertionError: Expected results not within 3.00 stddev of expected:
   <name>:   expected     actual     normal        Δ abs    Δ rel Δ stddev
   total: 1652326400 1652368296 1652368296        41896     0.00     0.02
      S0:     102400     106496     106493         4093     0.04     0.20
      S1:     102400      94208      94205        -8195    -0.08    -0.40
      S2:     102400      81920      81917       -20483    -0.20    -1.00
      S3:     102400      69632      69630       -32770    -0.32    -1.60
      M0:  102400000  100643840  100641288     -1758712    -0.02    -2.72
      M1:  102400000  103646296  103643668      1243668     0.01     1.92
      M2:  102400000  103140352  103137736       737736     0.01     1.14
      M3:  102400000  102438912  102436314        36314     0.00     0.06
      N0:  307200000  303657472  303649772     -3550228    -0.01    -3.17
      N1:  307200000  308594432  308586607      1386607     0.00     1.24
      N2:  307200000  308101464  308093652       893652     0.00     0.80
      N3:  307200000  308317432  308309614      1109614     0.00     0.99
      B0:   13107200   13107200   13106867         -333    -0.00    -0.00
      E0:     102400      94208      94205        -8195    -0.08    -0.40
      E1:     102400      86016      86013       -16387    -0.16    -0.80
      E2:     102400     114688     114685        12285     0.12     0.60
      E3:     102400      73728      73726       -28674    -0.28    -1.40
```

Here is an example of an egregious failure that indicates something is
definitely wrong:
```
STACKTRACE:
java.lang.AssertionError: Expected results not within 3.00 stddev of expected:
   <name>:   expected     actual     normal        Δ abs    Δ rel Δ stddev
   total: 1652326400  512235520  512235520  -1140090880    -0.69  -438.29
      S0:     102400      81920     264250       161850     1.58     7.90
      S1:     102400      29696      95790        -6610    -0.06    -0.32
      S2:     102400      20480      66062       -36338    -0.35    -1.77
      S3:     102400      18432      59456       -42944    -0.42    -2.10
      M0:  102400000   10512384   33909967    -68490033    -0.67  -105.77
      M1:  102400000   10736640   34633353    -67766647    -0.66  -104.65
      M2:  102400000   10577920   34121367    -68278633    -0.67  -105.44
      M3:  102400000   10481664   33810873    -68589127    -0.67  -105.92
      N0:  307200000  114229248  368471132     61271132     0.20    54.63
      N1:  307200000  114622464  369739535     62539535     0.20    55.76
      N2:  307200000  114736128  370106183     62906183     0.20    56.09
      N3:  307200000  114616320  369719716     62519716     0.20    55.74
      B0:   13107200   11534336   37206494     24099294     1.84   104.02
      E0:     102400       4096      13212       -89188    -0.87    -4.36
      E1:     102400      10240      33031       -69369    -0.68    -3.39
      E2:     102400      13312      42940       -59460    -0.58    -2.90
      E3:     102400      10240      33031       -69369    -0.68    -3.39
```
