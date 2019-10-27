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

DirAccessCombined::DirAccessCombined(): active_dir_index(npos) {

}

DirAccessCombined::~DirAccessCombined() {

	for (int i = 0; i < all_dirs.size(); i++) {
		memdelete(all_dirs[i]);
	}
}

Error DirAccessCombined::configure(const Vector<DirAccess *> &dirs) {

	// This should not be configured more than once
	if (all_dirs.size() > 0) {
		return ERR_BUG;
	}

	all_dirs = dirs;
	enabled_dirs = dirs;

	return OK;
}

void DirAccessCombined::_reset_enumeration() {

	active_dir_index = npos;
	directory_items.clear();
}

Error DirAccessCombined::change_dir(String p_dir) {

	Vector<DirAccess *> succeeded_dirs;
	for (int i = 0; i < all_dirs.size(); i++) {
		if (all_dirs[i]->change_dir(p_dir) == OK) {
			succeeded_dirs.push_back(all_dirs[i]);
		}
	}

	// If any succeeded, latch in the success and track the dirs that
	// actually contain that folder for enumeration
	if (succeeded_dirs.size() > 0) {
		enabled_dirs = succeeded_dirs;
		_reset_enumeration();
		return OK;
	} else {
		// If none succeeded, return an error - the change_dir didn't happen.
		return ERR_INVALID_PARAMETER;
	}
}

Error DirAccessCombined::list_dir_begin() {

	if (enabled_dirs.size() == 0) {
		return ERR_DOES_NOT_EXIST;
	}

	_reset_enumeration();

	active_dir_index = 0;
	return enabled_dirs[active_dir_index]->list_dir_begin();
}

String DirAccessCombined::get_next() {

	String result;
	while (result == "" && active_dir_index != npos) {
		result = enabled_dirs[active_dir_index]->get_next();
		if (result != "") {
			// On finding a duplicate, skip this entry.
			//
			// This only checks for duplicates after the first dir as an optimization since a single
			// file system will never have duplicate entries.
			if (active_dir_index > 0 && directory_items.has(result)) {
				result = "";
			} else {
				directory_items.insert(result);
			}
		} else {
			// The previously iterated dir is complete.
			// End iteration for that dir, and if there's a subsequent one, start iterating on it.
			enabled_dirs[active_dir_index]->list_dir_end();

			active_dir_index++;
			if (active_dir_index < enabled_dirs.size()) {
				enabled_dirs[active_dir_index]->list_dir_begin();
			} else {
				active_dir_index = npos;
			}
		}
	}
	return result;
}

bool DirAccessCombined::current_is_dir() const {

	if (active_dir_index == npos) {
		return false;
	}
	return enabled_dirs[active_dir_index]->current_is_dir();
}

bool DirAccessCombined::current_is_hidden() const {

	if (active_dir_index == npos) {
		return false;
	}
	return enabled_dirs[active_dir_index]->current_is_hidden();
}

void DirAccessCombined::list_dir_end() {

	if (active_dir_index != npos) {
		enabled_dirs[active_dir_index]->list_dir_end();
	}
	_reset_enumeration();
}

String DirAccessCombined::get_current_dir() {

	if (enabled_dirs.size() > 0) {
		return enabled_dirs[0]->get_current_dir();
	}
	return "";
}

bool DirAccessCombined::file_exists(String p_file) {

	const Vector<DirAccess *> &dirs_to_check = p_file.is_rel_path() ? enabled_dirs : all_dirs;

	for (int i = 0; i < dirs_to_check.size(); i++) {
		if (dirs_to_check[i]->file_exists(p_file)) {
			return true;
		}
	}
	return false;
}

bool DirAccessCombined::dir_exists(String p_dir) {

	const Vector<DirAccess *> &dirs_to_check = p_dir.is_rel_path() ? enabled_dirs : all_dirs;

	for (int i = 0; i < dirs_to_check.size(); i++) {
		if (dirs_to_check[i]->dir_exists(p_dir)) {
			return true;
		}
	}
	return false;
}

String DirAccessCombined::get_filesystem_type() const {

	return "COMBINED";
}

// The remaining methods are stubbed in since they don't make sense for
// a read-only virtual file system

Error DirAccessCombined::make_dir(String p_dir) {

	return ERR_UNAVAILABLE;
}

Error DirAccessCombined::rename(String p_from, String p_to) {

	return ERR_UNAVAILABLE;
}

Error DirAccessCombined::remove(String p_name) {

	return ERR_UNAVAILABLE;
}

size_t DirAccessCombined::get_space_left() {

	return 0;
}

int DirAccessCombined::get_drive_count() {

	return 0;
}

String DirAccessCombined::get_drive(int p_drive) {

	return "";
}
