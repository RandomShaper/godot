/*************************************************************************/
/*  dir_access_combined.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#ifndef DIR_ACCESS_COMBINED_H
#define DIR_ACCESS_COMBINED_H

#include "core/os/dir_access.h"
#include "core/set.h"
#include "core/vector.h"

/**
 * DirAccessCombined implements DirAccess against two or more underlying DirAccess implementations (viirtual file systems), combining
 * the directory listings with the following rules:
 *
 * - When changing dirs, DirAccessCombines provides access to the superset of all items across all file systems that have that (virtual) folder.
 * - When enumerating items in a dir, a given name is returned at most once.
 * - If there is a conflict (same path exists to file vs. folder or multiple files), it is loaded from the first file system that has that item.
 * - When checking if an item exists, relative paths only check the file systems that have the current directory.  Absolute paths check all file systems.
 * - This is a simplified read-only file system intended for ACCESS_RESOURCES, so it doesn't support drive letters.
 * - Regarding write/change operations, those are only supported in tools-enabled builds, where they will be routed to the real file system implementation
 *   found among the provided ones.
 *   In addition, such operations will be rejected if they are found to be targeting a branch of the file system tree "owned" by the packed file system.
 *   However, at runtime everything is simply read-only.
 */
class DirAccessCombined : public DirAccess {
	// dirs contains all dirs to combine into a virtual directory heirarchy
	Vector<DirAccess *> dirs;
#ifdef TOOLS_ENABLED
	// the real file system implementation
	DirAccess *non_packed_da;
#endif

	// the virtual notion of current directory, regardless which implementations could switch to it (as long as any did)
	String current_dir;

	struct {
		// dirs is the subset of dirs that include the current directory.
		// These directories and are used for enumeration within the current directory.
		Vector<DirAccess *> dirs;

		// dir_index is the active dir index for listing.  This indexes into dirs.
		// dir_index is -1 when there is no active dir index (not enumerating or enumeration complete)
		int dir_index;

		// items is the set of items already enumerated to avoid returning the same name more than once
		Set<String> items;
	} listing;

	void _reset_enumeration();

#ifdef TOOLS_ENABLED
	bool _path_parent_is_owned_by_pck(const String &p_path);
#endif

public:
	virtual Error list_dir_begin();
	virtual String get_next();
	virtual bool current_is_dir() const;
	virtual bool current_is_hidden() const;
	virtual void list_dir_end();

	virtual int get_drive_count();
	virtual String get_drive(int p_drive);

	virtual Error change_dir(String p_dir);
	virtual String get_current_dir();

	virtual bool file_exists(String p_file);
	virtual bool dir_exists(String p_dir);

	virtual Error make_dir(String p_dir);

	virtual Error rename(String p_from, String p_to);
	virtual Error remove(String p_path);

	size_t get_space_left();

	virtual String get_filesystem_type() const;

	/**
	 * configure() configures this object with the set of undelrying dirs to aggregate and combine.  It should be called once.
	 * The set of dirs are thereafter owned by this object and will be deleted when they're no longer needed.
	 */
	Error configure(const Vector<DirAccess *> &dirs);

	DirAccessCombined();
	~DirAccessCombined();
};

#endif // DIR_ACCESS_COMBINED_H
