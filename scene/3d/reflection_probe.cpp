/*************************************************************************/
/*  reflection_probe.cpp                                                 */
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

#include "reflection_probe.h"

#ifdef TOOLS_ENABLED
#include "core/io/config_file.h"
#include "core/os/dir_access.h"
#include "editor/editor_node.h"
#endif

static const int RESOLUTION_TO_PIXELS[] = { 16, 32, 64, 128, 256, 512 };

void ReflectionProbe::set_intensity(float p_intensity) {

	intensity = p_intensity;
	VS::get_singleton()->reflection_probe_set_intensity(probe, p_intensity);
}

float ReflectionProbe::get_intensity() const {

	return intensity;
}

void ReflectionProbe::set_interior_ambient(Color p_ambient) {

	interior_ambient = p_ambient;
	VS::get_singleton()->reflection_probe_set_interior_ambient(probe, p_ambient);
}

void ReflectionProbe::set_interior_ambient_energy(float p_energy) {
	interior_ambient_energy = p_energy;
	VS::get_singleton()->reflection_probe_set_interior_ambient_energy(probe, p_energy);
}

float ReflectionProbe::get_interior_ambient_energy() const {
	return interior_ambient_energy;
}

Color ReflectionProbe::get_interior_ambient() const {

	return interior_ambient;
}

void ReflectionProbe::set_interior_ambient_probe_contribution(float p_contribution) {

	interior_ambient_probe_contribution = p_contribution;
	VS::get_singleton()->reflection_probe_set_interior_ambient_probe_contribution(probe, p_contribution);
}

float ReflectionProbe::get_interior_ambient_probe_contribution() const {

	return interior_ambient_probe_contribution;
}

void ReflectionProbe::set_max_distance(float p_distance) {

	max_distance = p_distance;
	VS::get_singleton()->reflection_probe_set_max_distance(probe, p_distance);
}
float ReflectionProbe::get_max_distance() const {

	return max_distance;
}

void ReflectionProbe::set_extents(const Vector3 &p_extents) {

	extents = p_extents;

	for (int i = 0; i < 3; i++) {
		if (extents[i] < 0.01) {
			extents[i] = 0.01;
		}

		if (extents[i] - 0.01 < ABS(origin_offset[i])) {
			origin_offset[i] = SGN(origin_offset[i]) * (extents[i] - 0.01);
			_change_notify("origin_offset");
		}
	}

	VS::get_singleton()->reflection_probe_set_extents(probe, extents);
	VS::get_singleton()->reflection_probe_set_origin_offset(probe, origin_offset);
	_change_notify("extents");
	update_gizmo();
}
Vector3 ReflectionProbe::get_extents() const {

	return extents;
}

void ReflectionProbe::set_origin_offset(const Vector3 &p_extents) {

	origin_offset = p_extents;

	for (int i = 0; i < 3; i++) {

		if (extents[i] - 0.01 < ABS(origin_offset[i])) {
			origin_offset[i] = SGN(origin_offset[i]) * (extents[i] - 0.01);
		}
	}
	VS::get_singleton()->reflection_probe_set_extents(probe, extents);
	VS::get_singleton()->reflection_probe_set_origin_offset(probe, origin_offset);

	_change_notify("origin_offset");
	update_gizmo();
}
Vector3 ReflectionProbe::get_origin_offset() const {

	return origin_offset;
}

void ReflectionProbe::set_enable_box_projection(bool p_enable) {

	box_projection = p_enable;
	VS::get_singleton()->reflection_probe_set_enable_box_projection(probe, p_enable);
}
bool ReflectionProbe::is_box_projection_enabled() const {

	return box_projection;
}

void ReflectionProbe::set_as_interior(bool p_enable) {

	interior = p_enable;
	VS::get_singleton()->reflection_probe_set_as_interior(probe, interior);
	_change_notify();
}

bool ReflectionProbe::is_set_as_interior() const {

	return interior;
}

void ReflectionProbe::set_enable_shadows(bool p_enable) {

	enable_shadows = p_enable;
	VS::get_singleton()->reflection_probe_set_enable_shadows(probe, p_enable);
}
bool ReflectionProbe::are_shadows_enabled() const {

	return enable_shadows;
}

