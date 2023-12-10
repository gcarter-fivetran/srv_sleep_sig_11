# srv_sleep_sig_11
Demonstrates a signal 11 in CT-Lib's call to `srv_sleep()`.

The platform upon which I ran this demonstration was Ubuntu Linux running on my Dell laptop:
```
# uname -a
Linux lt-gcarter 5.15.0-86-generic #96-Ubuntu SMP Wed Sep 20 08:23:49 UTC 2023 x86_64 x86_64 x86_64 GNU/Linux
```

For me, this consistently produces the following segmentation fault stack trace:
```c
srv_sleep 0x00007ffff7c53671
sybnet.complete_io 0x00007ffff7ed60f5
np_io_read 0x00007ffff7f77898
ct.tds_recvpkt 0x00007ffff7f8dfc1
ct.tds_get_bytes 0x00007ffff7f8d107
ct.tds_read_token 0x00007ffff7f8dabe
ct_async_exec_stack 0x00007ffff7f38f2d
ct_connect 0x00007ffff7f44593
ex_connect exutils.c:770
main srv_sleep_sig_11.c:217
__libc_start_call_main 0x00007ffff7a01d90
__libc_start_main_impl 0x00007ffff7a01e40
_start 0x00005555555568e5
```

Note that there is a call to `sleep(5)` in the Open Server start handler, `start_handler()` which is commented. I have noticed that when I uncomment this line, the signal 11 does not occur.

