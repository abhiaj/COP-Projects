#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<math.h>
#include<fstream>
#include <iomanip>
#include<string>
#include<vector>
#include<chrono>
#include<thread>
#include "vehicles.h"
#include <GL/glut.h> 
  // using namespace std::this_thread; // sleep_for, sleep_until
  // using namespace std::chrono; // nanoseconds, system_clock, seconds
  // position denotes top=right corner
  //x => down, y => right
using namespace std;

class status
{
	public: int lane_auto;
			float position_auto[2];
			int shift_auto;
	status(int l, float p[2], int s)
	{
		lane_auto=l;
		position_auto[0]=p[0]; position_auto[1]=p[1]; 
		shift_auto=s;
	}
};

vector< vector<Vehicle> > input_gl;
int g_road_width, g_road_length; //for use by open gl
vector<int> g_signal;
int red_light;
int zebra;
int bikes_on_zebra[]={0,0,0,0};
//time for each frame 
float refreshMills = 30;
//int num_lanes;
int lane_width;
vector<status> autos_at_odd_angles;
int curr_signal;

vector<Vehicle> speed_control(vector<Vehicle> input)
{
		vector<vector<Vehicle>> vehicles_row_wise; 
		//vector<Vehicle> out_f; //for storing vehicles row wise in asc order
		// forming a matrix of vehicles

		for(int k=-4;k<=4;k++) //number of lanes to be updated here
		{
			//int temp_pos_y=psdout[k].position[1];
			vector<Vehicle> vehicles_samerow;
			for(int i=0;i<input.size();i++)
			{
				if(input[i].lane==k )
				{
					vehicles_samerow.push_back(input[i]);
					//psdout.erase(psdout.begin()+i);
					//i=0; k=0;
				}

			}
			//sort in asc
			for(int i1=0;i1<vehicles_samerow.size();i1++)
			{
				for(int i2=0;i2<vehicles_samerow.size()-1;i2++)
				{
					if(vehicles_samerow[i2].position[0]>vehicles_samerow[i2+1].position[0]) 
					{
						Vehicle temp= vehicles_samerow[i2];
						vehicles_samerow[i2]=vehicles_samerow[i2+1];
						vehicles_samerow[i2+1]=temp;
					}
				}
			}
			vehicles_row_wise.push_back(vehicles_samerow);
		}

		/*for(int i=0;i<vehicles_row_wise.size();i++)
		{
			vector<Vehicle> temp=vehicles_row_wise[i];
			for(int k=0;k<temp.size();k++)
			{
				//cout<<temp[k].position[0]<<","<<temp[k].position[1]<<endl;
			}
		}*/

		//cout<<"bleh";
		//updating speed by collision conditions (collision handling by changing speed)
		vector <Vehicle> speed_changed;

		for(int i=0; i<vehicles_row_wise.size();i++)
		{
			vector<Vehicle> vehicles_samerow=vehicles_row_wise[i];

			for(int k=vehicles_samerow.size()-1; k>0;k--) //change here for speed ontroll
			{
				Vehicle v= vehicles_samerow[k];

				if( vehicles_samerow[k].position[0] - vehicles_samerow[k-1].position[0]<=(vehicles_samerow[k].length+2) || 
					vehicles_samerow[k].position[0]+vehicles_samerow[k].speed - vehicles_samerow[k-1].position[0]+vehicles_samerow[k-1].speed <=(vehicles_samerow[k].length+2)  )
					if(vehicles_samerow[k].speed<vehicles_samerow[k-1].speed)
						vehicles_samerow[k-1].speed=vehicles_samerow[k].speed;

				/*if( vehicles_samerow[k].position[0] - vehicles_samerow[k-1].position[0] >= (vehicles_samerow[k].length+2) )
					if(vehicles_samerow[k].position[0]+vehicles_samerow[k].speed - vehicles_samerow[k-1].position[0]+vehicles_samerow[k-1].speed <=(vehicles_samerow[k].length+2))
						vehicles_samerow[k-1].speed=vehicles_samerow[k].speed;*/

				//cout<<i<<" nuh"<< k<<endl;
				speed_changed.push_back(vehicles_samerow[k]);

			}
			if(vehicles_samerow.empty()!=1)
			speed_changed.push_back(vehicles_samerow[0]);
		}

		return speed_changed;
}

