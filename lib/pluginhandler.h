/*  aKode: PluginHandler parent-type

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _AKODE_PLUGINHANDLER_H
#define _AKODE_PLUGINHANDLER_H

#include <string>
#include <list>

#include "akode_export.h"

namespace aKode {

using std::string;
using std::list;

//! The parent class of all plugin-handlers

/*!
 * Handles the loading and unloading of plugins, must be inherited from by all
 * specialized plugins-handlers.
 */
class AKODE_EXPORT PluginHandler {
public:
    static list<string> listPlugins();

    /*!
     * Default constructor that loads no library.
     */
    PluginHandler();
    /*!
     * The constructor loads the akode-library name \a lib.
     */
    PluginHandler(const string lib);
    /*!
     * The destructor unloads the library.
     */
    virtual ~PluginHandler();
    /*!
     * Try to load the library \a lib
     * Returns true if succesfull.
     */
    virtual bool load(const string lib);
    /*!
     * Unloads the library
     */
    virtual void unload();
    /*!
     * Ask for a plugin in the library named \a plugin.
     * Returns 0 on failure.
     */
    void* loadPlugin(const string plugin);
    /*!
     * Returns whether the library was succesfully loaded or not
     */
    bool isLoaded() { return library_loaded; };
protected:
    bool library_loaded;
    void* handle;
};

} // namespace

#endif
