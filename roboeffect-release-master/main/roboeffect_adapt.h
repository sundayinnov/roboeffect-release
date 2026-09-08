#ifndef __ROBOEFFECT_ADAPT_H__
#define __ROBOEFFECT_ADAPT_H__


#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"


typedef struct _roboeffect_adapt_device_node
{
	uint32_t io_id;
	uint32_t width;
	uint32_t ch;
	char name[32];
}roboeffect_adapt_device_node;


typedef struct _roboeffect_adapt_device_table
{
	uint32_t count;
	const roboeffect_adapt_device_node table[];
}roboeffect_adapt_device_table;

extern const roboeffect_adapt_device_table demo_adapt_device_table;

#endif/*__ROBOEFFECT_ADAPT_H__*/

