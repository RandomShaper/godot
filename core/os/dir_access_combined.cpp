/*************************************************************************/
/*  dir_access_combined.cpp                                              */
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

#include "dir_access_combined.h"

#ifdef TOOLS_ENABLED
#include "core/io/file_access_pack.h"
#endif

DirAccessCombined::DirAccessCombined() :
#ifdef TOOLS_ENABLED
		non_packed_da(NULL),
#endif
		current_dir("res://") {

	listing.dir_index = -1;
}

DirAccessCombined::~DirAccessCombined() {

	for (int i = 0; i < dirs.size(); i++) {
		memdelete(dirs[i]);
	}
}

Error DirAccessCombined::configure(const Vector<DirAccess *> &p_dirs) {

	// This should not be configured more than once
	CRASH_COND(dirs.size() > 0);

#ifdef TOOLS_ENABLED
	for (int i = 0; i < p_dirs.size(); ++i) {
		if (p_dirs[i]->get_filesystem_type() != "PCK") {
			// Only one real, non-PCK implementation allowed
			CRASH_COND(non_packed_da);
			non_packed_da = p_dirs[i];
		}
	}
	CRASH_COND(!non_packed_da);
#endif

	dirs = p_dirs;
	listing.dirs = p_dirs;

	return OK;
}

void DirAccessCombined::_reset_enumeration() {

	listing.dir_index = -1;
	listing.items.clear();
}

Error DirAccessCombined::change_dir(String p_dir) {

	const String &new_abs_dir = p_dir.is_abs_path() ? p_dir : current_dir.plus_file(p_dir).simplify_path();

	Vector<DirAccess *> succeeded_dirs;
	for (int i = 0; i < dirs.size(); i++) {
		if (dirs[i]->change_dir(new_abs_dir) == OK) {
			succeeded_dirs.push_back(dirs[i]);
		}
	}

	// If any succeeded, latch in the success and track the dirs that
	// actually contain that folder for enumeration
	if (succeeded_dirs.size() > 0) {
		current_dir = new_abs_dir;
		listing.dirs = succeeded_dirs;
		_reset_enumeration();
		return OK;
	} else {
		// If none succeeded, return an error - the change_dir didn't happen.
		return ERR_INVALID_PARAMETER;
	}
}

Error DirAccessCombined::list_dir_begin() {

	if (listing.dirs.size() == 0) {
		return ERR_DOES_NOT_EXIST;
	}

	_reset_enumeration();

	listing.dir_index = 0;
	return listing.dirs[listing.dir_index]->list_dir_begin();
}

String DirAccessCombined::get_next() {

	String result;
	while (listing.dir_index != -1 && result.empty()) {
		result = listing.dirs[listing.dir_index]->get_next();
		if (!result.empty()) {
			// On finding a duplicate, skip this entry.
			//
			// This only checks for duplicates after the first dir as an optimization since a single
			// file system will never have duplicate entries.
			if (listing.dir_index > 0 && listing.items.has(result)) {
				result.clear();
			} else {
				listing.items.insert(result);
			}
		} else {
			// The previously iterated dir is complete.
			// End iteration for that dir, and if there's a subsequent one, start iterating on it.
			listing.dirs[listing.dir_index]->list_dir_end();

			listing.dir_index++;
			if (listing.dir_index < listing.dirs.size()) {
				listing.dirs[listing.dir_index]->list_dir_begin();
			} else {
				listing.dir_index = -1;
			}
		}
	}
	return result;
}

bool DirAccessCombined::current_is_dir() const {

	if (listing.dir_index == -1) {
		return false;
	}
	return listing.dirs[listing.dir_index]->current_is_dir();
}

bool DirAccessCombined::current_is_hidden() const {

	if (listing.dir_index == -1) {
		return false;
	}
	return listing.dirs[listing.dir_index]->current_is_hidden();
}

void DirAccessCombined::list_dir_end() {

	if (listing.dir_index != -1) {
		listing.dirs[listing.dir_index]->list_dir_end();
	}
	_reset_enumeration();
}

String DirAccessCombined::get_current_dir() {

	return current_dir;
}

bool DirAccessCombined::file_exists(String p_file) {

	const String &abs_dir = p_file.is_abs_path() ? p_file : current_dir.plus_file(p_file);

	for (int i = 0; i < dirs.size(); i++) {
		if (dirs[i]->file_exists(p_file)) {
			return true;
		}
	}
	return false;
}

bool DirAccessCombined::dir_exists(String p_dir) {

	const String &abs_dir = p_dir.is_abs_path() ? p_dir : current_dir.plus_file(p_dir);

	for (int i = 0; i < dirs.size(); i++) {
		if (dirs[i]->dir_exists(p_dir)) {
			return true;
		}
	}
	return false;
}

String DirAccessCombined::get_filesystem_type() const {

	return "COMBINED";
}

Error DirAccessCombined::make_dir(String p_dir) {

#ifdef TOOLS_ENABLED
	const String &abs_dir = p_dir.is_abs_path() ? p_dir : current_dir.plus_file(p_dir);
	if (_path_parent_is_owned_by_pck(abs_dir)) {
		return ERR_UNAVAILABLE;
	} else {
		return non_packed_da->make_dir(p_dir);
	}
#else
	return ERR_UNAVAILABLE;
#endif
}

Error DirAccessCombined::rename(String p_from, String p_to) {

#ifdef TOOLS_ENABLED
	const String &abs_from = p_from.is_abs_path() ? p_from : current_dir.plus_file(p_from);
	const String &abs_to = p_to.is_abs_path() ? p_to : current_dir.plus_file(p_to);
	if (_path_parent_is_owned_by_pck(abs_from) || _path_parent_is_owned_by_pck(abs_to)) {
		return ERR_UNAVAILABLE;
	} else {
		return non_packed_da->rename(p_from, p_to);
	}
#else
	return ERR_UNAVAILABLE;
#endif
}

Error DirAccessCombined::remove(String p_path) {

#ifdef TOOLS_ENABLED
	const String &abs_path = p_path.is_abs_path() ? p_path : current_dir.plus_file(p_path);
	if (_path_parent_is_owned_by_pck(abs_path)) {
		return ERR_UNAVAILABLE;
	} else {
		return non_packed_da->remove(p_path);
	}
#else
	return ERR_UNAVAILABLE;
#endif
}

size_t DirAccessCombined::get_space_left() {

#ifdef TOOLS_ENABLED
	return non_packed_da->get_space_left();
#else
	return 0;
#endif
}

int DirAccessCombined::get_drive_count() {

	return 0;
}

String DirAccessCombined::get_drive(int p_drive) {

	return String();
}

#ifdef TOOLS_ENABLED
bool DirAccessCombined::_path_parent_is_owned_by_pck(const String &p_path) {

	ERR_FAIL_COND_V(!p_path.begins_with("res://"), false);

	String parent_dir = p_path.trim_suffix("/").get_base_dir();
	return PackedData::get_singleton()->owns_path(parent_dir);
}
#endif
