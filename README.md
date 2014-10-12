yasp
====

Yet Another Serial Protocol

Because all the world needs is another new protocol that is nearly the same as
every other one.  Intended to be lightweight with dependency injection so that it can be dropped
into any firmware or software project.

Building with CMake
====

To build the project you will need CMake and gcc. In Ubuntu Linux run the
following to install:

```
$ sudo apt-get install build-essential
$ sudo apt-get install cmake
```

Then, to actually build the code you will need to run the following:

```
$ git clone https://github.com/mjbrown/yasp_c.git
$ cd yasp_c
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Subsequent builds can be performed by simply running `make` from the build
directory.

Unit tests are written using the Unity framework and can be run using
automatically generated make targets.

First run `make` from the `build` directory to make sure the latest code is
built.

```
$ make
```

Then run `make test` to run the tests:

```
$ make test
```
