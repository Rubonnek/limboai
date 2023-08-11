/**
 * bt_check_agent_property.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_CHECK_AGENT_PROPERTY
#define BT_CHECK_AGENT_PROPERTY

#include "bt_condition.h"

#include "modules/limboai/blackboard/bb_param/bb_variant.h"
#include "modules/limboai/util/limbo_utility.h"

#include "core/object/object.h"
#include "core/string/string_name.h"

class BTCheckAgentProperty : public BTCondition {
	GDCLASS(BTCheckAgentProperty, BTCondition);

private:
	StringName property_name;
	LimboUtility::CheckType check_type = LimboUtility::CheckType::CHECK_EQUAL;
	Ref<BBVariant> value;

protected:
	static void _bind_methods();

	virtual String _generate_name() const override;
	virtual int _tick(double p_delta) override;

public:
	virtual String get_configuration_warning() const override;

	void set_property_name(StringName p_prop);
	StringName get_property_name() const { return property_name; }

	void set_check_type(LimboUtility::CheckType p_check_type);
	LimboUtility::CheckType get_check_type() const { return check_type; }

	void set_value(Ref<BBVariant> p_value);
	Ref<BBVariant> get_value() const { return value; }
};

#endif // BT_CHECK_AGENT_PROPERTY