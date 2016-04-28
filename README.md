# Cinth

a simple, 1bit synthesizer written for PIC16FXXX in C.
compiled using [SDCC](http://sdcc.sf.net/).

unfortunately, cpu speed is limited to 20MHz on PIC16F627A(which is the MCU i am using) and polyphony is not usable yet. maybe it will be available in future.

## Compile

clone repo, install sdcc and gputils and type:

```
$ make compile
```

## Program

install picprog and:

```
$ make program # or just make
```

## Simulation

install gpsim:

```
$ make simulate
```