vector<Vehicle> lane_change(vector<Vehicle> input)
{
	vector<vector<Vehicle>> vehicles_row_wise; 
	int rows = g_road_width; int columns = g_road_length;
		//vector<Vehicle> out_f; //for storing vehicles row wise in asc order

		// forming a matrix of vehicles
		//start
		for(int k=1;k<=4;k++) //number of lanes to be updated here
		{
			//int temp_pos_y=psdout[k].position[1];
			vector<Vehicle> vehicles_samerow;
			for(int i=0;i<input.size();i++)
			{


				if(input[i].lane==k || input[i].lane==-k)
				{
					vehicles_samerow.push_back(input[i]);
					//psdout.erase(psdout.begin()+i);
					//i=0; k=0;
				}

			}
			//sort in asc
			for(int i1=0;i1<vehicles_samerow.size();i1++)
			{
				for(int i2=0;i2<vehicles_samerow.size()-1;i2++)
				{
					if(vehicles_samerow[i2].position[0]>vehicles_samerow[i2+1].position[0])
					{
						Vehicle temp= vehicles_samerow[i2];
						vehicles_samerow[i2]=vehicles_samerow[i2+1];
						vehicles_samerow[i2+1]=temp;
					}
				}
			}
			vehicles_row_wise.push_back(vehicles_samerow);
		}
		//end

		vector < vector < int > > pmatrix2(rows, vector< int >(columns,0)); 
		//pmatrix2=pmatrix;
		int left_shift=0,right_shift=0;

		//collision control
		//start
		for(int i=0; i<vehicles_row_wise.size();i++)
		{
			vector<Vehicle> vehicles_samerow=vehicles_row_wise[i];

			for(int k=vehicles_samerow.size()-1; k>0;k--)
			{
				//updating matrix
				//start
				for(int ii=0;ii<rows;ii++)
				for(int jj=0;jj<columns;jj++)
					pmatrix2[ii][jj]=0;

				for(int i=0; i<vehicles_row_wise.size();i++)
				{
					vector<Vehicle> temp_row=vehicles_row_wise[i];

					for(int k=0; k<temp_row.size();k++)
					{
						Vehicle v = temp_row[k];
						int l = v.length; int w = v.width; int x = v.position[0]; int y = v.position[1];

						for(int p = x;p>x-l;p--)
						{
							for(int q = y;q<y+w;q++)
							{
								if(p>=0 && p<=g_road_length-1 && q>=0 && q<=g_road_width-1)
								{
									pmatrix2[q][p]=1;
								}
							}
						}

					}
				}
				//cout<<endl;
				for(int i=0;i<rows;i++)
				{
					for(int j=0;j<columns;j++)
					{
						if(j==columns-1)
						{
					//cout<<pmatrix2[i][j]<<"\n";
						}
						else
						{
					//cout<<pmatrix2[i][j]<<" ";
						}
					}
				}
				//end

				Vehicle v= vehicles_samerow[k-1];
				Vehicle temp=vehicles_samerow[k];

				//colliion check 
				//start

			if(temp.lane==v.lane)
			{
				if( temp.position[0] - v.position[0]<=(temp.length+2) || 
					temp.position[0]+(temp.speed*10) - v.position[0]-(v.speed*10) <=(temp.length+2)  )
				{	//cout<<"bc";
					if(temp.speed<v.speed || temp.max_speed< v.max_speed)
					{
						//int check_lanechange=-1;
							if(v.lane<=3 && v.lane>=0) 
							{
								left_shift=0,right_shift=0;
								//cout<<"cf";
								int check_lanechange=0;
								//checking for possibility of lane shift
								//cout<<v.position[0]+1<<" "<<v.position[1]+5<<endl;
								for( int i1=v.position[0]+2; i1>=v.position[0]-v.length-2 ; i1--) //error here
								{
									//cout<<"cf";
									for(int j=v.position[1]+lane_width;j<=v.position[1]+lane_width+3;j++) //width change bt
									{
										if(pmatrix2[j][i1]==1)
										{
											check_lanechange=1;
											//cout<<"bhc";
											goto rsexy;

										}
										//cout<<pmatrix2[j][i1];
										//check_lanechange=1;
									}
									//cout<<endl;
								}
								rsexy:
								//cout<<"b";
								//cout<<check_lanechange<<" "<<v.position[1]<<endl;
								if(check_lanechange==0)
								{
									right_shift=1;
								}
								//cout<<v.lane;
							}

							//left shift
							if(v.lane>=2)
							{
								int check_lanechange=0;
								left_shift=0,right_shift=0;
								//checking for possibility of lane shift
								for(int i1=v.position[0]+1;i1>=v.position[0]-v.length;i1--)
								{
									for(int j=v.position[1]-lane_width;j<=v.position[1]-1;j++) //bt
									{
										if(pmatrix2[j][i1]==1)
										{
											check_lanechange=1;
											goto lsexy;
										}
									}
								}
								lsexy:
							if(check_lanechange==0)
								{
									left_shift=1;
								}
								//cout<<v.lane;
							}


							if(right_shift==1 && temp.speed!=0)
							{
								//if(v.lane<=3)
								{
									vehicles_row_wise[i][k-1].position[1]+=lane_width;
									vehicles_row_wise[i][k-1].lane++;
									//cout<<v.lane;
									//psdout3.push_back(v);
								}
							}

							else if(left_shift==1 && temp.speed!=0)
							{
								//if(v.lane>=2)
								{
									vehicles_row_wise[i][k-1].position[1]-=lane_width;
									vehicles_row_wise[i][k-1].lane--;

								}
						//cout<<v.lane;
							}
					}	//vehicles_samerow[k-1].speed=vehicles_samerow[k].speed;
				}
				//end

				
			}	//cout<<i<<" nuh"<< k<<endl;
				//psdout3.push_back([k-1]);

			}
			//if(vehicles_samerow.empty()!=1)
			//psdout3.push_back(vehicles_samerow[0]);
		}
		//end

		vector<Vehicle> lane_changed;

		for(int i=0; i<vehicles_row_wise.size();i++)
				{
					vector<Vehicle> vehicles_samerow=vehicles_row_wise[i];

					for(int k=vehicles_samerow.size()-1; k>=0;k--)
					{
						lane_changed.push_back(vehicles_samerow[k]);

					}
				}

	return lane_changed;
}

