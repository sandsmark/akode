/*  aKode: PluginHandler

    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "akodelib.h"
#include "pluginhandler.h"
#include <dirent.h>
#include <iostream>

#ifdef HAVE_LIBLTDL
  #include <ltdl.h>
#else
  #include <dlfcn.h>
#endif

#ifdef HAVE_LIBLTDL
  #define DLCLOSE(x) lt_dlclose((lt_dlhandle)(x))
  #define DLERROR() lt_dlerror()
  #define DLSYM(x, y) lt_dlsym((lt_dlhandle)(x), (y))
  #define DLINIT() lt_dlinit()
  #define DLEXIT() lt_dlexit()
#else
  #define DLCLOSE dlclose
  #define DLERROR dlerror
  #define DLSYM dlsym
  #define DLINIT()
  #define DLEXIT()
#endif


namespace aKode {

PluginHandler::PluginHandler() : library_loaded(false), handle(0)
{
    DLINIT();
  #ifdef HAVE_LIBLTDL
    lt_dlsetsearchpath(AKODE_SEARCHDIR);
  #endif
}

PluginHandler::PluginHandler(const string lib) : library_loaded(false), handle(0)
{
    DLINIT();
    if (lib.size() > 0)
        load(lib);
}

PluginHandler::~PluginHandler()
{
    unload();
    DLEXIT();
}

list<string> PluginHandler::listPlugins() {
    DIR *dir = opendir(AKODE_SEARCHDIR);
    dirent *dp;

    if (!dir) return list<string>();

    list<string> out;

    while ((dp = readdir (dir)) != 0) {
        string name(dp->d_name);
        if (name.length() < 15) continue;
        if (name.substr(0,9) != "libakode_") continue;
  #ifdef HAVE_LIBLTDL
        int p = name.find(".la", 9);
  #else
        int p = name.find(".so", 9);
  #endif
        if (p == -1) continue;
        string plugin = name.substr(9,p-9);
        out.push_back(plugin);
    }
    return out;
}

bool PluginHandler::load(const string lib)
{
    if (library_loaded) return false;

  #ifdef HAVE_LIBLTDL
    const string library = string("libakode_") + lib;
    handle = lt_dlopenext(library.c_str());
  #else
    string library = "libakode_" + lib + ".so";
    handle = dlopen(library.c_str(), RTLD_NOW);
    if (!handle) { // this seems the simplest way to extent the search-path
        library = string(AKODE_SEARCHDIR) +"/"+ library;
        handle = dlopen(library.c_str(), RTLD_NOW);
    }
  #endif
    if (!handle) {
        return false;
    }

    library_loaded = true;
    return true;
}

void PluginHandler::unload()
{
    if (library_loaded) {
        DLCLOSE(handle);
        library_loaded = false;
    }
}

void* PluginHandler::loadPlugin(const string name) {
    void *plugin;
    if (!handle) return 0;

    DLERROR(); // clear errors
    plugin = DLSYM(handle, name.c_str());
    if (DLERROR()) return 0;

    return plugin;
}

} //namespace
