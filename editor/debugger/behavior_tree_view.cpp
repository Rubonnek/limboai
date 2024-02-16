/**
 * behavior_tree_view.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#include "behavior_tree_view.h"

#include "../../bt/tasks/bt_task.h"
#include "../../util/limbo_compat.h"
#include "../../util/limbo_utility.h"
#include "behavior_tree_data.h"

#ifdef LIMBOAI_MODULE
#include "core/math/color.h"
#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/typedefs.h"
#include "editor/themes/editor_scale.h"
#include "editor/editor_settings.h"
#include "scene/resources/style_box.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#endif // LIMBOAI_GDEXTENSION

void BehaviorTreeView::_draw_running_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	theme_cache.sbf_running->draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_draw_success_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	theme_cache.sbf_success->draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_draw_failure_status(Object *p_obj, Rect2 p_rect) {
	p_rect = p_rect.grow_side(SIDE_LEFT, p_rect.get_position().x);
	theme_cache.sbf_failure->draw(tree->get_canvas_item(), p_rect);
}

void BehaviorTreeView::_item_collapsed(Object *p_obj) {
	TreeItem *item = Object::cast_to<TreeItem>(p_obj);
	if (!item) {
		return;
	}
	int id = item->get_metadata(0);
	bool collapsed = item->is_collapsed();
	if (!collapsed_ids.has(id) && collapsed) {
		collapsed_ids.push_back(item->get_metadata(0));
	} else if (collapsed_ids.has(id) && !collapsed) {
		collapsed_ids.erase(id);
	}
}

double BehaviorTreeView::_get_editor_scale() const {
	if (Engine::get_singleton()->is_editor_hint()) {
		return EDSCALE;
	} else {
		return 1.0;
	}
}

void BehaviorTreeView::update_tree(const Ref<BehaviorTreeData> &p_data) {
	// Remember selected.
	int selected_id = -1;
	if (tree->get_selected()) {
		selected_id = tree->get_selected()->get_metadata(0);
	}

	tree->clear();
	TreeItem *parent = nullptr;
	List<Pair<TreeItem *, int>> parents;
	for (const BehaviorTreeData::TaskData &task_data : p_data->tasks) {
		// Figure out parent.
		parent = nullptr;
		if (parents.size()) {
			Pair<TreeItem *, int> &p = parents[0];
			parent = p.first;
			if (!(--p.second)) {
				// No children left, remove it.
				parents.pop_front();
			}
		}

		TreeItem *item = tree->create_item(parent);
		// Do this first because it resets properties of the cell...
		item->set_cell_mode(0, TreeItem::CELL_MODE_CUSTOM);
		item->set_cell_mode(1, TreeItem::CELL_MODE_ICON);

		item->set_metadata(0, task_data.id);

		item->set_text(0, task_data.name);
		if (task_data.is_custom_name) {
			item->set_custom_font(0, theme_cache.font_custom_name);
		}

		item->set_text_alignment(2, HORIZONTAL_ALIGNMENT_RIGHT);
		item->set_text(2, rtos(Math::snapped(task_data.elapsed_time, 0.01)).pad_decimals(2));

		String cors = (task_data.script_path.is_empty()) ? task_data.type_name : task_data.script_path;
		item->set_icon(0, LimboUtility::get_singleton()->get_task_icon(cors));
		item->set_icon_max_width(0, 16 * _get_editor_scale()); // Force user icon size.

		if (task_data.status == BTTask::SUCCESS) {
			item->set_custom_draw(0, this, LW_NAME(_draw_success_status));
			item->set_icon(1, theme_cache.icon_success);
		} else if (task_data.status == BTTask::FAILURE) {
			item->set_custom_draw(0, this, LW_NAME(_draw_failure_status));
			item->set_icon(1, theme_cache.icon_failure);
		} else if (task_data.status == BTTask::RUNNING) {
			item->set_custom_draw(0, this, LW_NAME(_draw_running_status));
			item->set_icon(1, theme_cache.icon_running);
		}

		if (task_data.id == selected_id) {
			tree->set_selected(item, 0);
		}

		if (collapsed_ids.has(task_data.id)) {
			item->set_collapsed(true);
		}

		// Add in front of parents stack if it expects children.
		if (task_data.num_children) {
			parents.push_front(Pair<TreeItem *, int>(item, task_data.num_children));
		}
	}
}

void BehaviorTreeView::clear() {
	tree->clear();
	collapsed_ids.clear();
}

void BehaviorTreeView::_do_update_theme_item_cache() {
	theme_cache.icon_running = LimboUtility::get_singleton()->get_task_icon("LimboExtraClock");
	theme_cache.icon_success = LimboUtility::get_singleton()->get_task_icon("BTAlwaysSucceed");
	theme_cache.icon_failure = LimboUtility::get_singleton()->get_task_icon("BTAlwaysFail");

	theme_cache.font_custom_name = get_theme_font(LW_NAME(bold), LW_NAME(EditorFonts));

	Color running_border = Color::html("#fea900");
	Color running_fill = Color(running_border, 0.1);
	Color success_border = Color::html("#2fa139");
	Color success_fill = Color(success_border, 0.1);
	Color failure_border = Color::html("#cd3838");
	Color failure_fill = Color(failure_border, 0.1);

	theme_cache.sbf_running.instantiate();
	theme_cache.sbf_running->set_border_color(running_border);
	theme_cache.sbf_running->set_bg_color(running_fill);
	theme_cache.sbf_running->set_border_width(SIDE_LEFT, 4.0);
	theme_cache.sbf_running->set_border_width(SIDE_RIGHT, 4.0);

	theme_cache.sbf_success.instantiate();
	theme_cache.sbf_success->set_border_color(success_border);
	theme_cache.sbf_success->set_bg_color(success_fill);
	theme_cache.sbf_success->set_border_width(SIDE_LEFT, 4.0);
	theme_cache.sbf_success->set_border_width(SIDE_RIGHT, 4.0);

	theme_cache.sbf_failure.instantiate();
	theme_cache.sbf_failure->set_border_color(failure_border);
	theme_cache.sbf_failure->set_bg_color(failure_fill);
	theme_cache.sbf_failure->set_border_width(SIDE_LEFT, 4.0);
	theme_cache.sbf_failure->set_border_width(SIDE_RIGHT, 4.0);

	double extra_spacing = 0.0;
	if (Engine::get_singleton()->is_editor_hint()) {
		extra_spacing = EDITOR_GET("interface/theme/additional_spacing");
		extra_spacing *= 2.0;
	}
	tree->set_column_clip_content(0, true);
	tree->set_column_custom_minimum_width(1, 18 * _get_editor_scale());

	Ref<Font> font = tree->get_theme_font(LW_NAME(font));
	int font_size = tree->get_theme_font_size(LW_NAME(font_size));
	int timings_size = font->get_string_size("00.00", HORIZONTAL_ALIGNMENT_RIGHT, -1, font_size).x + 16 + extra_spacing;
	tree->set_column_custom_minimum_width(2, timings_size * _get_editor_scale());
}

void BehaviorTreeView::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			tree->connect(LW_NAME(item_collapsed), callable_mp(this, &BehaviorTreeView::_item_collapsed));
		} break;
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED:
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			_do_update_theme_item_cache();
		} break;
	}
}

void BehaviorTreeView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_draw_running_status"), &BehaviorTreeView::_draw_running_status);
	ClassDB::bind_method(D_METHOD("_draw_success_status"), &BehaviorTreeView::_draw_success_status);
	ClassDB::bind_method(D_METHOD("_draw_failure_status"), &BehaviorTreeView::_draw_failure_status);
	ClassDB::bind_method(D_METHOD("_item_collapsed"), &BehaviorTreeView::_item_collapsed);
	ClassDB::bind_method(D_METHOD("update_tree", "p_behavior_tree_data"), &BehaviorTreeView::update_tree);
}

BehaviorTreeView::BehaviorTreeView() {
	tree = memnew(Tree);
	add_child(tree);
	tree->set_columns(3); // task | status icon | elapsed
	tree->set_column_expand(0, true);
	tree->set_column_expand(1, false);
	tree->set_column_expand(2, false);
	tree->set_anchor(SIDE_RIGHT, ANCHOR_END);
	tree->set_anchor(SIDE_BOTTOM, ANCHOR_END);
}

#endif // TOOLS_ENABLED