vector<Vehicle> handle_bikes(vector<Vehicle> input) //jab chal raha hai tab bhi change
{
	vector<Vehicle> bike_handled;
	for(int i=0; i<input.size(); i++)
	{
		Vehicle v=input[i];

		if(v.type=='b')
		{
			if(v.speed==0)
			{
				if(v.lane>0)
				{
					int num_auto_lane=0;
					for(int i=0; i<input.size(); i++)
					{
						Vehicle v2=input[i];
						if(v2.lane==(v.lane) && v2.type=='A')
						{
							num_auto_lane++;
						}
					}

					if(v.position[0]<red_light && num_auto_lane==0)
					{
						v.position[1]+=2; //bt
						v.lane-=2*(v.lane);
					}
				}

				else if(v.lane<0)
				{
					if(v.position[0]>red_light+2 && v.position[0]<zebra) //may change //on zebra
					{
						v.lane-=2*v.lane;
						int num= bikes_on_zebra[v.lane-1];
						for(int i=0; i<num; i++ )
						{
							v.position[0]+=v.length+1;
						}
						v.position[1]-=2;   //bt
						bikes_on_zebra[v.lane-1]++;
					}
				}
			}
		}
		//cout<<v.speed<<endl;
		bike_handled.push_back(v);
	}

	return bike_handled;
}