void ReflectionProbe::set_cull_mask(uint32_t p_layers) {

	cull_mask = p_layers;
	VS::get_singleton()->reflection_probe_set_cull_mask(probe, p_layers);
}
uint32_t ReflectionProbe::get_cull_mask() const {

	return cull_mask;
}

void ReflectionProbe::set_resolution(int p_resolution) {

	if (p_resolution == resolution) {
		return;
	}
	resolution = p_resolution;
}

int ReflectionProbe::get_resolution() const {

	return resolution;
}

void ReflectionProbe::set_bake_texture(const Ref<Texture> &p_texture) {

	if (p_texture == bake_texture)
		return;

	bake_texture = p_texture;
	VS::get_singleton()->reflection_probe_set_bake_texture(probe, bake_texture.is_valid() ? bake_texture->get_rid() : RID());
	_change_notify();
}

Ref<Texture> ReflectionProbe::get_bake_texture() const {

	return bake_texture;
}

#ifdef TOOLS_ENABLED
ReflectionProbe::BakeError ReflectionProbe::bake_reflections() {

	String save_path;
	if (get_owner() && get_owner()->get_filename() != "") {
		save_path = get_owner()->get_filename().get_base_dir();
	} else {
		return BAKE_ERROR_NO_SAVE_PATH;
	}

	DirAccessRef da = DirAccess::create_for_path(save_path);
	if (!da) {
		return BAKE_ERROR_CANT_WRITE_FiLES;
	}
	String save_subdir = GLOBAL_GET("rendering/reflection_probes/output_subfolder");
	if (save_subdir != "") {
		save_path = save_path.plus_file(save_subdir);
		if (!da->dir_exists(save_path)) {
			if (da->make_dir_recursive(save_path) != OK) {
				return BAKE_ERROR_CANT_WRITE_FiLES;
			}
		}
	}

	Ref<Image> image = VS::get_singleton()->reflection_probe_bake(probe, RESOLUTION_TO_PIXELS[resolution]);

	String image_path = save_path.plus_file(get_name()) + ".png";
	if (image->save_png(image_path) != OK) {
		return BAKE_ERROR_CANT_WRITE_FiLES;
	}

	Ref<ConfigFile> config;
	config.instance();
	if (FileAccess::exists(image_path + ".import")) {
		config->load(image_path + ".import");
	} else {
		// Set only if settings don't exist, to keep user choice
		config->set_value("params", "compress/mode", 1); // VRAM
	}
	config->set_value("remap", "importer", "texture");
	config->set_value("remap", "type", "StreamTexture");
	config->set_value("params", "detect_3d", false);
	config->set_value("params", "flags/repeat", false);
	config->set_value("params", "flags/filter", true);
	config->set_value("params", "flags/mipmaps", true);
	config->set_value("params", "flags/srgb", 0);
	if (config->save(image_path + ".import") != OK) {
		return BAKE_ERROR_CANT_WRITE_FiLES;
	}

	ResourceLoader::import(image_path);
	RES texture = ResourceLoader::load(image_path);
	set_bake_texture(texture);

	return BAKE_ERROR_OK;
}
#endif

AABB ReflectionProbe::get_aabb() const {

	AABB aabb;
	aabb.position = -origin_offset;
	aabb.size = origin_offset + extents;
	return aabb;
}
PoolVector<Face3> ReflectionProbe::get_faces(uint32_t p_usage_flags) const {

	return PoolVector<Face3>();
}

void ReflectionProbe::_validate_property(PropertyInfo &property) const {

	if (property.name == "interior/ambient_color" || property.name == "interior/ambient_energy" || property.name == "interior/ambient_contrib") {
		if (!interior) {
			property.usage = PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL;
		}
	}
}

