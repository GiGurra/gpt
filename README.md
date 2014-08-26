gpt
===

Gurra's Pit Tools (toolkit for bms flight sim enthusiasts).

GPT accomplishes the following:
  * Streaming Falcon4 BMS cockpit displays to remote rendering computers
  * Mirroring shared memory to remote systems for handing off responsibility of avionics rendering
  * Remote keyboard implementation to forward emulated keyboard input from one computer to another

Here's some demo footage of what you can use it for:

https://www.youtube.com/watch?v=X1YIXiCxA9w&list=UU4iUXqWGxAm5ojkxkq6gljw
https://www.youtube.com/watch?v=AlKxqR_7_Cc&list=UU4iUXqWGxAm5ojkxkq6gljw


[Discussion Thread](http://www.benchmarksims.org/forum/showthread.php?10677-Beta-Release-GPT-(cockpit-texture-extraction-remote-cockpit-control-shm-mirror))

Running GPT depends on:
  * Qt runtime libraries
  * Java JVM (1.7 or higher)
  * Visual Studio C++ Redistributable 2013
  * Libjpeg-turbo

Building GPT depends on:
  * https://github.com/culvertsoft/mgen
  * https://github.com/culvertsoft/mnet
  * Qt
  * Java JDK 1.7
  * VS2013
  * MS Detours
  * Python
  * JNA
  * Libjpeg-turbo/turbojpeg
  * CMake (3)

GPT is released under GPL v2.
