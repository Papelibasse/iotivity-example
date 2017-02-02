# IOTIVITY TUTORIAL #

URL: http://git.s-osg.org/iotivity-example


## ABOUT : ##

Minimal IoTivity examples, to get inspiration from.

This repo is a collection of standalone projects that are designed to be used out of iotivity main project. That said IoTivity project is shipping many samples app, you're welcome to check them, once you get familiar with basic projects provided in this repo.

Technically each project is stored in a git branch but this can be flattened into same directory using git submodules (index for walkthrough).


## USAGE: ##

### SETUP ###

Prelimary you need to build IoTivity for your system.
It has been tested with IoTivity 1.2.0 on GNU/Linux system (Debian).

* https://wiki.iotivity.org/build

### PREPARE ###

Then, you need to "unpack" all branches into current work directory:

    # 0: install git make
    make # 1:  pull all branches in src
    ls src # 2: explore all projects


## INDEX ##

* branch=example/master:
  * Shows discovery mechnism, can be used as skeleton for C++ projects
* branch=example/packaging/master: (on example/master)
  * integration for various OS: Tizen, Yocto, Debian
* branch=geolocation/master: (on example/packaging)
  * Shows Notify mechanism
* branch=switch/master: (on example/master)
  * Shows PUT mechanism 


## RESOURCES: ##

* https://wiki.iotivity.org/community
* https://wiki.iotivity.org/examples
* https://blogs.s-osg.org/category/iotivity/
* https://github.com/tizenteam/iotivity-example
* https://wiki.tizen.org/wiki/User:Pcoval
