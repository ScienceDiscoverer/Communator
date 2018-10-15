#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

typedef unsigned char Byte;
typedef unsigned short Word;

//tm_sec  seconds after the minute – [0, 61](until C99) / [0, 60] (since C99)[note 1]  
//int tm_min  minutes after the hour – [0, 59]  
//int tm_hour  hours since midnight – [0, 23]  
//int tm_mday  day of the month – [1, 31]  
//int tm_mon  months since January – [0, 11]  
//int tm_year  years since 1900  
//int tm_wday  days since Sunday – [0, 6]  
//int tm_yday  days since January 1 – [0, 365]  
//int tm_isdst  Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available  

int main(int argc, char** argv)
{
	
	double e1 = 0.9, e2 = 1.68, g = 6.9579, w1 = 8.208, w2 = 8.53, w3 = 8.58, t = 73.0, i1 = 180.0, i2 = 210.0;
	
	tm e;
	e.tm_sec = 0;
	e.tm_min = 0;
	e.tm_hour = 9;
	e.tm_mday = 1;
	e.tm_mon = 0;
	e.tm_year = 2014 - 1900;
	e.tm_isdst = -1;
	
	tm gd = e;
	gd.tm_mday = 1;
	gd.tm_mon = 5;
	gd.tm_year = 2017 - 1900;
	
	tm wd1 = e;
	wd1.tm_mday = 2;
	wd1.tm_mon = 8;
	wd1.tm_year = 2017 - 1900;
	
	tm wd2 = e;
	wd2.tm_mday = 25;
	wd2.tm_mon = 0;
	wd2.tm_year = 2018 - 1900;
	
	tm wd3 = e;
	wd3.tm_mday = 1;
	wd3.tm_mon = 1;
	wd3.tm_year = 2018 - 1900;
	
	tm td = e;
	td.tm_mday = 1;
	td.tm_mon = 0;
	td.tm_year = 2018 - 1900;
	
	tm id1 = e;
	id1.tm_mday = 1;
	id1.tm_mon = 3;
	id1.tm_year = 2018 - 1900;
	
	tm id2 = e;
	id2.tm_mday = 1;
	id2.tm_mon = 5;
	id2.tm_year = 2018 - 1900;
	
	int et = (int)mktime(&e);
	int gt = (int)mktime(&gd);
	int wt1 = (int)mktime(&wd1);
	int wt2 = (int)mktime(&wd2);
	int wt3 = (int)mktime(&wd3);
	int tt = (int)mktime(&td);
	int it1 = (int)mktime(&id1);
	int it2 = (int)mktime(&id2);
		
	ofstream ofs("tariffs.cdb", ios::out | ios::app | ios::binary);
	
	ofs.write((const char*)&e1, sizeof(double));
	ofs.write((const char*)&e2, sizeof(double));
	ofs.write((const char*)&et, sizeof(int));
	
	ofs.write((const char*)&g, sizeof(double));
	ofs.write((const char*)&gt, sizeof(int));
	
	ofs.write((const char*)&w1, sizeof(double));
	ofs.write((const char*)&wt1, sizeof(int));
	
	ofs.write((const char*)&w2, sizeof(double));
	ofs.write((const char*)&wt2, sizeof(int));
	
	ofs.write((const char*)&w3, sizeof(double));
	ofs.write((const char*)&wt3, sizeof(int));
	
	ofs.write((const char*)&i1, sizeof(double));
	ofs.write((const char*)&it1, sizeof(int));
	
	ofs.write((const char*)&i2, sizeof(double));
	ofs.write((const char*)&it2, sizeof(int));
	
	ofs.write((const char*)&t, sizeof(double));
	ofs.write((const char*)&tt, sizeof(int));
	
	ofs.close();
	
	return 0;
}
//double e1 = 0.9, e2 = 1.68, g = 6.9579, w1 = 8.208, w2 = 8.53, w3 = 8.58, t = 73.0, i1 = 180.0, i2 = 210.0;



