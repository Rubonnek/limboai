/**
 * test_bb_param.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_BB_PARAM_H
#define TEST_BB_PARAM_H

#include "limbo_test.h"

#include "core/object/ref_counted.h"
#include "core/string/node_path.h"
#include "core/variant/variant.h"
#include "modules/limboai/blackboard/bb_param/bb_node.h"
#include "modules/limboai/blackboard/bb_param/bb_param.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "tests/test_macros.h"

namespace TestBBParam {

TEST_CASE("[Modules][LimboAI] BBParam") {
	Ref<BBParam> param = memnew(BBParam);
	Node *dummy = memnew(Node);
	Ref<Blackboard> bb = memnew(Blackboard);

	SUBCASE("Test with a value and common data types") {
		param->set_value_source(BBParam::SAVED_VALUE);

		param->set_saved_value(123);
		CHECK(param->get_value(dummy, bb) == Variant(123));

		param->set_saved_value("test");
		CHECK(param->get_value(dummy, bb) == Variant("test"));

		param->set_saved_value(3.14);
		CHECK(param->get_value(dummy, bb) == Variant(3.14));
	}
	SUBCASE("Test with a BB variable") {
		param->set_value_source(BBParam::BLACKBOARD_VAR);
		param->set_variable("test_var");

		SUBCASE("With integer") {
			bb->set_var("test_var", 123);
			CHECK(param->get_value(dummy, bb) == Variant(123));
		}
		SUBCASE("With String") {
			bb->set_var("test_var", "test");
			CHECK(param->get_value(dummy, bb) == Variant("test"));
		}
		SUBCASE("With float") {
			bb->set_var("test_var", 3.14);
			CHECK(param->get_value(dummy, bb) == Variant(3.14));
		}
		SUBCASE("When variable doesn't exist") {
			ERR_PRINT_OFF;
			CHECK(param->get_value(dummy, bb, "default_value") == Variant("default_value"));
			ERR_PRINT_ON;
		}
	}

	memdelete(dummy);
}

TEST_CASE("[Modules][LimboAI] BBNode") {
	Ref<BBNode> param = memnew(BBNode);
	Node *dummy = memnew(Node);
	Ref<Blackboard> bb = memnew(Blackboard);

	Node *other = memnew(Node);
	other->set_name("Other");
	dummy->add_child(other);

	SUBCASE("With a valid path") {
		param->set_value_source(BBParam::SAVED_VALUE);
		param->set_saved_value(NodePath("./Other"));
		CHECK(param->get_value(dummy, bb).get_type() == Variant::Type::OBJECT);
		CHECK(param->get_value(dummy, bb) == Variant(other));
	}
	SUBCASE("With an invalid path") {
		param->set_value_source(BBParam::SAVED_VALUE);
		param->set_saved_value(NodePath("./SomeOther"));
		ERR_PRINT_OFF;
		CHECK(param->get_value(dummy, bb, Variant()).is_null());
		ERR_PRINT_ON;
	}
	SUBCASE("With an object on the blackboard") {
		param->set_value_source(BBParam::BLACKBOARD_VAR);
		param->set_variable("test_var");

		SUBCASE("When variable exists") {
			bb->set_var("test_var", other);
			CHECK(param->get_value(dummy, bb).get_type() == Variant::Type::OBJECT);
			CHECK(param->get_value(dummy, bb) == Variant(other));
		}
		SUBCASE("When variable doesn't exist") {
			ERR_PRINT_OFF;
			CHECK(param->get_value(dummy, bb, Variant()).is_null());
			ERR_PRINT_ON;
		}
		SUBCASE("When variable has wrong type") {
			bb->set_var("test_var", 123);
			ERR_PRINT_OFF;
			CHECK(param->get_value(dummy, bb, Variant()).is_null());
			ERR_PRINT_ON;
		}
		SUBCASE("When variable is an object") {
			// * Note: We allow also fetching objects on the blackboard.
			Ref<RefCounted> some_other = memnew(RefCounted);
			bb->set_var("test_var", some_other);
			CHECK(param->get_value(dummy, bb) == some_other);
		}
	}

	memdelete(other);
	memdelete(dummy);
}

} //namespace TestBBParam

#endif // TEST_BB_PARAM_H
