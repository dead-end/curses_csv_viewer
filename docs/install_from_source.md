# Installation from sources

In this document I build *ncurses* and *ccsvv* from its sources. This is necessary if your OS does not
provide an appropriate *ncuses*.

As a preparation we create an installation directory inside our home directory, so we can install our
programs without root.

```
# mkdir -p $HOME/usr/local
```

## ncurses

The first step is to download the *ncurses* sources for:

*https://invisible-mirror.net/archives/ncurses/*

For this document I used: *ncurses-6.1.tar.gz*. The next step is to extract the sources.


```
# gunzip ncurses-6.1.tar.gz

# tar xvf ncurses-6.1.tar

# cd ncurses-6.1/
```

The next step is to build the *ncurses*. We need the following build configuration:

Configuration|Description
-------------|-----------
--prefix=$HOME/usr/local|The directory where *ncurses* will be installed
--enable-widec|Enable wide character support
--with-shared|Create shared lib ($HOME/usr/local/lib/libncursesw.so.6)

The build is standard:

```
# ./configure --prefix=$HOME/usr/local --enable-widec --with-shared

# make

# make install
```

If you want to use the installed *ncurses* you have to set the `LD_LIBRARY_PATH` with
the install directory:

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/usr/local/lib
```

## ccsvv

The next step is to download and extract `ccsvv` from: `https://github.com/dead-end/curses_csv_viewer` 


```
# unzip curses_csv_viewer-master.zip

# cd curses_csv_viewer-master
```
To use the installed *ncurses*, we need to configure `ncursesw6-config` for the build:

```
make NCURSES_CONFIG=$HOME/usr/local/bin/ncursesw6-config
```

Now we can check to build result.

```
./ccsvv res/lang.csv
```

## Uninstall and cleanup

To uninstall `ncurses` we have to call:

```
# make uninstall
```

To cleanup `ccsvv` we have to call:

```
make clean
```
