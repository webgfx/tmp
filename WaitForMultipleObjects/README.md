# WaitForMultipleObjects Bug Test

## Purpose
This test is designed to catch a rare bug on Qualcomm Windows platforms where `WaitForMultipleObjects()` can incorrectly return `WAIT_TIMEOUT` even when called with `INFINITE` timeout and `nCount=1`.

## The Bug
When calling `WaitForMultipleObjects()` with:
- `nCount = 1`
- `dwMilliseconds = INFINITE`

On rare occasions, the function returns immediately with `WAIT_TIMEOUT` instead of waiting indefinitely for the event to be signaled.

## Test Approach
The test creates an event and a worker thread that signals the event after a 100ms delay. It then calls `WaitForMultipleObjects()` with `INFINITE` timeout. Since the event will eventually be signaled, the function should NEVER return `WAIT_TIMEOUT`.

The test runs many iterations (default: 100,000) to increase the chances of catching this rare bug.

## Building

### Using Visual Studio Build Tools:
```batch
build.bat
```

### Using CMake:
```batch
mkdir build
cd build
cmake ..
cmake --build .
```

### Manual compilation:
```batch
cl /EHsc /W4 /O2 WaitBugTest.cpp
```

## Running the Test
```batch
WaitBugTest.exe
```

The test will:
1. Display system information (processor architecture)
2. Run many iterations of the wait test
3. Report progress every 10,000 iterations
4. Immediately report any bug occurrences with details
5. Display final statistics

## Interpreting Results
- If the bug is caught, you'll see "!!! BUG DETECTED !!!" messages with timing information
- The test returns exit code 1 if bugs were found, 0 otherwise
- Final statistics show the bug rate as a percentage

## Customization
You can modify these constants in the source code:
- `MAX_ITERATIONS`: Number of test iterations (default: 100,000)
- `REPORT_INTERVAL`: Progress reporting frequency (default: 10,000)
- Worker thread delay in `SignalThread()` (default: 100ms)

## Platform Notes
This bug has been observed on Qualcomm ARM64 Windows devices. The test will display the processor architecture at startup to help identify the platform where the bug occurs.
