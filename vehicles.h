#ifndef HELLO
#define HELLO

#include <bits/stdc++.h> 
using namespace std;

class Vehicle {
	public:	int id;
			int length;
			int width;
			float speed;
	  		float acc;
			string colour;
			float max_speed;
			float position[2];
			int lane;
			char type;
			float angle=0;
			Vehicle(int l,int w,float s,float a,float m_s,float x,float y,int lan, string str, char c){
				length = l;
				width = w;
				speed = s;
				//id = iddd;
				acc = a;
				//brake = b;
				colour=str;
				//follow = f;
				//laneChangeFreq = lcf;
				max_speed = m_s;
				position[0] = x; position[1] = y;
				lane=lan;
				type=c;
			}						
};

class Road {
	public: int id;
			int length;
			int width;
	Road(int i,int l,int w){
		id = i;
		length = l;
		width = w;
	}
	};

class Traffic_Signal {
	public: int signal;
	Traffic_Signal(int s){
		signal = s;
	}
};




#endif
