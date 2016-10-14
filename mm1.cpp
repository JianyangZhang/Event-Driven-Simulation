/* @author: Jianyang Zhang */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <queue>
#include <math.h>
using namespace std;

void InitRandom(long);
int ExponentialInterval(double);

class packet{
public:
	packet(){
		arrival_time=0,inter_time=0,
		service_time=0,index=0;
	}
			
	int arrival_time; //cumulative time for this packet arriving
	int inter_time; //time between this packet arriving and the previous packet arriving
	int service_time; //service time that this packet requests
	int index; //packet number 
	
	void disp(){
		printf("\nindex = %d, arrival_time = %d, service_time = %d, inter_time = %d\n",
			index,arrival_time,service_time,inter_time);
	}
}; 

double st(int mstime) // change millisecond to second
{
	double stime = (double)mstime*0.001;
	return stime;
}

bool busy = false;
int end_server_time=0;
int clk = 0;
int cnt_pkg = 0;
int total_inter=0;
int total_service=0;
int *pinq;
int *pinsys;
queue<packet> q;
packet server_packet;
packet *pkgs;

// p# arrives and enters Q, inter-arrival time = ?s 
bool arrival_event(int clk, bool disTime, int max)
{
	if(cnt_pkg<max && clk == pkgs[cnt_pkg].arrival_time)
	{
		q.push(pkgs[cnt_pkg]);
		if(disTime){
			printf("              ");
		}
		else{
			printf("%012.3fs: ",st(clk));
		}
		printf("p%d arrives and enters Q, inter-arrival time = %.3fs\n",
			pkgs[cnt_pkg].index,st(pkgs[cnt_pkg].inter_time));
		cnt_pkg++;
		return true;
	}
	return false;
}

//p# leaves Q and begins service at S, time in Q = ?s, requesting ?s of service
bool departure_event(int clk, bool disTime)
{
	if(!busy && !q.empty())
	{
		server_packet = q.front();		
		if(disTime){
			printf("               ");
		}
		else{
			printf("%012.3fs: ",st(clk));
		}
	    pinq[server_packet.index-1] = clk-server_packet.arrival_time;
		printf("p%d leaves Q and begins service at S, time in Q = %0.3fs, requesting %0.3fs of service\n",
				server_packet.index,st(clk-server_packet.arrival_time),st(server_packet.service_time));
		end_server_time = clk+server_packet.service_time;
		q.pop();
		busy = true;
		return true;
	}
	return false;
}

//p# departs from S, service time = ?s, time in system = ?s
bool leave_server_event(int clk,bool disTime)
{
	if(clk == end_server_time && busy)
	{
		if(disTime){
			printf("              ");
		}
		else{
			printf("%012.3fs: ",st(clk));
		}
		pinsys[server_packet.index-1] = end_server_time-server_packet.arrival_time;
		printf("p%d departs from S, service time = %.3fs,  time in system = %.3fs\n",
			server_packet.index,
			st(server_packet.service_time),
			st(end_server_time-server_packet.arrival_time));
		busy = false;
		return true;
	}
	return false;
}

