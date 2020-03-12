#pragma once
#include "HDLinkedListNode.h"
#include "TypeDefine.h"

struct IHDLinkedList 
{
	virtual HDLinkedListNode get_node(HINT64 offset) = 0;
};