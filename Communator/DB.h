#ifndef DB_H
#define DB_H

#include <Windows.h>

#include "Month.h"

struct MinMax
{
	double min;
	double max;
};

struct SumArrays
{
	int n;
	bool pre_m;
	double ele[13];
	double gas[13];
	double wat[13];
};

class Months
{
	struct Element
	{
		Element* prev;
		Element* next;
		Month m;

		Element() : prev(nullptr), next(nullptr) {}
		Element(const Month& mon) : m(mon), prev(nullptr), next(nullptr) {}
	};

public:
	class Index
	{
	public:
		Index() : e_(nullptr) {}
		Index(Element* e) : e_(e) {}

		Index& operator++();
		Index& operator--();
		bool operator==(const Index& other) const { return e_ == other.e_; }
		bool operator!=(const Index& other) const { return e_ != other.e_; }
		Month& operator*() { return e_->m; }
		Month* operator->() { return &e_->m; }
		const Month& operator*() const { return e_->m; }

	private:
		Element* e_;
	};

	Months() : size_(0), first_(nullptr), last_(nullptr) {}
	~Months();

	Month& PushBack(const Month& m);
	Month& Last() { return last_->m; }
	const Month& Last() const { return last_->m; }
	Month& PreLast() { return last_->prev->m; }
	const Month& PreLast() const { return last_->prev->m; }
	Month& First() { return first_->m; }
	const Month& First() const { return first_->m; }
	Index Front() const { return Index(first_); }
	Index Back() const { return Index(last_); }
	Index End() const { return Index(); }
	int Size() const { return size_; }

	// To add later, maybe
	// Month& operator[](int i);
	// const Month& operator[](int i) const;
	// PushFront()

private:
	Element* first_;
	Element* last_;
	int size_;
};

class DataBase
{
public:
	DataBase() = default;

	void AddMonth(const Month& m) { months_.PushBack(m); }

	void SetTariffs(double e1, double e2, double et, double g, double gt, double w, double wt, double i, double p);

	Readings GetPastReads() const { return months_.Size() > 0 ? months_.Last().GetReadings() : Readings(); }
	Readings GetPastPastReads() const { return months_.Size() > 1 ? months_.PreLast().GetReadings() : Readings(); }
	Deltas GetPastDeltas() const { return months_.Size() > 0 ? months_.Last().GetDeltas() : Deltas(); }
	Deltas GetPastPastDeltas() const { return months_.Size() > 1 ? months_.PreLast().GetDeltas() : Deltas(); }
	Month& GetLastMonth();
	const Month& GetLastMonth() const;

	// Tariff price getters
	ElectroTariff TariffElectro() const { return electro_.size() > 0 ? electro_.back() : ElectroTariff(); }
	double TariffElectroTrans() const { return electro_.size() > 0 ? electro_.back().trans_price : 0.0; }
	double TariffGas() const { return gas_.size() > 0 ? gas_.back().price : 0.0; }
	double TariffGasTrans() const { return gas_.size() > 0 ? gas_.back().trans_price : 0.0; }
	double TariffWater() const { return water_.size() > 0 ? water_.back().price : 0.0; }
	double TariffWaterTrans() const { return water_.size() > 0 ? water_.back().trans_price : 0.0; }
	double TariffInet() const { return inet_.size() > 0 ? inet_.back().price : 0.0; }
	double TariffBankfee() const { return bankfee_.size() > 0 ? bankfee_.back().price : 0.0; }

	// Graph utilities
	MinMax GetMinMax() const;
	const SumArrays& GetSumArrays() const;

	bool Save();
	bool Load();

	int MonthsCount() { return months_.Size(); }

private:
	void CheckAndSet(vector<ElectroTariff>& e, double e1, double e2, double et);
	void CheckAndSet(vector<Tariff>& t, double p, double tp);
	void CheckAndSet(vector<SimpleTariff>& t, double p);
	void ExportCsv();

	// Data
	Months months_;
	// Tariffs
	vector<ElectroTariff> electro_;
	vector<Tariff> gas_;
	vector<Tariff> water_;
	vector<SimpleTariff> inet_;
	vector<SimpleTariff> bankfee_;
};

#endif /* DB_H */