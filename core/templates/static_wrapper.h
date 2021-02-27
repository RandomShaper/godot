/*************************************************************************/
/*  static_wrapper.h                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef STATIC_WRAPPER_H
#define STATIC_WRAPPER_H

#include <core/error/error_macros.h>
#include <core/os/memory.h>
#include <inttypes.h>

// This class holds an object of type T in its object-local memory and
// has pointer-like operators ->, * to access the contained object.
// The most important aspect of it is that it won't destroy its inner
// object when it is destroyed. To destroy the inner object, destroy()
// must be called explicitly where it makes sesnse to do so.
// The aim of this class is to help in scenarios where the lifetime of
// an object with static storage is hard to control, while at the same
// time avoiding the need to use such object via a pointer to heap memory,
// to avoid the potential performance cost of such indirection.

template <class T>
class StaticWrapper {
	uint8_t mem[sizeof(T)];
#ifdef DEBUG_ENABLED
	bool destroyed = false;
#endif

public:
	_FORCE_INLINE_ T *operator->() {
#ifdef DEBUG_ENABLED
		CRASH_COND(destroyed);
#endif
		return reinterpret_cast<T *>(mem);
	}

	_FORCE_INLINE_ T &operator*() {
#ifdef DEBUG_ENABLED
		CRASH_COND(destroyed);
#endif
		return *reinterpret_cast<T *>(mem);
	}

	void destroy() {
#ifdef DEBUG_ENABLED
		CRASH_COND(destroyed);
#endif
		(reinterpret_cast<T *>(mem))->~T();
	}

	StaticWrapper() {
		new (mem, sizeof(T), "") T();
	}
};

#endif
