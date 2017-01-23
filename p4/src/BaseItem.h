/*
 * BaseItem.h
 *
 *  Created on: Jan 23, 2017
 *      Author: Roman Vasilyev
 */

#ifndef BASEITEM_H_
#define BASEITEM_H_

#include <iostream>

#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#include <cstring>
#include <cstdlib>

#include <string>
#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

class BaseItem
{
public:
    virtual const std::string type() const = 0;
    virtual ~BaseItem();
};

#endif /* BASEITEM_H_ */
