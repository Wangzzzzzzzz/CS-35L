Wang, Zheng (404855295)
CS 35L readme.txt

(1) Issues

At first, I forgot to change the step size of the outer most for loop in my 'parallel_render'
function, causing my multithreaded program to repeat running the same thing over and over
again so that the performance with more threads fail does not improve the real run time. (Of
Course, the version I submitted has this issue fixed)


Also, I did not use 'return NULL;' at the end of my 'parallel_render' function and this cause
a warning from the system. I added the 'return' statement and fix the issue.


When I am testing the optimal multithreaded performance, I use the maximum threads that my
program will use (i.e. width = 131).

Commands:
    time ./srt 131 >/dev/null

Output:
    Creating thread failure.
    real	0m0.006s
    user	0m0.046s
    sys	0m0.004s

#   This issue exist until I use a thread number less or equal to 93

Commands:
    time ./srt 93 >/dev/null

Output:
    real	0m5.670s
    user	1m26.608s
    sys	0m0.005s

#   I modify my code so that it can print out the error code. (The output of the make-log.txt comes
    from the modified version here.)

Commands:
    time ./srt 100 >/dev/null

Output:
Creating thread failure, exit number is 11
real	0m0.005s
user	0m0.035s
sys	0m0.004s

#   I searched the web for this error code and find that this is due to the limitation of
    the resources allocated by kernels. (not due to the bug in my code)

-----------------------------------------------------------------------------------------------------------------------------

(2) Performance Test:

The following Commands are run to test the performance.

Commands:
    time ./srt 1 >/dev/null
    time ./srt 2 >/dev/null
    time ./srt 4 >/dev/null
    time ./srt 8 >/dev/null
    time ./srt 16 >/dev/null
    time ./srt 32 >/dev/null
    time ./srt 64 >/dev/null
    time ./srt 93 >/dev/null

Result summarized below:

 thread         Real         User          Sys
--------    -----------  -----------  ------------
   1         0m47.953s    0m47.943s     0m0.005s
   2         0m23.991s    0m47.700s     0m0.009s
   4         0m12.276s    0m48.498s     0m0.002s
   8         0m6.509s     0m50.231s     0m0.003s
   16        0m5.658s     1m26.515s     0m0.005s
   32        0m5.671s     1m26.471s     0m0.007s
   64        0m5.822s     1m26.424s     0m0.038s
   93        0m5.674s     1m26.619s     0m0.004s


#   Conclusion: The multithreading does improve the performance of the program. However, starting from
    16 threads, the performance improvement is no longer obvious and it seems that my program can in
    maximum lower the real running to about 1/9 of the single-threaded program.

    The time spend on system call is all very little. But the total user time has increased. I think this
    is due to the extra time needed to ensure synchronization of so many threads and time needed to run
    extra steps in order to create and join the threads.