vector<Vehicle> handle_autos(vector<Vehicle> input)
{
	vector<vector<Vehicle>> vehicles_row_wise; 
		//vector<Vehicle> out_f; //for storing vehicles row wise in asc order
		// forming a matrix of vehicles

		for(int k=1;k<=4;k++) //number of lanes to be updated here
		{
			//int temp_pos_y=psdout[k].position[1];
			vector<Vehicle> vehicles_samerow;
			for(int i=0;i<input.size();i++)
			{
				if(input[i].lane==k || input[i].lane==-k)
				{
					vehicles_samerow.push_back(input[i]);
					//psdout.erase(psdout.begin()+i);
					//i=0; k=0;
				}

			}
			//sort in asc
			for(int i1=0;i1<vehicles_samerow.size();i1++)
			{
				for(int i2=0;i2<vehicles_samerow.size()-1;i2++)
				{
					if(vehicles_samerow[i2].position[0]>vehicles_samerow[i2+1].position[0]) 
					{
						Vehicle temp= vehicles_samerow[i2];
						vehicles_samerow[i2]=vehicles_samerow[i2+1];
						vehicles_samerow[i2+1]=temp;
					}
				}
			}
			vehicles_row_wise.push_back(vehicles_samerow);
		}

	vector<Vehicle> autos_handled;
	//vector<Vehicle> autos_handled1;

	for(int i=0; i<vehicles_row_wise.size();i++)
	{
		vector<Vehicle> vehicles_samerow= vehicles_row_wise[i];
		for(int k=0; k<vehicles_samerow.size(); k++)
		{
			Vehicle v=vehicles_samerow[k];

			if(v.type=='A' && curr_signal==0)
			{
				if(v.speed==0)
				{
						//v.angle=20.0;
					//v.lane++;
					//v.position[0]+=lane_width;
						//v.position[1]+=1.5;
						//if(v.lane>0)
						//v.lane-=2*v.lane;
						//status temp= status(v.lane, v.position, 1); //1 for riight shift 0 for left
						//autos_at_odd_angles.push_back(temp);
						float theta=v.angle;
						float d= sqrt(v.length*v.length + v.width*v.width);
						if( v.position[1]+d*cos(theta) < (v.lane)*lane_width+1 )
						{
							v.angle+=1;
							v.position[1]+=0.08;
						}
					

				}
			}

			if(v.type=='A' && curr_signal==1)
			{
				if(v.angle!=0)
				{
					v.angle-=1;
					//v.position[1]=(-v.lane-1)*lane_width+2;
					if(v.position[1]<(v.lane-1)*lane_width+1)
					{
						v.position[1]+=0.05;
					}
					//v.lane-=2*v.lane;
				}
			}
			autos_handled.push_back(v);
		}
	}

	/*if(curr_signal==1)
	{

	for(int i=0; i<autos_at_odd_angles.size(); i++)
	{
		status temp= autos_at_odd_angles[i];
		int index=temp.lane_auto-1+ temp.shift_auto;
		vector<Vehicle> vehicles_samerow= vehicles_row_wise[index];

		for(int j=0; j<vehicles_samerow.size();j++)
		{
			Vehicle v=vehicles_samerow[j];
			if(v.position[0]<temp.position_auto[0])
			{
				if(v.position[0]>temp.position_auto[0]-6) //6 needs to be changed here
				{
					vehicles_row_wise[index][j].speed=0;
				}
			}
		}
	}

	}*/

	

	return autos_handled;
}

