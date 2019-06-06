# NXP trace daemon

```
CROSS_COMPILE=/opt/toolchains/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
${CROSS_COMPILE}gcc -Wall -o trace_get trace_get.c
```
sudo cp trace_get /srv/nfs/buildroot/imx8mm/usr/local/bin/trace_get

# CTF file generation

* Use Python3 version 3.6 or latter

Convert traces with below command:
* tools/trace_bin2lltng.py trace.bin trace.ctf

* Import CTF folder under TraceCompass

# Install TraceCompass

Some documentation can be found here: https://github.com/tuxology/tracevizlab/tree/master/labs
