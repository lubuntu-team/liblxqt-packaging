#=============================================================================
# Copyright 2015 Luís Pereira <luis.artur.pereira@gmail.com>
# Copyright 2015 Palo Kisa <palo.kisa@gmail.com>
# Copyright 2013 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

#-----------------------------------------------------------------------------
# Honor visibility properties for all target types.
#
# The ``<LANG>_VISIBILITY_PRESET`` and
# ``VISIBILITY_INLINES_HIDDEN`` target properties affect visibility
# of symbols during dynamic linking.  When first introduced these properties
# affected compilation of sources only in shared libraries, module libraries,
# and executables with the ``ENABLE_EXPORTS`` property set.  This
# was sufficient for the basic use cases of shared libraries and executables
# with plugins.  However, some sources may be compiled as part of static
# libraries or object libraries and then linked into a shared library later.
# CMake 3.3 and above prefer to honor these properties for sources compiled
# in all target types.  This policy preserves compatibility for projects
# expecting the properties to work only for some target types.
#
# The ``OLD`` behavior for this policy is to ignore the visibility properties
# for static libraries, object libraries, and executables without exports.
# The ``NEW`` behavior for this policy is to honor the visibility properties
# for all target types.
#
# This policy was introduced in CMake version 3.3.  CMake version
# 3.3.0 warns when the policy is not set and uses ``OLD`` behavior. Use
# the ``cmake_policy()`` command to set it to ``OLD`` or ``NEW``
# explicitly.
#-----------------------------------------------------------------------------
if(COMMAND CMAKE_POLICY)
    if (POLICY CMP0063)
        cmake_policy(SET CMP0063 NEW)
    endif()
endif()

include(CheckCXXCompilerFlag)

#-----------------------------------------------------------------------------
# Detect Clang compiler
#-----------------------------------------------------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(LXQT_COMPILER_IS_CLANGCXX 1)
endif()


#-----------------------------------------------------------------------------
# Set visibility to hidden to hide symbols, unless they're exported manually
# in the code
#-----------------------------------------------------------------------------
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)


#-----------------------------------------------------------------------------
# Disable exceptions
#-----------------------------------------------------------------------------
if (CMAKE_COMPILER_IS_GNUCXX OR LXQT_COMPILER_IS_CLANGCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()


#-----------------------------------------------------------------------------
# Common warning flags
#-----------------------------------------------------------------------------
set(__LXQT_COMMON_WARNING_FLAGS "-Wall")


#-----------------------------------------------------------------------------
# Warning flags
#-----------------------------------------------------------------------------
if (CMAKE_COMPILER_IS_GNUCXX OR LXQT_COMPILER_IS_CLANGCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${__LXQT_COMMON_WARNING_FLAGS}")
endif()


#-----------------------------------------------------------------------------
# Linker flags
# Do not allow undefined symbols
#-----------------------------------------------------------------------------
if (CMAKE_COMPILER_IS_GNUCXX OR LXQT_COMPILER_IS_CLANGCXX)
    set(CMAKE_SHARED_LINKER_FLAGS
        "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}"
    )
    set(CMAKE_MODULE_LINKER_FLAGS
        "-Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}"
    )
endif()


#-----------------------------------------------------------------------------
# CXX11 and CXX0X requirements
#-----------------------------------------------------------------------------
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. C++11 support is required")
endif()


#-----------------------------------------------------------------------------
# Enable exceptions for an target
#
# lxqt_enable_target_exceptions(<target>
#    <INTERFACE|PUBLIC|PRIVATE>
# )
#
#-----------------------------------------------------------------------------
function(lxqt_enable_target_exceptions target mode)
    target_compile_options(${target} ${mode}
        "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fexceptions>"
    )
endfunction()