vector<Vehicle> run(Road r,vector<Vehicle> vehicles, Traffic_Signal t,int go)
{

	//input_gl.push_back(vehicles);
	

	red_light = g_road_length/2 ;
	zebra=red_light+g_road_length/10;

	//int lc;
	//vector<Vehicle> psdout,out;float rr;
	vector<Vehicle> psdout1,psdout2,psdout3,psdout4,psdout5,psdout6,out; 
	int rows = r.width; int columns = r.length;
	char canvas[rows][columns];
	//vector < vector < int > > pmatrix(rows, vector< int >(columns,0));

	for(int i=0;i<vehicles.size();i++)
	{
		if(vehicles[i].position[0]==0)
		{
		if(vehicles[i].lane==1)
			vehicles[i].position[1]=0;
		else if(vehicles[i].lane==2)
			vehicles[i].position[1]=lane_width;
		else if(vehicles[i].lane==3)
			vehicles[i].position[1]=2*lane_width;
		else if(vehicles[i].lane==4)
			vehicles[i].position[1]=3*lane_width;
		}
	}

	//storing the state of vheicles and traffuc light
	input_gl.push_back(vehicles);
	g_signal.push_back(t.signal);

	//initial updating of road matrix

	for(int i=0;i<rows;i++)
		for(int j=0;j<columns;j++)
			canvas[i][j]=' ';

	for(int i=0;i<vehicles.size();i++)
	{
		Vehicle v = vehicles[i];
		int l = v.length; int w = v.width; int x = v.position[0]; int y = v.position[1];

		for(int p = x;p>x-l;p--){
			for(int q = y;q<y+w;q++){
				if(p>=0 && p<=r.length-1 && q>=0 && q<=r.width-1){
					canvas[q][p]=v.type;
				}
			}
		} 
	}

	for(int i=0;i<rows;i++)
	{
		canvas[i][red_light]='|';
		canvas[i][zebra]='|';
	}

	//print
	cout<<endl<<"-------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
	
			for(int i=0;i<rows;i++){
				for(int j=0;j<columns;j++){
					if(j==columns-1){
						cout<<canvas[i][j]<<"\n";
					}
					else
					{
					cout<<canvas[i][j]<<" ";
					}
				}
			}

		//first normal updation of speed
		for(int i=0;i<vehicles.size();i++)
		{
			Vehicle v=vehicles[i];
			v.speed+=v.acc;
			if(v.speed>=v.max_speed )
			{
				v.speed=v.max_speed;
			}
			if(v.angle!=0)
				v.speed=0;
			//cout<<v.speed;
			psdout1.push_back(v);
		}

		//checking traffic light and updaave to type "run" before "record"). If you want to stting speeeds
		if(t.signal==0)
		{
			for(int i=0;i<psdout1.size();i++)
			{
				Vehicle v=psdout1[i];

				if(v.type!='b')
				{
				if(v.position[0]>=red_light)
					if(v.position[0]-v.length<red_light)
						v.speed=0;

				if(v.position[0]<red_light)
					if(v.position[0]+v.speed>red_light)
						v.speed=0;

				//goint to cross in next step
				else if(v.position[0]+v.speed>=red_light && (v.position[0]+v.speed-v.length<red_light) )
					v.speed=0;
				}

				else if(v.type='b')
				{
					if(v.position[0]<zebra && v.position[0]>red_light+2)
						v.speed=0;
				}

				

				psdout2.push_back(v);
			}
		}
		else
		{
			psdout2=psdout1;
		}


		//psdout3=psdout2;
		psdout3=lane_change(psdout2);
		//psdout3=psdout2;
		
		psdout4=speed_control(psdout3);
		//psdout4=psdout3;

		if(t.signal==0)
		{
			psdout5=handle_bikes(psdout4);
		}
		
		else
		{
			psdout5=psdout4;
		}
		
		
		psdout6=handle_autos(psdout5);

		//updating position of vehicles
		for(int i=0;i<psdout6.size();i++)
		{
			//Vehicle v= psdout4[i];
			psdout6[i].position[0]+=psdout6[i].speed;
			//out_f.push_back(v);
		}

		//reoving vehicles frm class object who have crossed ht eroad
		/*for(int i=0; i<psdout5.size(); i++)
		{
			Vehicle v= psdout5[i];
			if(v.position[0]>=0 && v.position[0]<=g_road_length)
			{
				psdout6.push_back(v);
			}
		}*/
		
			//cout<<"\n";
		if(go>0){
			out = run(r,psdout6,t,go-1);
		}
		else{
			out=psdout6;
		}
	return out;	
//	sleep_for(seconds(1));
}

//vector<

vector<string> parse(string filename){
	fstream inFile;vector<string> out;
    inFile.open(filename);
    string config;
    if (!inFile) {
        cout << "Unable to open file";
        exit(1); 
    }
	while (inFile >> config) {	
	stringstream file(config);
	
	string line;
	while(getline(file,line)){
		istringstream is_line(line);
		string key;
		 if(line.c_str()[0]=='#'){
		 	
		 	}	
		 else if(getline(is_line, key, '=') )
  			{
    			
				string value;
    			if( getline(is_line, value) ){
      			
					out.push_back(key);
      				out.push_back(value);
      			
      			}
  			}
	}
	
}
return out;		
}

void initGL() {
   // Set "clearing" or background color
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black and opaque
   //cout<<input_gl.size();
}

void Timer(int value) {
   glutPostRedisplay();      // Post re-paint request to activate display()
   glutTimerFunc(refreshMills, Timer, 0); // next Timer call milliseconds later
}

int number_sim=0;

