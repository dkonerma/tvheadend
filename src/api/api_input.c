/*
 *  API - channel related calls
 *
 *  Copyright (C) 2013 Adam Sutton
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TVH_API_INPUT_H__
#define __TVH_API_INPUT_H__

#include "tvheadend.h"
#include "idnode.h"
#include "input.h"
#include "access.h"
#include "api.h"

static idnode_set_t *
api_input_hw_tree_sel ( int active )
{
  tvh_hardware_t *th;
  idnode_set_t *is = idnode_set_create();
  TVH_HARDWARE_FOREACH(th) {
    if (th->th_active == active)
      idnode_set_add(is, &th->th_id, NULL);
  }
  return is;
}

static idnode_set_t *
api_input_hw_tree_active ( void )
{
  return api_input_hw_tree_sel(1);
}

static idnode_set_t *
api_input_hw_tree_inactive ( void )
{
  return api_input_hw_tree_sel(0);
}

static int
api_input_hw_tree
  ( void *opaque, const char *op, htsmsg_t *args, htsmsg_t **resp )
{
  htsmsg_t *m = NULL;
  const char *uuid;
  int ret;
 
  /* UUID */
  if (!(uuid = htsmsg_get_str(args, "uuid")))
    return EINVAL;

  /* Root */
  if (!strcmp("root", uuid)) {
    *resp = htsmsg_create_list();
    m     = htsmsg_create_map();
    htsmsg_add_u32(m, "leaf", 0);
    htsmsg_add_str(m, "uuid",  "active");
    htsmsg_add_str(m, "id",  "active");
    htsmsg_add_str(m, "text", "Active");
    htsmsg_add_msg(*resp, NULL, m);
    m     = htsmsg_create_map();
    htsmsg_add_u32(m, "leaf", 0);
    htsmsg_add_str(m, "uuid",  "inactive");
    htsmsg_add_str(m, "id",  "inactive");
    htsmsg_add_str(m, "text", "In-Active");
    htsmsg_add_msg(*resp, NULL, m);
    return 0;
  }

  /* Active/Inactive */
  // Note: bit of a hack to allow re-use of api_idnode_tree to some extent
  if (!strcmp(uuid, "inactive")) {
    args   = m = htsmsg_create_map();
    htsmsg_add_str(args, "uuid", "root");
    opaque = api_input_hw_tree_inactive;
  } else if (!strcmp(uuid, "active")) {
    args   = m = htsmsg_create_map();
    htsmsg_add_str(args, "uuid", "root");
    opaque = api_input_hw_tree_active;
  }
    
  ret = api_idnode_tree(opaque, op, args, resp);
  if (m) htsmsg_destroy(m);
  return ret;
}


void api_input_init ( void )
{
  static api_hook_t ah[] = {
    { "hardware/tree", ACCESS_ADMIN,     api_input_hw_tree, NULL }, 
    { NULL },
  };

  api_register_all(ah);
}


#endif /* __TVH_API_INPUT_H__ */
