# nfwk
Lightweight framework for C++ games and applications.
Requires minimal code to get a simple application up and running.

# Building
The project can be generated with [Cmake](https://cmake.org/download).
You can use ``project/vs16.bat`` on Windows to quickly create the folder ``project/vs/nfwk.sln``.

You must copy ``default-config.hpp`` as ``config.hpp``, and enable the modules you want to compile. The default config should be fine in most cases.

# Usage
For some examples on usage, see:
* [Inmate](https://github.com/noctare/ludum-dare-45) - game made in 3 days for Ludum Dare 45
* [Einheri](https://github.com/sebastsg/ntnu-bachelor) - online 3D game for my bachelor project
* [Milky Tags](https://github.com/sebastsg/milky-tags) - file tagging software (with lots of [ImGui](https://github.com/ocornut/imgui) use)

# Platforms
Only Windows is currently supported. Linux will be supported in the future.