void display() {
   glClear(GL_COLOR_BUFFER_BIT);   // Clear the color buffer
   glMatrixMode(GL_MODELVIEW);     // To operate on Model-View matrix
   glLoadIdentity();               // Reset the model-view matrix
 	
 	glTranslatef(-1.0f,1.0f,0.0f);
 	float x_scale= (float)2/g_road_length; float y_scale= (float)2/g_road_width/2;

 	glBegin(GL_LINES);

 	if(g_signal[number_sim]==0)
 	{
 		glColor3f(1.0f,0.0f,0.0f);
 	}
 	else
 	{
 		glColor3f(0.0f,1.0f,0.0f);
 	}
 	glVertex2f(1.0f,0.0f);
 	glVertex2f(1.0f,-1.0f);

 	glEnd();
 	
 	glPushMatrix();

 	glTranslatef(0.99f,-1.1f,0.0f);

 	glBegin(GL_POLYGON);

    for(int ii = 0; ii < 20; ii++)
    {
        float theta = 2.0f * 3.1415926f * float(ii) / float(20);//get the current angle

        float x = 0.1 * cosf(theta);//calculate the x component
        float y = 0.1* sinf(theta);//calculate the y component

        glVertex2f(x + 0.01, y + 0.01);//output vertex

    }
    glEnd(); 

    glPopMatrix();


 	glBegin(GL_QUADS);

 	glColor3f(1.0f,1.0f,1.0f);

 	for(float i=0;i<1.0;i+=0.2)
 	{
 	glVertex2f(1.0f,0.0f-i);
 	glVertex2f(1.0f,-0.1f-i);
 	glVertex2f(1.2f,-0.1f-i);
 	glVertex2f(1.2f,0.0f-i);
 	}
 	
 	glEnd();

 	glBegin(GL_LINES);

 	glVertex2f(1.2f,0.0f);
 	glVertex2f(1.2f,-0.8f);

 	glEnd();

 	vector<Vehicle> tempv= input_gl[number_sim];

 	for(int k=0; k<tempv.size();k++)
 	{

 		glPushMatrix();                     // Save model-view matrix setting
 		float x_pos=(float)tempv[k].position[0]*x_scale, y_pos=(float)-tempv[k].position[1]*y_scale;
 		float length= (float)tempv[k].length*x_scale; float width= (float)tempv[k].width*y_scale;
 		
 		//Add directed light
    	if(tempv[k].position[0]<=g_road_length)
    	{
 		
   		glTranslatef(x_pos, y_pos, 0.0f); 
   		if(tempv[k].angle!=0)
  		{
  		glRotatef(tempv[k].angle, 0.0f, 0.0f, 1.0f);
  		//cout<<"bc"<<endl;
  		}
   
   glBegin(GL_QUADS);                  // Each set of 4 vertices form a quad

   		if(tempv[k].colour=="RED")
   			glColor3f(1.0f,0.0f,0.0f);
   		else if(tempv[k].colour=="GREEN")
   			glColor3f(0.0f,1.0f,0.0f);
   		else if(tempv[k].colour=="BLUE")
   			glColor3f(0.0f,0.0f,1.0f);
   		else if(tempv[k].colour=="YELLOW")
		   	glColor3f(1.0f,1.0f,0.0f);
		else if(tempv[k].colour=="WHITE")
		   	glColor3f(1.0f,1.0f,1.0f);
      
      
      glVertex2f(0.0f,0.0f);
      glVertex2f(-length,0.0f);
      glVertex2f(-length, -width);
      glVertex2f(0.0f, -width);
  	


   glEnd();

		}

   glBegin(GL_LINES);

   	glColor3f(0.0f,0.0f,0.0f);

   	  glVertex2f(0.0f,0.0f);
   	  glVertex2f(-length,0.0f);

      glVertex2f(-length,0.0f);
      glVertex2f(-length, -width);

      glVertex2f(-length, -width);
      glVertex2f(0.0f, -width);

      glVertex2f(0.0f, -width);
      glVertex2f(0.0f,0.0f);

   glEnd();

   		glPopMatrix(); 
 	}

   
   glutSwapBuffers();   // Double buffered - swap the front and back buffers
 
   if(number_sim<=input_gl.size())
   	number_sim++;
   else
   	glFlush();

}

void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
   // Compute aspect ratio of the new window
   if (height == 0) height = 1;                // To prevent divide by 0
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   // Set the viewport to cover the new window
   glViewport(0, 0, width, height);
 
   // Set the aspect ratio of the clipping area to match the viewport
   glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
   glLoadIdentity();
   if (width >= height) {
     // aspect >= 1, set the height from -1 to 1, with larger width
      gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
   } else {
      // aspect < 1, set the width to -1 to 1, with larger height
     gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
   }
}