void ReflectionProbe::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_intensity", "intensity"), &ReflectionProbe::set_intensity);
	ClassDB::bind_method(D_METHOD("get_intensity"), &ReflectionProbe::get_intensity);

	ClassDB::bind_method(D_METHOD("set_interior_ambient", "ambient"), &ReflectionProbe::set_interior_ambient);
	ClassDB::bind_method(D_METHOD("get_interior_ambient"), &ReflectionProbe::get_interior_ambient);

	ClassDB::bind_method(D_METHOD("set_interior_ambient_energy", "ambient_energy"), &ReflectionProbe::set_interior_ambient_energy);
	ClassDB::bind_method(D_METHOD("get_interior_ambient_energy"), &ReflectionProbe::get_interior_ambient_energy);

	ClassDB::bind_method(D_METHOD("set_interior_ambient_probe_contribution", "ambient_probe_contribution"), &ReflectionProbe::set_interior_ambient_probe_contribution);
	ClassDB::bind_method(D_METHOD("get_interior_ambient_probe_contribution"), &ReflectionProbe::get_interior_ambient_probe_contribution);

	ClassDB::bind_method(D_METHOD("set_max_distance", "max_distance"), &ReflectionProbe::set_max_distance);
	ClassDB::bind_method(D_METHOD("get_max_distance"), &ReflectionProbe::get_max_distance);

	ClassDB::bind_method(D_METHOD("set_extents", "extents"), &ReflectionProbe::set_extents);
	ClassDB::bind_method(D_METHOD("get_extents"), &ReflectionProbe::get_extents);

	ClassDB::bind_method(D_METHOD("set_origin_offset", "origin_offset"), &ReflectionProbe::set_origin_offset);
	ClassDB::bind_method(D_METHOD("get_origin_offset"), &ReflectionProbe::get_origin_offset);

	ClassDB::bind_method(D_METHOD("set_as_interior", "enable"), &ReflectionProbe::set_as_interior);
	ClassDB::bind_method(D_METHOD("is_set_as_interior"), &ReflectionProbe::is_set_as_interior);

	ClassDB::bind_method(D_METHOD("set_enable_box_projection", "enable"), &ReflectionProbe::set_enable_box_projection);
	ClassDB::bind_method(D_METHOD("is_box_projection_enabled"), &ReflectionProbe::is_box_projection_enabled);

	ClassDB::bind_method(D_METHOD("set_enable_shadows", "enable"), &ReflectionProbe::set_enable_shadows);
	ClassDB::bind_method(D_METHOD("are_shadows_enabled"), &ReflectionProbe::are_shadows_enabled);

	ClassDB::bind_method(D_METHOD("set_cull_mask", "layers"), &ReflectionProbe::set_cull_mask);
	ClassDB::bind_method(D_METHOD("get_cull_mask"), &ReflectionProbe::get_cull_mask);

	ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &ReflectionProbe::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &ReflectionProbe::get_resolution);

	ClassDB::bind_method(D_METHOD("set_bake_texture", "texture"), &ReflectionProbe::set_bake_texture);
	ClassDB::bind_method(D_METHOD("get_bake_texture"), &ReflectionProbe::get_bake_texture);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "intensity", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_intensity", "get_intensity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_distance", PROPERTY_HINT_EXP_RANGE, "0,16384,0.1,or_greater"), "set_max_distance", "get_max_distance");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "extents"), "set_extents", "get_extents");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "origin_offset"), "set_origin_offset", "get_origin_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "box_projection"), "set_enable_box_projection", "is_box_projection_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_shadows"), "set_enable_shadows", "are_shadows_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cull_mask", PROPERTY_HINT_LAYERS_3D_RENDER), "set_cull_mask", "get_cull_mask");

	ADD_GROUP("Interior", "interior_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "interior_enable"), "set_as_interior", "is_set_as_interior");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "interior_ambient_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_interior_ambient", "get_interior_ambient");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "interior_ambient_energy", PROPERTY_HINT_RANGE, "0,16,0.01"), "set_interior_ambient_energy", "get_interior_ambient_energy");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "interior_ambient_contrib", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_interior_ambient_probe_contribution", "get_interior_ambient_probe_contribution");

	ADD_GROUP("Bake", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_ENUM, "16,32,64,128,256,512"), "set_resolution", "get_resolution");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bake_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_bake_texture", "get_bake_texture");
}

ReflectionProbe::ReflectionProbe() {

	resolution = 4; // 256
	intensity = 1.0;
	interior_ambient = Color(0, 0, 0);
	interior_ambient_probe_contribution = 0;
	interior_ambient_energy = 1.0;
	max_distance = 0;
	extents = Vector3(1, 1, 1);
	origin_offset = Vector3(0, 0, 0);
	box_projection = false;
	interior = false;
	enable_shadows = false;
	cull_mask = (1 << 20) - 1;

	probe = VisualServer::get_singleton()->reflection_probe_create();
	VS::get_singleton()->instance_set_base(get_instance(), probe);
	set_disable_scale(true);
}

ReflectionProbe::~ReflectionProbe() {

	VS::get_singleton()->free(probe);
}
