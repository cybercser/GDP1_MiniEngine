# MiniEngine for GDP1

MiniEngine is a custom game engine made for the [Game Development – Advanced Programming (GDP1)](https://www.fanshawec.ca/programs/gdp1-game-development-advanced-programming/next) program at Fanshawe College. It's designed and deveoped from what we learned from this program and for the final project - a first-person zombie shooter game.

This project follows the paradigm advocated by _The Cherno_ in his tutorial video ["How to Properly Setup C++ Projects"](https://www.youtube.com/watch?v=5glH8dGoeCA). I also choose to organise the renderng code by learning from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL).

## Getting Started
This project is built using [premake](https://premake.github.io/) and Visual Studio 2022. You don't have to install premake. It's already included in this project.

To build and run this project, type in the following commands:
```bash
cd Scripts
.\Setup-Windows.bat
```
Voila. You will see a Visual Studio solution named `MiniEngine.sln`. Open it with Visual Studio 2022, then press F5 to build and run.

## User Controls
W, S, A, D - Camera Movement

## References
I learned a lot from the professors of GDP1 as well as the following online tutorials:

For OpenGL rendering techniques:

1. [LearnOpenGL](https://learnopengl.com/)
2. [OGLdev](https://ogldev.org/)
3. [OpenGL 4 Shading Language Cookbook, 3rd Edition](https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition)

For physics techniques:
1. [Gaffer On Games](https://gafferongames.com/categories/game-physics/)
2. [Game Physics in One Weekend](https://gamephysicsweekend.github.io/)
3. [Collision Detection with SAT (Math for Game Developers)](https://www.youtube.com/watch?v=-EsWKT7Doww)

For architecture and design patterns
1. [How to make a simple entity-component-system in C++](https://www.david-colson.com/2020/02/09/making-a-simple-ecs.html)
2. [Game Engine Architecture](https://www.gameenginebook.com/)
3. [Game Programming Patterns](https://gameprogrammingpatterns.com/)