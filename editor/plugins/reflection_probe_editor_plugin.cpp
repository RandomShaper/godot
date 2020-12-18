/*************************************************************************/
/*  reflection_probe_editor_plugin.cpp                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "reflection_probe_editor_plugin.h"

void ReflectionProbeEditorPlugin::_bake() {

	if (!probe) {
		return;
	}

	switch (probe->bake_reflections()) {
		case ReflectionProbe::BAKE_ERROR_NO_SAVE_PATH: {
			EditorNode::get_singleton()->show_warning(TTR("Can't determine a save path for the reflection texture. Please save your scene first."));
		} break;
		case ReflectionProbe::BAKE_ERROR_CANT_WRITE_FiLES: {
			EditorNode::get_singleton()->show_warning(TTR("Failed creating reflection files. Please make sure path is writable."));
		} break;
	}
}

void ReflectionProbeEditorPlugin::edit(Object *p_object) {

	ReflectionProbe *rp = Object::cast_to<ReflectionProbe>(p_object);
	if (rp) {
		probe = rp;
	}
}

bool ReflectionProbeEditorPlugin::handles(Object *p_object) const {

	return p_object->is_class("ReflectionProbe");
}

void ReflectionProbeEditorPlugin::make_visible(bool p_visible) {

	if (p_visible) {
		bake->show();
	} else {
		bake->hide();
	}
}

void ReflectionProbeEditorPlugin::_bind_methods() {

	ClassDB::bind_method("_bake", &ReflectionProbeEditorPlugin::_bake);
}

ReflectionProbeEditorPlugin::ReflectionProbeEditorPlugin(EditorNode *p_node) {

	editor = p_node;
	bake = memnew(ToolButton);
	bake->set_icon(editor->get_gui_base()->get_icon("Bake", "EditorIcons"));
	bake->set_text(TTR("Bake Reflections"));
	bake->hide();
	bake->connect("pressed", this, "_bake");
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, bake);
	probe = nullptr;
}

ReflectionProbeEditorPlugin::~ReflectionProbeEditorPlugin() {
}
