# Serenity
A game engine focused on producing stunning visual effects and supporting custom user made rendering and logic code.

Note
----
* This project is currently in the early stages of development and is therefore not release worthy just yet.

Installing & Building - Visual Studio
-------------------------------------

1. Clone this repository

2. Download [Boost](http://www.boost.org/users/history/version_1_64_0.html). I personally put it into `C:/boost/`

3. Modify the include directory for boost. In Visual Studio, Project > Serenity Properties...
  - Make sure Configuration is set to **All Configurations**.
  - Expand the C/C++ Tab. Select General.
  - Modify the Additional Include Directories to include your directory to boost.

4. Build the boost libraries. The project uses boost's filesystem, iostreams, and system library. For more information on how to build boost, visit [The getting started guide](http://www.boost.org/doc/libs/1_64_0/more/getting_started/windows.html)
  - The project expects the /MT Static version of the library to be built. `./b2 link=static runtime-link=static variant=debug,release threading=single,multi`. Your version of Visual Studio will determine what toolset version you will use. Example: VS 2012 requires `toolset=msvc-11.0` added to the ./b2 build command.

5. Modify the linker to include the boost library directories. Normally they are built in your directory of boost under the stage/lib folder. Example: `C:/boost/stage/lib`. In Visual Studio, Project > Serenity Properties...
  - Make sure Configuration is set to **All Configurations**.
  - Expand the Linker. Select General.
  - Modify the Additional Library Directories to include your directory to the boost libraries.

6. All other dependencies are included in the dependencies folder and properly linked to the visual studio project. The other dependencies are SFML, GLEW, Bullet Physics, Assimp, and GLM.

7. Build. In Visual Studio, hit Ctrl-F5. Debug builds will be in the Debug folder, Release builds will be in the Release folder.
