# Serenity
A game engine focused on producing stunning visual effects and supporting custom user made rendering and logic code.

Features
-------------------------------------
> * Physically based rendering using deferred rendering
> * Several post-processing effects (Depth of Field, HDR, SSAO, Bloom, Fog, God Rays, FXAA / SMAA based Anti-aliasing)
> * Entity-Component based game logic
> * Custom logic and rendering code via functors
> * Bullet Physics world
> * Multi-threading using boost::asio worker pool and functor based jobs
> * Resource loading (3d meshes, textures, sounds, fonts)
> * Optimized render calls using Render Graphs

Installing & Building - Visual Studio
-------------------------------------

The project uses several library dependencies. The current solution file is designed to be used in Visual Studio 2019 with the libraries being built statically and not dynamically, on a Windows OS. Make sure to link the dependencies and their header files. The dependencies are: 
> * Assimp
> * Bullet Physics
> * SFML
> * freeglut
> * GLEW
> * glm
> * Boost (filesystem, iostreams, systems)
> * LUA

The solution contains 3 projects: the engine itself, which will be built into Serenity.lib, and 2 sample applications that will be built into .exe's.

In order for the exe's to run, the `SerenityLib/Engine/data` folder will have to be copied over to the parent directory of any exe, which are located in the builds folder and will be generated upon compiling the solution.

In the future, the engine will receive an overhaul with the intention of opening it up to client usage. Part of this endeavour will be a wiki section with API documentation as well as continued refactoring of the engine code for readability and understanding.
