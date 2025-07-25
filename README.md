# fred-tester

## Overview
`fred-tester` is a tool for testing the FIT FRED system. It sends commands to MAPI topics and verifies the responses, and also tracks the responses from looping MAPI topics. 
It displays the results in its logs only - it is not fully automatic, since the appearance of some errors in the logs is expected due to some timing mismatches (to be improved).
The tests are always run in a predefined sequence - `.toml` configuration files are used to configure only which tests are run for which boards.
`fred-tester` can run the tests once, or be launched as a DIM server, awaiting requests for tests.

## Build instructions
Clone this repository. Then run
```
git submodule update --init
mkdir build && cd build
cmake ..
make
```
Note that at `gcc-13` is the minimum compiler version. DIM is also required, in `/opt/dim` or `/usr/local`.

## Running
```
./fred-tester [-d/--debug] dim
./fred-tester [-d/--debug] single [config-file]
```
The debug option enables verbose logs.

It might be necessary to have `LD_LIBRARY_PATH` defined to include DIM.

## Configuration file format
All the fields are provided in the `full.toml` configuration file. From there, you can disable specific tests.

Tests which are run on the whole FEE are represented by boolean fields. Tests run on specific boards can either be boolean (if true, run on all connected boards) or a list of boards to run on. `bad_channel_map` can be set to false.

## DIM interface
The DIM interface is the same as the `.toml` configuration file format.
Requests are to be sent to `FRED_TESTER/CONTROL/RpcIn`, and a short response will be published on `FRED_TESTER/CONTROL/RpcOut`.
`FRED_TESTER/LOG` sends the test logs line by line, and `FRED_TESTER/BAD_CHANNEL_MAP` publishes the bad channel map.

## Test procedure (full)
1. START sent to MANAGER
2. Sleep - 1 second; STATUS tracking for 2s (to check PM_MASK_SPI - expect errors related to readiness changed)
3. RESET_SYSTEM
4. Sleep - 1 second
5. RESET_ERRORS
6. Sleep - 1 second
7. Start STATUS Monitors
8. Load configuration
9. Sleep - 2.5 seconds
10. Wait for attenuator
11. Start COUNTER_RATES Monitoring
12. TCM PARAMETERS (`GBT_EMULATED_TRIGGERS_PATTERN_*`)
13. Sleep - 1 second
14. PM PARMETERS (`GBT_EMULATED_TRIGGERS_PATTERN_*`)
15. Sleep - 5 seconds
16. Stop All Histograms
17. Sleep - 2 seconds
18. TCM Histograms - single read with no selected counter
19. TCM Histograms - single read with selected counter 1
20. TCM Histograms Tracking 
    - Sleep - 5 seconds
21. PM Histograms - single empty read
22. PM Histograms - single with ADC0
23. PM Histograms - single with ADC1
24. PM Histograms - single with TIME
25. PM Histograms Tracking (TIME only)
    - Sleep - 5 seconds
    - RESET
    - Sleep - 5 seconds
26. Main sleep
27. Stop COUNTER_RATES monitoring and display data, publish bad channel map
28. Change read interval (expect warnings for no counters/rates):
    - Sleep - 10ms
    - Change Read Interval to 3 (0.5s)
    - Start monitoring (0.25s interval)
    - Stop monitoring
    - Sleep - 10ms
    - Reset Read Interval to 4 (1s)
    - Restart Monitoring (0.5s interval)
29. Reset counters (expect warnings for no counters/rates):
    - Sleep - 5 seconds
    - Reset TCM Counters
    - Sleep - 5 seconds
    - Reset PM Counters
    - Sleep - 5 seconds
30. Cleanup - apply configuration, reset errors
