cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS ${PWD}/components)

set(COMPONENT_SRCS "src/mesh_framework.c")

set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_PRIV_REQUIRES main)

register_component(meshino_framework)

component_compile_options(PUBLIC -DMPU_COMPONENT_TRUE=1)
