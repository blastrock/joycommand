============
 joycommand
============

What is it?
===========

joycommand is a small program that will translate game controllers' events to
shell commands.

Why?
====

I sometimes watch movies or shows on my computer, which is connected to the TV.
When I watch something on the TV, I sit on the sofa, which is far from my
computer and keyboard. But sometimes, I need to pause the video, rewind it a
little because I missed something, or skip the opening of the show.

For that, I use a game controller (a DualShock 4 connected in BlueTooth, but
joycommand works with anything supported by SDL). This lets me control the
playback without moving from my sofa, as you would do with a remote control.

How to build?
=============

You need to install a C++ compiler, meson, ninja, yaml-cpp and SDL2:

.. code-block:: shell

    apt install build-essential meson ninja-build libyaml-cpp-dev libsdl2-dev

Then you can compile the project:

.. code-block:: shell

    meson build
    cd build
    ninja

Finally, you can run joycommand:

.. code-block:: shell

    cd ..
    ./build/joycommand

It will load the default configuration file named config.yaml.

Author
======

joycommand is written by Philippe Daouadi.

License
=======

joycommand is distributed with the BSD 2-clause license.
