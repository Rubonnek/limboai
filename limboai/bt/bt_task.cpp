/* bt_task.cpp */

#include "bt_task.h"

#include "core/class_db.h"
#include "core/object.h"
#include "core/script_language.h"
#include "core/variant.h"
#include "editor/editor_node.h"

#include "../limbo_string_names.h"
#include "../limbo_utility.h"

String BTTask::_generate_name() const {
	if (get_script_instance()) {
		if (get_script_instance()->has_method(LimboStringNames::get_singleton()->_generate_name)) {
			return get_script_instance()->call(LimboStringNames::get_singleton()->_generate_name);
		}
		String name = get_script_instance()->get_script()->get_path();
		if (!name.empty()) {
			// Generate name based on script file
			name = name.get_basename().get_file();
			return name;
		}
	}
	return get_class().trim_prefix("BT");
}

Array BTTask::_get_children() const {
	Array arr;
	int num_children = get_child_count();
	arr.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		arr[i] = get_child(i).ptr();
	}

	return arr;
}

void BTTask::_set_children(Array p_children) {
	_children.clear();
	const int num_children = p_children.size();
	_children.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		Variant task_var = p_children[i];
		const Ref<BTTask> task_ref = task_var;
		_children.set(i, task_var);
	}
}

String BTTask::get_task_name() const {
	if (_custom_name.empty()) {
		return _generate_name();
	}
	return _custom_name;
}

Ref<BTTask> BTTask::get_root() const {
	const BTTask *task = this;
	while (!task->is_root()) {
		task = task->get_parent().ptr();
	}
	return Ref<BTTask>(task);
}

void BTTask::set_custom_name(const String &p_name) {
	if (_custom_name != p_name) {
		_custom_name = p_name;
		emit_changed();
	}
};

void BTTask::initialize(Object *p_agent, Dictionary p_blackboard) {
	_agent = p_agent;
	_blackboard = p_blackboard;
	for (int i = 0; i < _children.size(); i++) {
		get_child(i)->initialize(p_agent, p_blackboard);
	}
	if (get_script_instance() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_setup)) {
		get_script_instance()->call(LimboStringNames::get_singleton()->_setup);
	} else {
		_setup();
	}
}

Ref<BTTask> BTTask::clone() const {
	Ref<BTTask> inst = duplicate(true);
	inst.ptr()->_parent.unref();
	CRASH_COND(inst.ptr()->get_parent().is_valid());
	for (int i = 0; i < _children.size(); i++) {
		Ref<BTTask> c = get_child(i)->clone();
		c->_parent = inst;
		inst->_children.set(i, c);
	}
	return inst;
}

int BTTask::execute(float p_delta) {
	if (_status == RUNNING) {
		if (get_script_instance() &&
				// get_script_instance()->get_script()->is_valid() &&
				get_script_instance()->has_method(LimboStringNames::get_singleton()->_enter)) {
			get_script_instance()->call(LimboStringNames::get_singleton()->_enter);
		} else {
			_enter();
		}
	}
	if (get_script_instance() &&
			// get_script_instance()->get_script()->is_valid() &&
			get_script_instance()->has_method(LimboStringNames::get_singleton()->_tick)) {
		_status = get_script_instance()->call(LimboStringNames::get_singleton()->_tick, Variant(p_delta));
	} else {
		_status = _tick(p_delta);
	}

	if (_status != RUNNING) {
		if (get_script_instance() &&
				// get_script_instance()->get_script()->is_valid() &&
				get_script_instance()->has_method(LimboStringNames::get_singleton()->_exit)) {
			get_script_instance()->call(LimboStringNames::get_singleton()->_exit);
		} else {
			_exit();
		}
	}
	return _status;
}

void BTTask::cancel() {
	for (int i = 0; i < _children.size(); i++) {
		get_child(i)->cancel();
	}
	if (_status == RUNNING) {
		if (get_script_instance() &&
				// get_script_instance()->get_script()->is_valid() &&
				get_script_instance()->has_method(LimboStringNames::get_singleton()->_exit)) {
			get_script_instance()->call(LimboStringNames::get_singleton()->_exit);
		} else {
			_exit();
		}
	}
	_status = FRESH;
}

Ref<BTTask> BTTask::get_child(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, _children.size(), nullptr);
	return _children.get(p_idx);
}

int BTTask::get_child_count() const {
	return _children.size();
}

void BTTask::add_child(Ref<BTTask> p_child) {
	ERR_FAIL_COND_MSG(p_child.ptr()->get_parent().is_valid(), "p_child already has a parent!");
	p_child->_parent = Ref<BTTask>(this);
	_children.push_back(p_child);
	emit_changed();
}

void BTTask::add_child_at_index(Ref<BTTask> p_child, int p_idx) {
	ERR_FAIL_COND_MSG(p_child.ptr()->get_parent().is_valid(), "p_child already has a parent!");
	if (p_idx < 0 || p_idx > _children.size()) {
		p_idx = _children.size();
	}
	_children.insert(p_idx, p_child);
	p_child->_parent = Ref<BTTask>(this);
	emit_changed();
}

void BTTask::remove_child(Ref<BTTask> p_child) {
	int idx = _children.find(p_child);
	if (idx == -1) {
		ERR_FAIL_MSG("p_child not found!");
	} else {
		_children.remove(idx);
		p_child->_parent.unref();
		emit_changed();
	}
}

bool BTTask::has_child(const Ref<BTTask> &p_child) const {
	return _children.find(p_child) != -1;
}

int BTTask::get_child_index(const Ref<BTTask> &p_child) const {
	return _children.find(p_child);
}

