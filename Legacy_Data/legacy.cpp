#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef unsigned char Byte;
typedef unsigned short Word;

struct Readings
{
	int ele_1;
	int ele_2;
	int gas;
	int water;
};

struct Sums
{
	double electro;
	double gas;
	double water;
	double phone;
	double inet;
	double main_sum;
};

struct Deltas
{
	int e1;
	int e2;
	int g;
	int w;
};

struct Percents
{
	int ele;
	int gas;
	int water;
} ps;

int main(int argc, char** argv)
{
	++argv;
	Word date = (Word)stoi(argv[0]) << 8 | (Word)stoi(argv[1]);
	
	Readings reads;
	reads.ele_1 = stoi(argv[2]);
	reads.ele_2 = stoi(argv[3]);
	reads.gas = stoi(argv[4]);
	reads.water = stoi(argv[5]);
	
	Deltas dels;
	dels.e1 = stoi(argv[6]);
	dels.e2 = stoi(argv[7]);
	dels.g = stoi(argv[8]);
	dels.w = stoi(argv[9]);
	
	Sums s;
	s.electro = stod(argv[10]);
	s.gas = stod(argv[11]);
	s.water = stod(argv[12]);
	s.phone = stod(argv[13]);
	s.inet = stod(argv[14]);
	s.main_sum = stod(argv[15]);
	
	vector<char> out;
	out.insert(out.end(), (char*)&date, (char*)&date+sizeof(date));
	out.insert(out.end(), (char*)&reads, (char*)&reads+sizeof(reads));
	out.insert(out.end(), (char*)&dels, (char*)&dels+sizeof(dels));
	out.insert(out.end(), (char*)&ps, (char*)&ps+sizeof(ps));
	out.insert(out.end(), (char*)&s, (char*)&s+sizeof(s));
	
	ofstream ofs("months.cdb", ios::out | ios::app | ios::binary);
	
	ofs.write(out.data(), out.size());
	
	ofs.close();
	
	return 0;
}

0    1  2  3 4 5   6   7  8  9 10 11 12 13 14 15
1 2018 e1 e2 g w de1 de2 dg dw se sg sw sp si ms




