#include <cmath>
#include <string>
#include "Month.h"
#include "DB.h"
#include "Helpers.h"

// Helper funcs
double round2(double x);
string dts(double d);

// Month definition
Month::Month(Byte month, Byte year)
{
	date_ = (Word)month << 8 | (Word)year;
}

Sums Month::GetSums(const DataBase& db, int sum_type)
{
	Readings past_read;
	if(sum_type == SUM_NEW)
	{
		past_read = db.GetPastReads();
	}
	else
	{
		past_read = db.GetPastPastReads();
	}
	
	// Find deltas
	delta_.e1 = reads_.ele_1 - past_read.ele_1;
	delta_.e2 = reads_.ele_2 - past_read.ele_2;
	delta_.g = reads_.gas - past_read.gas;
	delta_.w = reads_.water - past_read.water;

	// Compute electro sum
	int edsum = delta_.e1 + delta_.e2; // Electro Delta sum
	
	double e1_perc = (double)delta_.e1/(double)edsum * 100.0; // Tariff 1 usage percentage
	double e2_perc = (double)delta_.e2/(double)edsum * 100.0; // Tariff 2 usage percentage
	
	double e1_1 = e1_perc * db.TariffElectro().less_min;
	double e2_1 = e2_perc * db.TariffElectro().less_min * t2_coef;

	double e1_2 = ((double)delta_.e1 - e1_perc) * db.TariffElectro().more_min;
	double e2_2 = ((double)delta_.e2 - e2_perc) * db.TariffElectro().more_min * t2_coef;

	sums_.electro = round2(e1_1 + e1_2 + e2_1 + e2_2 + db.TariffElectroTrans());

	// Compute other sums
	sums_.gas = round2(db.TariffGas() * delta_.g + db.TariffGasTrans());
	sums_.water = round2(db.TariffWater() * delta_.w + db.TariffWaterTrans());
	sums_.inet = round2(db.TariffInet());
	sums_.bankfee = round2(db.TariffBankfee());

	// Compute main sum
	sums_.main_sum = round2(sums_.electro + sums_.gas + sums_.water + sums_.inet + sums_.bankfee);

	// Compute percents
	Deltas pd; 
	if(sum_type == SUM_NEW)
	{
		pd = db.GetPastDeltas();
	}
	else
	{
		pd = db.GetPastPastDeltas();
	}

	if(pd.e1 == 0.0)
	{
		return sums_;
	}

	int tot_p_dele = pd.e1 + pd.e2;
	int tot_dele = delta_.e1 + delta_.e2;

	if(tot_dele > tot_p_dele)
	{
		percents_.ele = (int)round(((double)tot_dele/(double)tot_p_dele - 1.0) * 100.0);
	}
	else
	{
		percents_.ele = ((int)round(((double)(tot_p_dele - tot_dele)/(double)tot_p_dele) * 100.0)) * -1;;
	}

	if(delta_.g > pd.g)
	{
		percents_.gas = (int)round(((double)delta_.g/(double)pd.g - 1.0) * 100.0);
	}
	else
	{
		percents_.gas = ((int)round(((double)(pd.g - delta_.g)/(double)pd.g) * 100.0)) * -1;;
	}

	if(delta_.w > pd.w)
	{
		percents_.water = (int)round(((double)delta_.w/(double)pd.w - 1.0) * 100.0);
	}
	else
	{
		percents_.water = ((int)round(((double)(pd.w - delta_.w)/(double)pd.w) * 100.0)) * -1;;
	}

	return sums_;
}

double Month::MaxSum() const
{
	double* s = (double*)&sums_;
	double max = -1.0;
	for(int i = 0; i < 3; ++i)
	{
		if(s[i] > max)
		{
			max = s[i];
		}
	}

	return max;
}

double Month::MinSum() const
{
	double* s = (double*)&sums_;
	double min = 1000000.0;
	for(int i = 0; i < 3; ++i)
	{
		if(s[i] < min)
		{
			min = s[i];
		}
	}

	return min;
}

vector<char> Month::Serialize() const
{
	vector<char> out;
	out.insert(out.end(), (char*)&date_, (char*)&date_+sizeof(date_));
	out.insert(out.end(), (char*)&reads_, (char*)&reads_+sizeof(reads_));
	out.insert(out.end(), (char*)&delta_, (char*)&delta_+sizeof(delta_));
	out.insert(out.end(), (char*)&percents_, (char*)&percents_+sizeof(percents_));
	out.insert(out.end(), (char*)&sums_, (char*)&sums_+sizeof(sums_));

	return out;
}

void Month::Deserialize(vector<char>& v)
{
	char* data = v.data();

	date_ = *(Word*)data;
	data += sizeof(Word);
	reads_ = *(Readings*)data;
	data += sizeof(Readings);
	delta_ = *(Deltas*)data;
	data += sizeof(Deltas);
	percents_ = *(Percents*)data;
	data += sizeof(Percents);
	sums_ = *(Sums*)data;
}

string Month::Csv() const
{
	const string c = ",";
	string out = to_string((Byte)(date_ >> 8)) + "/" + to_string((Byte)date_);
	// Electro
	out += c + dts(sums_.electro) + c + to_string(delta_.e1) + c + to_string(delta_.e2);
	out += c + to_string(reads_.ele_1) + c + to_string(reads_.ele_2);
	// Gas
	out += c + dts(sums_.gas) + c + to_string(delta_.g) + c + to_string(reads_.gas);
	// Water
	out += c + dts(sums_.water) + c + to_string(delta_.w) + c + to_string(reads_.water);
	// Other
	out += c + dts(sums_.inet) + c + dts(sums_.bankfee) + c + dts(sums_.main_sum) + "\n";

	return out;
}
// End of Month definition

// Helper funcs definitions
inline double round2(double x)
{
	return round(x * 100)/100.0;
}

inline string dts(double d)
{
	return dToStrFull(d);
}