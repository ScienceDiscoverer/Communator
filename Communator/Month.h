#ifndef MONTH_H
#define MONTH_H

#include <vector>

#define SUM_NEW 0
#define SUM_RECALC 1

// Add this to user interface
const double t2_coef = 0.5; // Coefficient for nigh-time electricity
const int min_tar_end = 100; // Maximum electro energy applied to small tariff


typedef unsigned short Word;
typedef unsigned char Byte;

using namespace std;

class DataBase;

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

struct Tariff
{
	double price;
	int update_time;
};

struct ElectroTariff
{
	double less_min;
	double more_min;
	int update_time;
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
};

struct ReadTariff
{
	Readings rd;
	double e1;
	double e2;
	double g;
	double w;
	double i;
	double p;
	bool state;
};

class Month
{
public:
	Month() = default;
	Month(Byte month, Byte year);
	Month(Word date) : date_(date) {}

	void SetDate(Word d) { date_ = d; }
	void SetReadings(const Readings& rd) { reads_ = rd; }
	Readings GetReadings() const { return reads_; }
	Percents GetPercents() const { return percents_; }
	Deltas GetDeltas() const { return delta_; }

	Sums GetSums(const DataBase& db, int sum_type);

	Sums GetSums() const { return sums_; }
	double MaxSum() const;
	double MinSum() const;


	Byte Year() const { return (Byte)date_; }
	Byte Mon() const { return (Byte)(date_ >> 8); }
	Word Date() const { return date_; }

	vector<char> Serialize() const;
	void Deserialize(vector<char>& v);
	string Csv() const;

private:
	Word date_;
	Readings reads_;
	// Deltas
	Deltas delta_;
	Percents percents_;
	Sums sums_;
};

#endif /* MONTH_H */