int main(int argc,char *argv[])
{	
	double lambda = 1.0;
	double mu = 0.7;	
	int packet_num = 20;
	long seed = 0;
	int ch;
	char filename[20];
	int mode=1;	// 1 for deterministic, 2 for exponential, 3 for trace-driven


	bool chk_lambda=false;
	bool chk_mu = false;
	bool chk_det = false;
	bool chk_exp = false;
	bool chk_s = false;
	bool chk_t = false;
	
	while((ch = getopt(argc,argv,"l:m:d:e:s:t:")) !=-1)
	{
		switch(ch)
		{
			case 'l':	// [-lambda lambda]
				if(strcmp(argv[optind-1],"-lambda")!=0)
				{
					fprintf(stderr,"ERROR: wrong parameter\n");
					exit(0);
				}
				lambda = atof(argv[optind]);
				if(lambda<=0)
				{
					fprintf(stderr,"ERROR: wrong lambda\n");
					exit(0);
				}
				chk_lambda = true;
				optind++;
				break;
			case 'm':	// [-mu mu]
				if(strcmp(argv[optind-1],"-mu")!=0)
				{
					fprintf(stderr,"ERROR: wrong parameter\n");
					exit(0);
				}
				mu = atof(argv[optind]);
				if(mu<=0)
				{
					fprintf(stderr,"ERROR: wrong mu\n");
				}
				chk_mu = true;
				optind++;
				break;
			case 'd':	// [-det num]
				if(strcmp(argv[optind-1],"-det")!=0)
				{
					fprintf(stderr,"ERROR: wrong parameter, input -det\n");
					exit(0);
				}
				packet_num = atoi(argv[optind]);
				if(packet_num<1 || packet_num>0x7fffffff)
				{
					fprintf(stderr,"ERROR: number of packets is out of range\n");
					exit(0);
				}
				chk_det = true;
				optind++;
				break;
			case 'e':	//[-exp num]
				if(strcmp(argv[optind-1],"-exp")!=0)
				{
					fprintf(stderr,"ERROR: wrong parameter, input -exp\n");
					exit(0);
				}
				packet_num = atoi(argv[optind]);
				if(packet_num<1 || packet_num>0x7fffffff)
				{
					fprintf(stderr,"ERROR: number of packet is out of range\n");
					exit(0);
				}
				mode =2;
				chk_exp = true;
				optind++;
				break;
			case 's':	//[-s seed]
				seed = atoi(optarg);
				if(seed<0)
				{
					fprintf(stderr,"ERROR: seed should be greater than 0\n");
					exit(0);
				}
				chk_s = true;
				break;
			case 't':	//[-t tsfile]
				mode = 3;
				strcpy(filename,optarg);
				chk_t = true;
				break;
			case '?':	// no such optional parameters OR lack of param
				printf("wrong parameters OR lack of parameters!!!\n");
				exit(0);
		}
	}

	if(chk_t)
	{
		mode =3;
	}else if(chk_det || (!chk_t && !chk_exp))
	{
		mode = 1;
	}else if(chk_exp)
	{
		mode =2;
	}

	if(mode == 3) // read data from the file
	{
		FILE * fp;
		fp = fopen(filename,"r");
		fscanf(fp,"%d",&packet_num);		
		pkgs = new packet[packet_num];
		int temp1,temp2;		
		for(int i= 0 ;i<packet_num;i++)
		{
			fscanf(fp,"%d %d\n",&temp1,&temp2);
			pkgs[i].inter_time = temp1;
			pkgs[i].service_time = temp2;
			pkgs[i].arrival_time = total_inter+temp1;
			pkgs[i].index = i+1;
			total_inter+=temp1;
			total_service+=temp2;			
		}
		fclose(fp);
	}else if(mode == 1)	//deterministic mode
	{		
		int inter = (int)(1000.0/lambda);
		pkgs = new packet[packet_num];
		for(int i = 0 ;i<packet_num;i++)
		{
			pkgs[i].inter_time = inter;
			pkgs[i].service_time = (int) (1000.0/mu);
			pkgs[i].arrival_time = total_inter+inter;
			pkgs[i].index = i+1;
			total_inter+= inter;
			total_service+= pkgs[i].service_time;			
		}
	}else if(mode == 2) //exponential mode
	{
		InitRandom(seed);		
		pkgs = new packet[packet_num];
		for(int i= 0 ;i<packet_num;i++)
		{
			pkgs[i].inter_time = ExponentialInterval(lambda);
			pkgs[i].service_time = ExponentialInterval(mu);
			pkgs[i].arrival_time = total_inter + pkgs[i].inter_time;
			pkgs[i].index = i+1;
			total_inter+= pkgs[i].inter_time;
			total_service += pkgs[i].service_time;			
		}
	}
	
	// Simulation Parameters
	pinsys = new int[packet_num];
	pinq = new int[packet_num];
	
	printf("Simulation Parameters:\n");
	printf("\tnumber to arrive = %d\n",packet_num);
	if(!chk_t)
	{
		printf("\tlambda = %0.2f\n",lambda);
		printf("\tmu = %0.2f\n",mu);
	}
	if(chk_det || chk_exp)
	{
		if(mode==2){printf("\tmode = exp\n");}
		if(mode==3){printf("\tmode = det\n");}
	}
	if(chk_exp)
	{
		printf("\tseed = %d\n",seed);
	}
	if(chk_t)
	{
		printf("\ttsfile = %s\n",filename);
	}
	printf("\n");
	printf("%012.3fs: simulation begins\n",0.0);
	// simulation begins---------------------------------------------------	
	bool hasTime = false;
	bool arrival_flag = false;
	end_server_time =pkgs[0].arrival_time;
	clk = 0;	
	while(cnt_pkg<packet_num || !q.empty() || busy)
	{
		arrival_flag = false;
		if(cnt_pkg<packet_num && clk == pkgs[cnt_pkg].arrival_time){
			arrival_flag = true;
		}

		if(arrival_flag || clk >= end_server_time ){
			if(arrival_event(clk,hasTime,packet_num)){hasTime = true;}
			if(leave_server_event(clk,hasTime)){hasTime = true;}
			departure_event(clk,hasTime);
			hasTime = false;		
		}
		clk++;
	}	
	int simulation_time = clk-1;
	// simulation ends-------------------------------------------------------------------
	delete pkgs;
	printf("               ");
	printf("simulation ends\n\n");

	int sq = 0;
	for(int i = 0 ; i<packet_num;i++)
	{
		sq += pinq[i];
	}
	printf("Statistics:\n\n");
	printf("\taverage packet inter-arrival time = %.3fs\n",st(total_inter)/packet_num);
	printf("\taverage packet service time = %.6fs\n\n",st(total_service)/packet_num);
	printf("\taverage number of packets in Q = %.6fs\n",st(sq)/st(simulation_time));
	printf("\taverage number of packets at S = %.6fs\n\n",st(total_service)/st(simulation_time));
	double s = 0.0,m=0.0;
	
	for(int i = 0 ;i<packet_num;i++)
	{
		s+= (double)pinsys[i];
	}
	m = s/packet_num;
	double std = 0.0;
	for(int i = 0;i<packet_num;i++)
	{
		std += ((double)pinsys[i] - m)*((double)pinsys[i]-m)*1e-6;
	}
	std = sqrt(std);
	printf("\taverage time a packet spent in system = %.6fs\n",m*0.001);
	printf("\tstandard deviation for time spent in system = %.6fs\n\n",std);
	printf("\tpacket drop probability = 0\n");
	return 0;
}


