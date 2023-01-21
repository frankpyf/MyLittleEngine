-- premake5.lua
workspace "MyLittleEngine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Editor"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"
include "Editor"