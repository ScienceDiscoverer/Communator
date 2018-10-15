#include <fstream>
#include <ctime>
#include "DB.h"
#include "Helpers.h"

Month empty_month;

// DataBase implementation
void DataBase::SetTariffs(double e1, double e2, double g, double w, double i, double p)
{
	CheckAndSet(electro_, e1, e2);
	CheckAndSet(gas_, g);
	CheckAndSet(water_, w);
	CheckAndSet(inet_, i);
	CheckAndSet(phone_, p);
}

Month& DataBase::GetLastMonth()
{
	if(months_.Size() <= 0)
	{
		return empty_month;
	}

	return months_.Last();
}

const Month& DataBase::GetLastMonth() const
{
	if(months_.Size() <= 0)
	{
		return empty_month;
	}

	return months_.Last();
}

MinMax DataBase::GetMinMax() const
{
	MinMax mm;
	mm.max = -1.0;
	mm.min = 1000000.0;
	Byte y = GetLastMonth().Year();

	for(Months::Index i = months_.Front(); i != months_.End(); ++i)
	{
		// Account for last month of pervious year
		if(i->Year() != y && (i->Year() < y-1 || i->Mon() != 12))
		{
			continue;
		}
		
		if(mm.max < i->MaxSum())
		{
			mm.max = i->MaxSum();
		}

		if(mm.min > i->MinSum())
		{
			mm.min = i->MinSum();
		}
	}

	return mm;
}

const SumArrays& DataBase::GetSumArrays() const
{
	static SumArrays sa;
	Byte y = GetLastMonth().Year();

	sa.n = 0;
	sa.pre_m = false;
	for(Months::Index i = months_.Front(); i != months_.End(); ++i)
	{
		if(i->Year() == y)
		{
			++sa.n;
		}
		else if(i->Year() == y-1 && i->Mon() == 12)
		{
			sa.pre_m = true; // Previous month found
		}
	}

	sa.n += (int)sa.pre_m;

	int j = 0;
	for(Months::Index i = months_.Front(); i != months_.End(); ++i)
	{
		if(i->Year() != y && (i->Year() < y-1 || i->Mon() != 12))
		{
			continue;
		}

		const Sums s = i->GetSums();

		sa.ele[j] = s.electro;
		sa.gas[j] = s.gas;
		sa.wat[j] = s.water;
		++j;
	}

	return sa;
}

bool DataBase::Save()
{
	ofstream ofs("months.cdb", ios::out | ios::trunc | ios::binary);
	bool r; // Result

	for(Months::Index i = months_.Front(); i != months_.End(); ++i)
	{
		vector<char> bytes = i->Serialize();
		r = (bool)ofs.write(bytes.data(), bytes.size());
	}

	ofs.close();

	ofs.open("tariffs.cdb", ios::out | ios::trunc | ios::binary);

	int s = (int)electro_.size();
	r = (bool)ofs.write((char*)&s, sizeof(int));
	r = (bool)ofs.write((char*)electro_.data(), sizeof(ElectroTariff) * s);
	s = (int)gas_.size();
	r = (bool)ofs.write((char*)&s, sizeof(int));
	r = (bool)ofs.write((char*)gas_.data(), sizeof(Tariff) * s);
	s = (int)water_.size();
	r = (bool)ofs.write((char*)&s, sizeof(int));
	r = (bool)ofs.write((char*)water_.data(), sizeof(Tariff) * s);
	s = (int)inet_.size();
	r = (bool)ofs.write((char*)&s, sizeof(int));
	r = (bool)ofs.write((char*)inet_.data(), sizeof(Tariff) * s);
	s = (int)phone_.size();
	r = (bool)ofs.write((char*)&s, sizeof(int));
	r = (bool)ofs.write((char*)phone_.data(), sizeof(Tariff) * s);

	ofs.close();

	ExportCsv();

	return r;
}

bool DataBase::Load()
{
	ifstream ifs("months.cdb", ios::in | ios::binary);
	bool r; // Result

	if(!ifs.is_open())
	{
		return false;
	}

	int s = sizeof(Word) + sizeof(Readings) + sizeof(Deltas) + sizeof(Percents) + sizeof(Sums);

	char* buff = new char[s];
	while(ifs.read(buff, s))
	{
		vector<char> v;
		v.insert(v.end(), buff, buff+s);
		Month m;
		m.Deserialize(v);
		months_.PushBack(m);
	}

	delete[] buff;
	ifs.close();

	ifs.open("tariffs.cdb", ios::in | ios::binary);

	if(!ifs.is_open())
	{
		return false;
	}

	r = (bool)ifs.read((char*)&s, sizeof(int));
	buff = new char[s*sizeof(ElectroTariff)];
	ifs.read(buff, s*sizeof(ElectroTariff));
	electro_.insert(electro_.end(), (ElectroTariff*)buff, (ElectroTariff*)buff+s);
	delete[] buff;

	r = (bool)ifs.read((char*)&s, sizeof(int));
	buff = new char[s*sizeof(Tariff)];
	ifs.read(buff, s*sizeof(Tariff));
	gas_.insert(gas_.end(), (Tariff*)buff, (Tariff*)buff+s);
	delete[] buff;

	r = (bool)ifs.read((char*)&s, sizeof(int));
	buff = new char[s*sizeof(Tariff)];
	ifs.read(buff, s*sizeof(Tariff));
	water_.insert(water_.end(), (Tariff*)buff, (Tariff*)buff+s);
	delete[] buff;

	r = (bool)ifs.read((char*)&s, sizeof(int));
	buff = new char[s*sizeof(Tariff)];
	ifs.read(buff, s*sizeof(Tariff));
	inet_.insert(inet_.end(), (Tariff*)buff, (Tariff*)buff+s);
	delete[] buff;

	r = (bool)ifs.read((char*)&s, sizeof(int));
	buff = new char[s*sizeof(Tariff)];
	ifs.read(buff, s*sizeof(Tariff));
	phone_.insert(phone_.end(), (Tariff*)buff, (Tariff*)buff+s);
	delete[] buff;

	ifs.close();

	return r;
}

