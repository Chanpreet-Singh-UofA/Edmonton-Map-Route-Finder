//--------------------------------------------------------------------------------------------------
// Name: Chanpreet Singh and Kaiwen Tang
// ID : 1576137 and 1575518
// CMPUT 275 - Tangible Compting - Winter 2019
// Assignment 2 Part 2
//--------------------------------------------------------------------------------------------------


#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <list>
#include <string.h>
#include "wdigraph.h"
#include "dijkstra.h"
#include "serialport.h"

struct Point {
	long long lat, lon;
};

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

// finds the id of the point that is closest to the given point "pt"
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();
  for (const auto& check : points) {
	if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
	  best = check;
	}
  }
  return best.first;
}

// read the graph from the file that has the same format as the "Edmonton graph" file
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;
  while (getline(fin, line)) {
	// split the string around the commas, there will be 4 substrings either way
	string p[4];
	int at = 0;
	for (auto c : line) {
	  if (c == ',') {
		// start new string
		++at;
	  }
	  else {
		// append character to the string we are building
		p[at] += c;
	  }
	}
	if (at != 3) {
	  // empty line
	  break;
	}
	if (p[0] == "V") {
	  // new Point
	  int id = stoi(p[1]);
	  assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
	  points[id].lat = static_cast<long long>(stod(p[2])*100000);
	  points[id].lon = static_cast<long long>(stod(p[3])*100000);
	  g.addVertex(id);
	}
	else {
	  // new directed edge
	  int u = stoi(p[1]), v = stoi(p[2]);
	  g.addEdge(u, v, manhattan(points[u], points[v]));
	}
  }
}


//------------------------------------------------------------------------------------------------
//Part 2 

enum {REQUEST, PROCESS, NUM_WAYPOINTS, WAIT, WAYPOINTS} curr_mode = REQUEST;

int main() {

  	SerialPort Serial("/dev/ttyACM0");
  	Point sPoint, ePoint;
  	string line;

	//This loop never ends and keep server running
	while(true){

	  if(curr_mode==REQUEST){
		cout<<"Waiting for input from the Client"<<endl;
		//When we get \n then we process line and see if its expected thing or not
		line = Serial.readline();
		if(line[0]=='R'){
			//If we resceived right statement then its processed and coordinates are stored.
			// Expected Syntax: R <start lat> <start lon> <end lat> <end lon>
			int space1=line.find(' ',2);
			sPoint.lat=stoi(line.substr(2, space1));
			int space2=line.find(' ',space1+1);
			sPoint.lon =stoi( line.substr(space1+1, space2));
			int space3=line.find(' ',space2+1);
			ePoint.lat=stoi( line.substr(space2+1, space3));
			ePoint.lon=stoi( line.substr(space3+1,'\n'));
			cout<<"Input resceived: R "<<sPoint.lat<<" "<<sPoint.lon<<" "<<ePoint.lat<<" "<<ePoint.lon<<endl;
			curr_mode=PROCESS;
		}
	  }

	  if(curr_mode==PROCESS){
	  	//At this stage we make the graph and path ready as per part 1, we have other stages inside
	  	//process since we want to be able to use process initializations in other processes.
		WDigraph graph;
		unordered_map<int, Point> points;
		readGraph("edmonton-roads-2.0.1.txt", graph, points);
		int start,end;	
		start = findClosest(sPoint, points);
		end = findClosest(ePoint, points);
		unordered_map<int, PIL> tree;
		dijkstra(graph, start, tree);
		list<int> path;
		curr_mode=NUM_WAYPOINTS;
	  
		  //At this stage we send number of waypoints
		if(curr_mode==NUM_WAYPOINTS){
			// no path
			if (tree.find(end) == tree.end()) {
				cout << "NO PATH - Please wait for few sec for screen to refresh and be able to select points again" << endl;
				Serial.writeline("N 0");
				curr_mode=REQUEST;
			}
			else {
				while (end != start) {
				  path.push_front(end);
				  end = tree[end].first;
				}
				path.push_front(start);
				// output the path
				cout << "N " << path.size() << endl;
				Serial.writeline("N "+ to_string(path.size())+"\n");
				//If we get more then 500 waypoints then we use NO PATH Mechanism
				if(path.size()>=500){
				  cout << "More then 500 waypoint so using NO PATH Mechanism" << endl;
				  cout<<"Wait for few sec for screen to refresh and be able to select points again"<<endl;
				  curr_mode=REQUEST;
				}
				curr_mode=WAIT;
			}
		}

		//After sending number of waypoints we wait for Achnoledgement
		if(curr_mode==WAIT){
			string ack=Serial.readline();
			if(ack[0]=='A'){
				curr_mode=WAYPOINTS;
			}else{
				cout<<"Timeout for Ack with - "<<ack<<endl;
				cout<<"User can select two points to find shortest way between them again"<<endl;
				curr_mode=REQUEST;
			}
		}

		//This stage is used to send all the waypoints to the client and rescieve ack.
		if(curr_mode==WAYPOINTS){
			for (int v : path) {
			  cout << "W " << points[v].lat << ' ' << points[v].lon << endl;
			  Serial.writeline("W "+ to_string(points[v].lat) +" " + to_string(points[v].lon) +"\n");
			  string a=Serial.readline();
			  if(a[0]!='A'){
				cout<<"waypoint no acknoledgement"<<endl;
				break;
			  }
			}
			cout << "E" << endl;
			Serial.writeline("E");
			//Now we get ready again to get two points and find shortest distance between them.
			cout<<"Wait for few sec for screen to refresh and be able to see the route."<<endl;
			curr_mode=REQUEST;
		}
	  }
	}
  return 0;
}
