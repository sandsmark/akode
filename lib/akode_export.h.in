/* This file is part of the KDE libraries
    Copyright (c) 2002-2003 KDE Team

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

#ifndef AKODE_EXPORT_H
#define AKODE_EXPORT_H

#undef __KDE_HAVE_GCC_VISIBILITY

/**
 * The ARTS_NO_EXPORT macro marks the symbol of the given variable
 * to be hidden. A hidden symbol is stripped during the linking step,
 * so it can't be used from outside the resulting library, which is similar
 * to static. However, static limits the visibility to the current
 * compilation unit. hidden symbols can still be used in multiple compilation
 * units.
 *
 * \code
 * int AKODE_NO_EXPORT foo;
 * int AKODE_EXPORT bar;
 * \end
 */

#if defined(__KDE_HAVE_GCC_VISIBILITY)
/* Visibility is available for GCC newer than 3.4.
 * See: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=9283
 */
#define AKODE_NO_EXPORT __attribute__ ((visibility("hidden")))
#define AKODE_EXPORT __attribute__ ((visibility("default")))
#elif defined(_WIN32)
#define AKODE_NO_EXPORT
#define AKODE_EXPORT __declspec(dllexport)
#else
#define AKODE_NO_EXPORT
#define AKODE_EXPORT
#endif

#endif /* AKODE_EXPORTS */
