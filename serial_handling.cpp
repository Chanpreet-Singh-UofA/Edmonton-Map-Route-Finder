//--------------------------------------------------------------------------------------------------
// Name: Chanpreet Singh and Kaiwen Tang
// ID : 1576137 and 1575518
// CMPUT 275 - Tangible Compting - Winter 2019
// Assignment 2 Part 2
//--------------------------------------------------------------------------------------------------


#include "serial_handling.h"
#include <Arduino.h>
#include "consts_and_types.h"

extern shared_vars shared;

// current number of chars in buffer, not counting null terminator
uint16_t buf_len = 0;
const uint16_t buf_size = 256;
// input buffer
char* buffer = (char *) malloc(buf_size);
int waypoint_index = 0;


//This function is used to read the characters from serial and add characters into the buffer untill we get \r ot \n
//Disclaimer: This Function is developed using in-class example provided to students
int reading(int amount){
  long long start_time=0, end_time=0, time_taken=0;
  start_time=millis();
  //We read untill time outs
  while(time_taken<amount){
    if(Serial.available()){
      char command=Serial.read();
      if(command=='E'){
        return 0;
      }
      if (command == '\n' || command == '\r') {
        // we return 1 whenever line have been added to buffer
          return 1;
      }
      else{
        // add character to buffer, provided that we don't overflow.
        // drop any excess characters.
        if ( buf_len < buf_size-1 ) {
          buffer[buf_len] = command;
          buf_len++;
          buffer[buf_len] = 0;
        }
      }
    }
    end_time=millis();
    time_taken=end_time-start_time;
  }
}

uint8_t get_waypoints(const lon_lat_32& start, const lon_lat_32& end) {

  enum {REQUEST, WAYPOINTS, END} curr_mode = REQUEST;

  shared.num_waypoints = 0;
  waypoint_index = 0;
  char input;

  //This loop runs untill we dont finish the request stage
  while(true){
    if(curr_mode==REQUEST){
      Serial.print("R ");
      Serial.print(start.lat);
      Serial.print(" ");
      Serial.print(start.lon);
      Serial.print(" ");
      Serial.print(end.lat);
      Serial.print(" ");
      Serial.println(end.lon);
      Serial.flush();
      //We read for max of 10 sec for the number of waypoints and then process the line
      reading(10000);
      String input = strtok(buffer," ");// N
      if(input[0]=='N'){
        input = strtok(NULL," "); //wayp-points
        shared.num_waypoints = input.toInt(); // number of way points  
        //if we get 0 or more then 500 waypoints then its a NO PATH case. Thus we go to END stage
        if ( shared.num_waypoints == 0 or shared.num_waypoints >= 500){
          buf_len = 0;
          buffer[buf_len] = 0;
          waypoint_index = 0;
          curr_mode=END;
        }
        else{
          Serial.print("A\n"); //send acknowledgement
          Serial.flush();
          buf_len = 0;
          buffer[buf_len] = 0;
          curr_mode=WAYPOINTS;
        }
        //we bread the while loop and go to WAYPOINT stage once we get number of waypoints
        break;
      }
      else{
        //if we get wrong input in 10 sec then we refresh buffer and and send the request again
        buf_len = 0;
        buffer[buf_len] = 0;
      }
    }
  }

  //This stage is used to process the waypoints being rescieved
  if(curr_mode==WAYPOINTS){
    //we use 1 sec timeout
    while(reading(1000)){
      String input = strtok(buffer," ");
      if(input[0]=='W'){
        String lati=strtok(NULL," ");
        String loni=strtok(NULL," ");
        shared.waypoints[waypoint_index]=lon_lat_32(loni.toInt(),lati.toInt());
        waypoint_index ++;
        //send acknowledgement
        Serial.print("A\n"); 
        Serial.flush();    
        buf_len = 0;
        buffer[buf_len] = 0;
      }
    }
    curr_mode=END; 
  }

  //This stage is reached after processing all the waypoints or 0/500 waypoints case
  if(curr_mode==END){
    Serial.flush();
    return 1;
  }
}
