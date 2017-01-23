/*
 * OperItem.h
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#ifndef OPERITEM_H_
#define OPERITEM_H_

#include "BaseItem.h"

typedef enum {
    OP_AND,
    OP_OR,
    OP_PIPELINE,
    OP_RUN_IN_BACKGROUND,
    OP_DELIMITER
} op_type;


class OperItem : public BaseItem {
public:
    OperItem(op_type _op_type);
    const std::string type() const;
    const op_type name();

private:
    op_type op_name;
};

#endif /* OPERITEM_H_ */
