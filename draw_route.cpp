//--------------------------------------------------------------------------------------------------
// Name: Chanpreet Singh and Kaiwen Tang
// ID : 1576137 and 1575518
// CMPUT 275 - Tangible Compting - Winter 2019
// Assignment 2 Part 2
//--------------------------------------------------------------------------------------------------

#include "draw_route.h"
#include "map_drawing.h"
#include "consts_and_types.h"

extern shared_vars shared;

void draw_route() {
	//int32_t used since Longitude_to_x and other conversition return int32_t type
	int32_t sX,sY, eX,eY;

	//This loop is used to connect every two waypoints and make route
	for (int i=0;i<(shared.num_waypoints-1);i++){
		sX = longitude_to_x(shared.map_number, shared.waypoints[i].lon);
		sY = latitude_to_y(shared.map_number, shared.waypoints[i].lat);
		eX = longitude_to_x(shared.map_number, shared.waypoints[i+1].lon);
		eY = latitude_to_y(shared.map_number, shared.waypoints[i+1].lat);
		//We use start and end points and draw a line to join them after adjusting the zoom difference
		shared.tft->drawLine(sX- shared.map_coords.x,sY- shared.map_coords.y,eX- shared.map_coords.x,eY- shared.map_coords.y,TFT_RED);
	}
}
