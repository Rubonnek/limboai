/**
 * test_call_method.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef TEST_EVALUATE_EXPRESSION_H
#define TEST_EVALUATE_EXPRESSION_H

#include "limbo_test.h"

#include "modules/limboai/blackboard/bb_param/bb_node.h"
#include "modules/limboai/blackboard/blackboard.h"
#include "modules/limboai/bt/tasks/bt_task.h"
#include "modules/limboai/bt/tasks/utility/bt_evaluate_expression.h"

#include "core/os/memory.h"
#include "core/variant/array.h"

namespace TestCallMethod {

TEST_CASE("[Modules][LimboAI] BTEvaluateExpression") {
	Ref<BTEvaluateExpression> ee = memnew(BTEvaluateExpression);

	SUBCASE("When node parameter is null") {
		ee->set_node_param(nullptr);
		ERR_PRINT_OFF;
		CHECK(ee->execute(0.01666) == BTTask::FAILURE);
		ERR_PRINT_ON;
	}

	SUBCASE("With object on the blackboard") {
		Node *dummy = memnew(Node);
		Ref<Blackboard> bb = memnew(Blackboard);

		Ref<BBNode> node_param = memnew(BBNode);
		ee->set_node_param(node_param);
		Ref<CallbackCounter> callback_counter = memnew(CallbackCounter);
		bb->set_var("object", callback_counter);
		node_param->set_value_source(BBParam::BLACKBOARD_VAR);
		node_param->set_variable("object");
		ee->set_expression_string("callback()");

		ee->initialize(dummy, bb);

		SUBCASE("When expression string is empty") {
			ee->set_expression_string("");
			ERR_PRINT_OFF;
			CHECK(ee->parse() == FAILED);
			CHECK(ee->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
		}
		SUBCASE("When expression string calls non-existent function") {
			ee->set_expression_string("not_found()");
			ERR_PRINT_OFF;
			CHECK(ee->parse() == OK);
			CHECK(ee->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
		}
		SUBCASE("When expression string accesses a non-existent property") {
			ee->set_expression_string("not_found");
			ERR_PRINT_OFF;
			CHECK(ee->parse() == OK);
			CHECK(ee->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
		}
		SUBCASE("When expression string can't be parsed") {
			ee->set_expression_string("assignment = failure");
			ERR_PRINT_OFF;
			CHECK(ee->parse() == ERR_INVALID_PARAMETER);
			CHECK(ee->execute(0.01666) == BTTask::FAILURE);
			ERR_PRINT_ON;
		}
		SUBCASE("When expression is valid") {
			ee->set_expression_string("callback()");
			ERR_PRINT_OFF;
			CHECK(ee->parse() == OK);
			CHECK(ee->execute(0.01666) == BTTask::SUCCESS);
			ERR_PRINT_ON;
			CHECK(callback_counter->num_callbacks == 1);
		}
		SUBCASE("With arguments") {
			ee->set_expression_string("callback_delta(delta)");

			SUBCASE("Should fail with 0 arguments") {
				ee->set_input_include_delta(false);
				ee->set_input_vars(Vector<String>());
				ee->set_input_values(TypedArray<BBVariant>());
				ERR_PRINT_OFF;
				CHECK(ee->parse() == OK);
				CHECK(ee->execute(0.01666) == BTTask::FAILURE);
				ERR_PRINT_ON;
				CHECK(callback_counter->num_callbacks == 0);
			}
			SUBCASE("Should succeed with too many inputs") {
				ee->set_input_include_delta(true);
				Vector<String> input_vars;
				input_vars.push_back("point_two");
				ee->set_input_vars(input_vars);
				TypedArray<BBVariant> input_values;
				input_values.push_back(memnew(BBVariant(0.2)));
				ee->set_input_values(input_values);
				ERR_PRINT_OFF;
				CHECK(ee->parse() == OK);
				CHECK(ee->execute(0.01666) == BTTask::SUCCESS);
				ERR_PRINT_ON;
				CHECK(callback_counter->num_callbacks == 1);
			}
			SUBCASE("Should fail with a wrong type arg") {
				ee->set_input_include_delta(false);
				Vector<String> input_vars;
				input_vars.push_back("delta");
				ee->set_input_vars(input_vars);
				TypedArray<BBVariant> input_values;
				input_values.push_back(memnew(BBVariant("wrong data type")));
				ee->set_input_values(input_values);
				ERR_PRINT_OFF;
				CHECK(ee->parse() == OK);
				CHECK(ee->execute(0.01666) == BTTask::FAILURE);
				ERR_PRINT_ON;
				CHECK(callback_counter->num_callbacks == 1);
			}
			SUBCASE("Should succeed with delta included") {
				ee->set_input_include_delta(true);
				ee->set_input_vars(Vector<String>());
				ee->set_input_values(TypedArray<BBVariant>());
				ERR_PRINT_OFF;
				CHECK(ee->parse() == OK);
				CHECK(ee->execute(0.01666) == BTTask::SUCCESS);
				ERR_PRINT_ON;
				CHECK(callback_counter->num_callbacks == 1);
			}
			SUBCASE("Should succeed with one float arg") {
				ee->set_input_include_delta(false);
				Vector<String> input_vars;
				input_vars.push_back("delta");
				ee->set_input_vars(input_vars);
				TypedArray<BBVariant> input_values;
				input_values.push_back(memnew(BBVariant(0.2)));
				ee->set_input_values(input_values);
				ERR_PRINT_OFF;
				CHECK(ee->parse() == OK);
				CHECK(ee->execute(0.01666) == BTTask::SUCCESS);
				ERR_PRINT_ON;
				CHECK(callback_counter->num_callbacks == 1);
			}
		}

		memdelete(dummy);
	}
}

} //namespace TestCallMethod

#endif // TEST_EVALUATE_EXPRESSION_H
