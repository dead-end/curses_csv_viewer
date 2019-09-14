# Installation from sources

In this document I build *ncurses* and *ccsvv* from its sources. This is necessary if your OS does not
provide an appropriate *ncuses* library. *ccsvv* uses the wide character libraries of *ncurses*, *form*
and *menu*, which might be not installed.

As a preparation we create an installation directory inside our home directory, so we can install our
programs without *root*.

```bash
mkdir -p $HOME/usr/local
```

## ncurses

The first step is to download the *ncurses* and extract the sources. At the time, writing this
document the highest version is: *ncurses-6.1.tar.gz*

```bash
wget https://invisible-mirror.net/archives/ncurses/ ncurses-6.1.tar.gz

gunzip ncurses-6.1.tar.gz

tar xvf ncurses-6.1.tar

cd ncurses-6.1/
```

The next step is to build the *ncurses*. We need the following build configuration:

Configuration                                 |Description
------------------------                      |-----------
--prefix=$HOME/usr/local                      |The directory where *ncurses* will be installed.
--enable-widec                                |Enable wide character support.
--with-shared                                 |Create shared lib ($HOME/usr/local/lib/libncursesw.so.6).
--disable-leaks                               |The flag is only necessary if you use valgrind.

The build is standard:

```bash
./configure --prefix=$HOME/usr/local \
              --enable-widec \
              --with-shared \
              --disable-leaks

make

make install
```

If you want to use the installed *ncurses* you have to set the *LD_LIBRARY_PATH* with
the install directory:

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/usr/local/lib
```

For the unit tests to sucessfully run, it is necessary to set a UTF8 locale. The command
`locale -a` lists all installed locales.

```bash
export LANG=C.UTF-8
```

## ccsvv

The next step is to download and extract *ccsvv*:

```bash

wget https://github.com/dead-end/curses_csv_viewer/archive/master.zip

unzip master.zip

cd curses_csv_viewer-master
```

To use the installed *ncurses*, we need to configure *ncursesw6-config* for the build:

```bash
make NCURSES_CONFIG=$HOME/usr/local/bin/ncursesw6-config
```

Now we can check to build result.

```bash
./ccsvv -d : /etc/passwd
```

## Uninstall and cleanup

To uninstall *ncurses* you have to call:

```bash
make uninstall
```

To cleanup *ccsvv* you have to call:

```bash
make clean
```

## Docker verification

The instalation can be verified with the docker file, which is based on a debian image:

[ncurses_src.dockerfile](ncurses_src.dockerfile)

To build the docker image and run the smoke test call:

```bash
sudo docker build -t ncurses_src -f ncurses_src.dockerfile docker/

docker run -it ncurses_src sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
```