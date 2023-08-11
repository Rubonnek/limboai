/**
 * bt_check_agent_property.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_check_agent_property.h"

#include "modules/limboai/util/limbo_utility.h"

#include "core/variant/callable.h"

void BTCheckAgentProperty::set_property_name(StringName p_prop) {
	property_name = p_prop;
	emit_changed();
}

void BTCheckAgentProperty::set_check_type(LimboUtility::CheckType p_check_type) {
	check_type = p_check_type;
	emit_changed();
}

void BTCheckAgentProperty::set_value(Ref<BBVariant> p_value) {
	value = p_value;
	emit_changed();
	if (Engine::get_singleton()->is_editor_hint() && value.is_valid()) {
		value->connect(SNAME("changed"), Callable(this, SNAME("emit_changed")));
	}
}

String BTCheckAgentProperty::get_configuration_warning() const {
	String warning = BTCondition::get_configuration_warning();
	if (!warning.is_empty()) {
		warning += "\n";
	}
	if (property_name == StringName()) {
		warning += "`property_name` should be assigned.\n";
	}
	if (!value.is_valid()) {
		warning += "`value` should be assigned.\n";
	}
	return warning;
}

String BTCheckAgentProperty::_generate_name() const {
	if (property_name == StringName()) {
		return "CheckAgentProperty ???";
	}

	return vformat("Check if: agent.%s %s %s", property_name,
			LimboUtility::get_singleton()->get_check_operator_string(check_type),
			value.is_valid() ? Variant(value) : Variant("???"));
}

int BTCheckAgentProperty::_tick(double p_delta) {
	ERR_FAIL_COND_V_MSG(property_name == StringName(), FAILURE, "BTCheckAgentProperty: `property_name` is not set.");
	ERR_FAIL_COND_V_MSG(!value.is_valid(), FAILURE, "BTCheckAgentProperty: `value` is not set.");

	bool r_valid;
	Variant left_value = get_agent()->get(property_name, &r_valid);
	ERR_FAIL_COND_V_MSG(r_valid == false, FAILURE, vformat("BTCheckAgentProperty: Agent has no property named \"%s\"", property_name));

	Variant right_value = value->get_value(get_agent(), get_blackboard());

	return LimboUtility::get_singleton()->perform_check(check_type, left_value, right_value) ? SUCCESS : FAILURE;
}

void BTCheckAgentProperty::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_property_name", "p_property_name"), &BTCheckAgentProperty::set_property_name);
	ClassDB::bind_method(D_METHOD("get_property_name"), &BTCheckAgentProperty::get_property_name);
	ClassDB::bind_method(D_METHOD("set_check_type", "p_check_type"), &BTCheckAgentProperty::set_check_type);
	ClassDB::bind_method(D_METHOD("get_check_type"), &BTCheckAgentProperty::get_check_type);
	ClassDB::bind_method(D_METHOD("set_value", "p_value"), &BTCheckAgentProperty::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &BTCheckAgentProperty::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "property_name"), "set_property_name", "get_property_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "check_type", PROPERTY_HINT_ENUM, "Equal,Less Than,Less Than Or Equal,Greater Than,Greater Than Or Equal,Not Equal"), "set_check_type", "get_check_type");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "value", PROPERTY_HINT_RESOURCE_TYPE, "BBVariant"), "set_value", "get_value");
}