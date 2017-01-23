/*
 * OperItem.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#include "OperItem.h"

OperItem::OperItem(op_type _op_type) : op_name(_op_type) { }

const std::string OperItem::type() const {
	return "op";
}

const op_type OperItem::name() {
	return op_name;
}
