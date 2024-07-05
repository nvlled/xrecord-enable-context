In here is a sample C program to reproduce
a possible xlib bug on linux with
XRecordEnableContext.

## Bug description
CPU usage gradually increases
over time as the mouse is moved.

## Expected behaviour
CPU usage should remain low and constant.


1. build and run file
```sh
$ ./build.sh && ./main
```

2. (In another terminal) Monitor process called main
```sh
$ top -p $(pgrep main)
```

3. Keep moving the mouse and observe how
the CPU usage gradually increases
over time.

