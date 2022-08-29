/* bt_repeat_until_failure.h */

#ifndef BT_REPEAT_UNTIL_FAILURE_H
#define BT_REPEAT_UNTIL_FAILURE_H

#include "bt_decorator.h"
#include "core/object.h"

class BTRepeatUntilFailure : public BTDecorator {
	GDCLASS(BTRepeatUntilFailure, BTDecorator);

protected:
	virtual int _tick(float p_delta);
};

#endif // BT_REPEAT_UNTIL_FAILURE_H