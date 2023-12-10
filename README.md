# srv_sleep_sig_11
Demonstrates a signal 11 in CT-Lib's call to `srv_sleep()`.

The platform upon which I ran this demonstration was Ubuntu Linux running on my Dell laptop:
```
# uname -a
Linux lt-gcarter 5.15.0-86-generic #96-Ubuntu SMP Wed Sep 20 08:23:49 UTC 2023 x86_64 x86_64 x86_64 GNU/Linux
```

Note that there is a call to `sleep(5)` in the Open Server start handler, `start_handler()`. I have noticed that when I uncomment this line, the signal 11 does not occur.

