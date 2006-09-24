/***************************************************************************\
*                                                                           *
*  BitlBee - An IRC to IM gateway                                           *
*  Jabber module - Handling of presence (tags), etc                         *
*                                                                           *
*  Copyright 2006 Wilmer van der Gaast <wilmer@gaast.net>                   *
*                                                                           *
*  This program is free software; you can redistribute it and/or modify     *
*  it under the terms of the GNU General Public License as published by     *
*  the Free Software Foundation; either version 2 of the License, or        *
*  (at your option) any later version.                                      *
*                                                                           *
*  This program is distributed in the hope that it will be useful,          *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*  GNU General Public License for more details.                             *
*                                                                           *
*  You should have received a copy of the GNU General Public License along  *
*  with this program; if not, write to the Free Software Foundation, Inc.,  *
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
*                                                                           *
\***************************************************************************/

#include "jabber.h"

xt_status jabber_pkt_presence( struct xt_node *node, gpointer data )
{
	struct gaim_connection *gc = data;
	char *from = xt_find_attr( node, "from" );
	char *type = xt_find_attr( node, "type" );	/* NULL should mean the person is online. */
	char *s;
	
	if( !from )
		return XT_HANDLED;
	
	s = strchr( from, '/' );
	if( s )
		*s = 0;
	
	if( type == NULL )
		serv_got_update( gc, from, 1, 0, 0, 0, 0, 0 );
	else if( strcmp( type, "unavailable" ) == 0 )
		serv_got_update( gc, from, 0, 0, 0, 0, 0, 0 );
	else if( strcmp( type, "subscribe" ) == 0 )
		jabber_buddy_ask( gc, from );
	else if( strcmp( type, "subscribed" ) == 0 )
		serv_got_crap( gc, "%s just accepted your authorization request", from );
	else if( strcmp( type, "unsubscribe" ) == 0 || strcmp( type, "unsubscribed" ) == 0 )
	{
		/* Do nothing here. Plenty of control freaks or over-curious
		   souls get excited when they can see who still has them in
		   their buddy list and who finally removed them. Somehow I
		   got the impression that those are the people who get
		   removed from many buddy lists for "some" reason...
		   
		   If you're one of those people, this is your chance to write
		   your first line of code in C... */
	}
	else
	{
		printf( "Received PRES from %s:\n", from );
		xt_print( node );
	}
	
	if( s )
		*s = '/';
	
	return XT_HANDLED;
}

/* Whenever presence information is updated, call this function to inform the
   server. */
int presence_send_update( struct gaim_connection *gc )
{
	struct jabber_data *jd = gc->proto_data;
	struct xt_node *node;
	char *show = jd->away_state->code;
	char *status = jd->away_message;
	int st;
	
	node = jabber_make_packet( "presence", NULL, NULL, NULL );
	if( show && *show )
		xt_add_child( node, xt_new_node( "show", show, NULL ) );
	if( status )
		xt_add_child( node, xt_new_node( "status", status, NULL ) );
	/* if( set_getint( &gc->acc->set, "priority" ) != 0 ) */
	/* Let's just send this every time... */
		xt_add_child( node, xt_new_node( "priority", set_getstr( &gc->acc->set, "priority" ), NULL ) );
	
	st = jabber_write_packet( gc, node );
	
	xt_free_node( node );
	return st;
}

/* Send a subscribe/unsubscribe request to a buddy. */
int presence_send_request( struct gaim_connection *gc, char *handle, char *request )
{
	struct xt_node *node;
	int st;
	
	node = jabber_make_packet( "presence", NULL, NULL, NULL );
	xt_add_attr( node, "to", handle );
	xt_add_attr( node, "type", request );
	
	st = jabber_write_packet( gc, node );
	
	xt_free_node( node );
	return st;
}
