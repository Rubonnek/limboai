/* bt_repeat.cpp */

#include "bt_repeat.h"
#include "core/object.h"
#include "core/variant.h"

String BTRepeat::_generate_name() const {
	return vformat("Repeat x%s", times);
}

void BTRepeat::_enter() {
	_cur_iteration = 1;
}

int BTRepeat::_tick(float p_delta) {
	int status = get_child(0)->execute(p_delta);
	if (status == RUNNING) {
		return RUNNING;
	} else if (status == FAILURE && abort_on_failure) {
		return FAILURE;
	} else if (_cur_iteration >= times) {
		return SUCCESS;
	} else {
		_cur_iteration += 1;
		return RUNNING;
	}
}

void BTRepeat::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_times", "p_value"), &BTRepeat::set_times);
	ClassDB::bind_method(D_METHOD("get_times"), &BTRepeat::get_times);
	ClassDB::bind_method(D_METHOD("set_abort_on_failure", "p_value"), &BTRepeat::set_abort_on_failure);
	ClassDB::bind_method(D_METHOD("get_abort_on_failure"), &BTRepeat::get_abort_on_failure);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "times", PROPERTY_HINT_RANGE, "1,65535"), "set_times", "get_times");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "abort_on_failure"), "set_abort_on_failure", "get_abort_on_failure");
}