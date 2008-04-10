/*  aKode Arithmetics

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef _AKODE_ARITHMETIC_H
#define _AKODE_ARITHMETIC_H

// Parametirize the difference in aritmetics between floating point and integer.
template<typename S>
struct Arithm_FP {
    static inline S div(S value, S divisor) {
        return value/divisor;
    }
    static inline S rem(S, S) {
        return 0.0;
    }
    static inline S muldiv(S value, S multiplier, S divisor) {
        return (value*multiplier)/divisor;
    }
    static inline S max(int) {
        return 1.0;
    }
};

template<typename S>
struct Arithm_Int {
    static inline S div(S value, S divisor) {
        return value / divisor;
    }
    static inline S rem(S value, S divisor) {
        return value % divisor;
    }
    // A multiplication and division that avoids overflow
    static inline S muldiv(S value, S multiplier, S divisor) {
        S out = Arithm_Int<S>::div(value, divisor)*multiplier;
        S rem = Arithm_Int<S>::rem(value, divisor)*multiplier;
        out += div(rem, divisor);
        return out;
    }
    static inline S max(int bits) {
        return (((S)1)<<(bits-1))-1;
    }
};

#endif