void DataBase::CheckAndSet(vector<ElectroTariff>& e, double e1, double e2)
{
	int s;
	if((s = (int)e.size()) <= 0)
	{
		goto sizezero;
	}
	if(e[s-1].less_min == e1 && e[s-1].more_min == e2)
	{
		return;
	}

sizezero:
	ElectroTariff et;
	et.less_min = e1;
	et.more_min = e2;
	et.update_time = (int)time(nullptr);

	e.push_back(et);
}

void DataBase::CheckAndSet(vector<Tariff>& t, double p)
{
	int s;
	if((s = (int)t.size()) <= 0)
	{
		goto sizezero;
	}
	if(t[s-1].price == p)
	{
		return;
	}

sizezero:
	Tariff tar;
	tar.price = p;
	tar.update_time = (int)time(nullptr);

	t.push_back(tar);
}

void DataBase::ExportCsv()
{
	ofstream ofs("Commune_Stats.csv", ios::out | ios::trunc);
	
	const char n = '\n';
	const char c = ',';
	ofs << "sep =," << n;

	// Tariffs
	ofs << "T1|T2,";
	for(int i = (int)electro_.size()-1; i >= 0; --i)
	{
		ofs << dToStrFull(electro_[i].less_min) << c << dToStrFull(electro_[i].more_min)
			<< c << dateToStr(electro_[i].update_time) << c;
	}
	ofs << n << "Gas,";
	for(int i = (int)gas_.size()-1; i >= 0; --i)
	{
		ofs << dToStrFull(gas_[i].price) << c << dateToStr(gas_[i].update_time) << c;
	}
	ofs << n << "Water,";
	for(int i = (int)water_.size()-1; i >= 0; --i)
	{
		ofs << dToStrFull(water_[i].price) << c << dateToStr(water_[i].update_time) << c;
	}
	ofs << n << "Phone,";
	for(int i = (int)phone_.size()-1; i >= 0; --i)
	{
		ofs << dToStrFull(phone_[i].price) << c << dateToStr(phone_[i].update_time) << c;
	}
	ofs << n << "Internet,";
	for(int i = (int)inet_.size()-1; i >= 0; --i)
	{
		ofs << dToStrFull(inet_[i].price) << c << dateToStr(inet_[i].update_time) << c;
	}
	ofs << n << ",,,,,,,,,,,,,," << n;

	// Months
	ofs << "Date,Electricity,,,,,Gas,,,Water,,,Phone,Internet,Total" << n;
	ofs << ",Sum,dkW*h,dkW*h,T1,T2,Sum,dm^3,m^3,Sum,dm^3,m^3,Sum,Sum,Sum" << n;

	for(Months::Index i = months_.Front(); i != months_.End(); ++i)
	{
		ofs << i->Csv().c_str();
	}

	ofs.close();
}
// DataBase implementation end

// Months implementation
// Index implementation

Months::Index& Months::Index::operator++()
{
	e_ = e_->next;
	return *this;
}

Months::Index& Months::Index::operator--()
{
	e_ = e_->prev;
	return *this;
}
// Index implementation end

Months::~Months()
{
	switch(size_)
	{
	case 0:
		break;
	
	case 1:
		delete first_;
		break;

	case 2:
		delete first_;
		delete last_;
		break;

	default:
	{
		Element* ep = first_->next;
		while(ep != last_)
		{
			delete ep->prev;
			ep->prev = nullptr;
			ep = ep->next;
		}

		delete last_;
		last_ = nullptr;
		break;
	}
	}
}

Month& Months::PushBack(const Month& m)
{
	switch(size_)
	{
	case 0:
		first_ = new Element(m);
		last_ = first_;
		++size_;
		break;

	case 1:
		last_ = new Element(m);
		last_->prev = first_;
		first_->next = last_;
		++size_;
		break;

	default:
		Element* tmp = last_;
		last_ = new Element(m);
		tmp->next = last_;
		last_->prev = tmp;
		++size_;
		break;
	}

	return last_->m;
}
// Months implementation end