Ref<BTTask> BTTask::next_sibling() const {
	if (_parent.is_valid()) {
		int idx = _parent->get_child_index(Ref<BTTask>(this));
		if (idx != -1 && _parent->get_child_count() > (idx + 1)) {
			return _parent->get_child(idx + 1);
		}
	}
	return Ref<BTTask>();
}

String BTTask::get_configuration_warning() const {
	return String();
}

Ref<Texture> BTTask::get_icon() const {
	return EditorNode::get_singleton()->get_class_icon("BTAction", "Object");
}

void BTTask::print_tree(int p_initial_tabs) const {
	String tabs = "--";
	for (int i = 0; i < p_initial_tabs; i++) {
		tabs += "--";
	}
	print_line(vformat("%s Name: %s Instance: %s", tabs, get_task_name(), Ref<BTTask>(this)));
	for (int i = 0; i < get_child_count(); i++) {
		get_child(i)->print_tree(p_initial_tabs + 1);
	}
}

void BTTask::_bind_methods() {
	// Properties.
	ClassDB::bind_method(D_METHOD("get_custom_name"), &BTTask::get_custom_name);
	ClassDB::bind_method(D_METHOD("set_custom_name", "p_name"), &BTTask::set_custom_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_name"), "set_custom_name", "get_custom_name");
	ClassDB::bind_method(D_METHOD("get_agent"), &BTTask::get_agent);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "agent", PROPERTY_HINT_NONE, "", 0, "Object"), "", "get_agent");
	ClassDB::bind_method(D_METHOD("get_blackboard"), &BTTask::get_blackboard);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "blackboard", PROPERTY_HINT_NONE, "", 0, "Dictionary"), "", "get_blackboard");
	ClassDB::bind_method(D_METHOD("get_parent"), &BTTask::get_parent);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "parent", PROPERTY_HINT_NONE, "", 0, "BTTask"), "", "get_parent");
	ClassDB::bind_method(D_METHOD("get_children"), &BTTask::_get_children);
	ClassDB::bind_method(D_METHOD("set_children", "p_children"), &BTTask::_set_children);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_children", "get_children");
	ClassDB::bind_method(D_METHOD("get_status"), &BTTask::get_status);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status"), "", "get_status");

	// Virtual methods.
	ClassDB::bind_method(D_METHOD("_setup"), &BTTask::_setup);
	BIND_VMETHOD(MethodInfo("_setup"));
	ClassDB::bind_method(D_METHOD("_enter"), &BTTask::_enter);
	BIND_VMETHOD(MethodInfo("_enter"))
	ClassDB::bind_method(D_METHOD("_exit"), &BTTask::_exit);
	BIND_VMETHOD(MethodInfo("_exit"));
	ClassDB::bind_method(D_METHOD("_tick", "p_delta"), &BTTask::_tick);
	BIND_VMETHOD(MethodInfo(Variant::INT, "_tick", PropertyInfo(Variant::REAL, "p_delta")));
	ClassDB::bind_method(D_METHOD("_generate_name"), &BTTask::_generate_name);
	BIND_VMETHOD(MethodInfo(PropertyInfo(Variant::STRING, ""), "_generate_name"));
	ClassDB::bind_method(D_METHOD("_get_configuration_warning"), &BTTask::get_configuration_warning);
	BIND_VMETHOD(MethodInfo(PropertyInfo(Variant::STRING, ""), "_get_configuration_warning"));
	ClassDB::bind_method(D_METHOD("_get_icon"), &BTTask::get_icon);
	BIND_VMETHOD(MethodInfo(PropertyInfo(Variant::OBJECT, "", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "_get_icon"));

	// Public Methods.
	ClassDB::bind_method(D_METHOD("is_root"), &BTTask::is_root);
	ClassDB::bind_method(D_METHOD("get_root"), &BTTask::get_root);
	ClassDB::bind_method(D_METHOD("initialize", "p_agent", "p_blackboard"), &BTTask::initialize);
	ClassDB::bind_method(D_METHOD("clone"), &BTTask::clone);
	ClassDB::bind_method(D_METHOD("execute", "p_delta"), &BTTask::execute);
	ClassDB::bind_method(D_METHOD("get_child", "p_idx"), &BTTask::get_child);
	ClassDB::bind_method(D_METHOD("get_child_count"), &BTTask::get_child_count);
	ClassDB::bind_method(D_METHOD("add_child", "p_child"), &BTTask::add_child);
	ClassDB::bind_method(D_METHOD("add_child_at_index", "p_child", "p_idx"), &BTTask::add_child_at_index);
	ClassDB::bind_method(D_METHOD("remove_child", "p_child"), &BTTask::remove_child);
	ClassDB::bind_method(D_METHOD("has_child", "p_child"), &BTTask::has_child);
	ClassDB::bind_method(D_METHOD("get_child_index", "p_child"), &BTTask::get_child_index);
	ClassDB::bind_method(D_METHOD("next_sibling"), &BTTask::next_sibling);
	ClassDB::bind_method(D_METHOD("print_tree", "p_initial_tabs"), &BTTask::print_tree, Variant(0));
	ClassDB::bind_method(D_METHOD("get_task_name"), &BTTask::get_task_name);

	// Enums.
	ClassDB::bind_integer_constant(get_class_static(), "TaskStatus", "FRESH", FRESH);
	ClassDB::bind_integer_constant(get_class_static(), "TaskStatus", "RUNNING", RUNNING);
	ClassDB::bind_integer_constant(get_class_static(), "TaskStatus", "FAILURE", FAILURE);
	ClassDB::bind_integer_constant(get_class_static(), "TaskStatus", "SUCCESS", SUCCESS);
}

BTTask::BTTask() {
	_custom_name = String();
	_agent = nullptr;
	_blackboard = Dictionary();
	_children = Vector<Ref<BTTask>>();
	_status = FRESH;
}