int main(int argc, char** argv){
   	int road_id,road_width,road_length,n_iter;
   	float speed,car_acc,bike_acc,bus_acc,truck_acc,auto_acc;
   	float car_maxspeed,bike_maxspeed,bus_maxspeed,truck_maxspeed,auto_maxspeed;
   	int car_length,car_width,bike_width,bike_length,bus_length,bus_width,truck_length,truck_width,auto_length,auto_width;
	Traffic_Signal t = Traffic_Signal(0);
	Road r = Road(1,5,30);
	//int ider = 0;
	vector<Vehicle> out; vector<Vehicle> vehicles; vector<string> sth; 
	float y= 0;
	int lane=0;
	speed=0;
	string key,value,vehicle_colour;
	sth = parse(argv[1]);
	for(int i=0;i<sth.size();i++){
		key=sth[i];
		if(key=="Road_Id"){
			 i=i+1;
			 value=sth[i];
			 road_id = stoi(value); 
		}
		else if(key=="Road_Width"){
			 i=i+1;
			 value=sth[i];
			 road_width = stoi(value);
			 g_road_width=road_width;
			 lane_width=g_road_width/4;
			 r = Road(road_id,road_length,road_width);
		}
		else if(key=="Road_Length"){
			 i=i+1;
			 value=sth[i];
			 road_length = stoi(value);
			 g_road_length=road_length;
			 
		}
		else if(key=="Vehicle_Type"){
			 i=i+1;
			 value=sth[i];
			 if(value=="Car"){
			 	i=i+1;
			 	key=sth[i];
			 	 if(key=="Vehicle_Length"){
			 		i=i+1;
			 		value=sth[i];
			 		car_length = stoi(value);
				 }
				i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Width"){
			 		i=i+1;
			 		value=sth[i];
			 		car_width = stoi(value);
				 }
				i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Acceleration"){
			 		i=i+1;
			 		value=sth[i];
			 		car_acc = stof(value);
			 		//cout<<"yooooooooooooooooo";
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_MaxSpeed"){
			 		i=i+1;
			 		value=sth[i];
			 		car_maxspeed = stof(value);
			 		//cout<<value;
			 		//cout<<"caryooooooooooooooooo";
				 }
			 }
			 if(value=="Bike"){
			 	i=i+1;
			 	key=sth[i];
			 	if(key=="Vehicle_Length"){
			 		i=i+1;
			 		value=sth[i];
			 		bike_length = stoi(value);
				 }
				 i++;
				 key=sth[i];
				 if(key=="Vehicle_Width"){
			 		i=i+1;
			 		value=sth[i];
			 		bike_width = stoi(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Acceleration"){
			 		i=i+1;
			 		value=sth[i];
			 		bike_acc = stof(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_MaxSpeed"){
			 		i=i+1;
			 		value=sth[i];
			 		bike_maxspeed = stof(value);
				 }
			 }
			 if(value=="Bus"){
			 	i=i+1;
			 	key=sth[i];
			 	if(key=="Vehicle_Length"){
			 		i=i+1;
			 		value=sth[i];
			 		bus_length = stoi(value);
				 }
				i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Width"){
			 		i=i+1;
			 		value=sth[i];
			 		bus_width = stoi(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Acceleration"){
			 		i=i+1;
			 		value=sth[i];
			 		bus_acc = stof(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_MaxSpeed"){
			 		i=i+1;
			 		value=sth[i];
			 		bus_maxspeed = stof(value);
				 }			 
				 }
			 if(value=="Truck"){
			 	i=i+1;
			 	key=sth[i];
			 	if(key=="Vehicle_Length"){
			 		i=i+1;
			 		value=sth[i];
			 		truck_length = stoi(value);
				 }
				i++;
				key=sth[i]; 
				 if(key=="Vehicle_Width"){
			 		i=i+1;
			 		value=sth[i];
			 		truck_width = stoi(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Acceleration"){
			 		i=i+1;
			 		value=sth[i];
			 		truck_acc = stof(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_MaxSpeed"){
			 		i=i+1;
			 		value=sth[i];
			 		truck_maxspeed = stof(value);
				 }
			 }
			  if(value=="Auto"){
			 	i=i+1;
			 	key=sth[i];
			 	if(key=="Vehicle_Length"){
			 		i=i+1;
			 		value=sth[i];
			 		auto_length = stoi(value);
				 }
				i++;
				key=sth[i]; 
				 if(key=="Vehicle_Width"){
			 		i=i+1;
			 		value=sth[i];
			 		auto_width = stoi(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_Acceleration"){
			 		i=i+1;
			 		value=sth[i];
			 		auto_acc = stof(value);
				 }
				 i++; 
				key=sth[i]; 
				 if(key=="Vehicle_MaxSpeed"){
			 		i=i+1;
			 		value=sth[i];
			 		auto_maxspeed = stof(value);
				 }
			 }
		}
		else if(key=="SIGNAL"){
			 i=i+1;
			 value=sth[i];//cout<<value;
			 if(value=="RED"){
			 	t.signal=0;
			 	curr_signal=0;
			 }
			 else{
			 	t.signal=1;
			 	curr_signal=1;
			 }
		}
		else if(key=="Car"){
			 i=i+1;
			 value=sth[i];
			 vehicle_colour=value;
			 if(lane==4){
			 	lane=1;
			 }
			 else{
			 	lane=lane+1;
			 }
			 Vehicle c = Vehicle(car_length,car_width,speed,car_acc,car_maxspeed,0,y,lane,vehicle_colour,'C');
		     vehicles.push_back(c);
		}
		else if(key=="bike"){
			 i=i+1;
			 value=sth[i];
			 vehicle_colour=value;
			 if(lane==4){
			 	lane=1;
			 }
			 else{
			 	lane=lane+1;
			 }
			vehicle_colour=value;  //cout<<"chaljaaa";
			Vehicle b = Vehicle(bike_length,bike_width,speed,bike_acc,bike_maxspeed,0,y,lane,vehicle_colour,'b');
			vehicles.push_back(b); 
		}
		else if(key=="Bus"){
			 i=i+1;
			 value=sth[i];
			 vehicle_colour=value;
			 if(lane==4){
			 	lane=1;
			 }
			 else{
			 	lane=lane+1;
			 }
			Vehicle b = Vehicle(bus_length,bus_width,speed,bus_acc,bus_maxspeed,0,y,lane,vehicle_colour,'B');
			vehicles.push_back(b);
		}
		else if(key=="Truck"){
			 i=i+1;
			 value=sth[i];
			 vehicle_colour=value;
			 if(lane==4){
			 	lane=1;
			 }
			 else{
			 	lane=lane+1;
			 }
			 vehicle_colour=value;
			 Vehicle t = Vehicle(truck_length,truck_width,speed,truck_acc,truck_maxspeed,0,y,lane,vehicle_colour,'T'); 
				 vehicles.push_back(t);
		}
		else if(key=="Auto"){
			 i=i+1;
			 value=sth[i];
			 vehicle_colour=value;
			 if(lane==4){
			 	lane=1;
			 }
			 else{
			 	lane=lane+1;
			 }
			 vehicle_colour=value;
			 Vehicle a = Vehicle(auto_length,auto_width,speed,auto_acc,auto_maxspeed,0,y,lane,vehicle_colour,'A'); 
				 vehicles.push_back(a);
		}
		else if(key=="Pass"){
			 i=i+1;
			 value=sth[i];
			 n_iter = stoi(value);
			 y=0;
			 vehicles=run(r,vehicles,t,n_iter);
		}
	}


   glutInit(&argc, argv);          // Initialize GLUT
   glutInitDisplayMode(GLUT_DOUBLE);  // Enable double buffered mode
   glutInitWindowSize(640, 480);   // Set the window's initial width & height - non-square
   glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
   glutCreateWindow("Animation via Idle Function");  // Create window with the given title
   glutDisplayFunc(display);       // Register callback handler for window re-paint event
   glutReshapeFunc(reshape);       // Register callback handler for window re-size event
   glutTimerFunc(0, Timer, 0);     // First timer call immediately
   initGL();                       // Our own OpenGL initialization
   glutMainLoop();                 // Enter the infinite event-processing loop
	return 0;
}
