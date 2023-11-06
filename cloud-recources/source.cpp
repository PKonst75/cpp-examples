
#include<boost/beast.hpp>
#include<boost/asio.hpp>
#include<boost/asio/connect.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/asio/ssl.hpp>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/json_parser.hpp>
#include<iostream>
#include<string>
#include <map>
#include <vector>
#include <Windows.h>

//std::string GetResponseSSL(const std::string& , const std::string&);
//std::string PostResponseSSL(const std::string&, const std::string&, const std::string&);

const int DB_RAM_LOAD_AVG = 89;
const int DB_CPU_LOAD_AVG = 89;
const int VM_RAM_LOAD_AVG = 89;
const int VM_CPU_LOAD_AVG = 89;
const int UPGRADE_LOAD_LIMIT = 92;
const int DOWNGRADE_LOAD_LIMIT = 86;
const int RESPONCE_MAX_LIMIT = 350;

std::string PostResponseSSLCreateNewVm(const std::string&);
std::string GetStatistic();
std::string GetResources();
std::string DeleteResponseSSL(int);
std::string GetPrice();

using namespace std::literals;
namespace http = boost::beast::http;

void PrintMessage(std::string message) {
	std::cout << message << std::endl;
}

struct MashinesConfiguration {
	int cpu = 0;
	int ram = 0;
	bool operator > (const int value) {
		return ((cpu > 0) || (ram > 0));
	}
	bool operator > (const MashinesConfiguration& r) {
		return ((cpu > r.cpu) && (ram > r.ram));
	}
	bool operator == (const MashinesConfiguration& r) {
		return ((cpu == r.cpu) && (ram == r.ram));
	}
	void Print() {
		std::cout << "CPU = "s << cpu << " RAM = "s << ram << std::endl;
	}
};
struct CloudConfiguration {
	MashinesConfiguration vm;
	MashinesConfiguration db;
	void Print() {
		std::cout << "[VM] : "s;
		vm.Print();
		std::cout << "[DB] : "s;
		db.Print();
	}
};

MashinesConfiguration operator - (const MashinesConfiguration& l, const MashinesConfiguration& r) {
	return MashinesConfiguration{ l.cpu - r.cpu, l.ram - r.ram };
}

CloudConfiguration operator - (const CloudConfiguration& l, const CloudConfiguration& r) {
	return CloudConfiguration{ l.vm - r.vm, l.db - r.db };
}

enum class VmType {
	DB,
	VM,
	ERR
};

VmType StringToVmType(std::string type_string) {
	if (type_string == "vm") {
		return VmType::VM;
	}
	else if (type_string == "db") {
		return VmType::DB;
	}
	return VmType::ERR;
}

std::string VmTypeToString(VmType type) {
	switch (type) {
	case VmType::DB:
		return "db"s;
		break;
	case VmType::VM:
		return "vm"s;
		break;
	default:
		return "ERROR";
		break;
	}
}

boost::property_tree::ptree MakeJsonPTree(std::string json_data) {
	std::stringstream json_enc(json_data);
	boost::property_tree::ptree root;
	boost::property_tree::read_json(json_enc, root);
	return root;
}

template<typename Tdata>
bool GetFieldData(boost::property_tree::ptree root, std::string field, Tdata& data) {
	if (root.empty()) {
		return false;
	}
	data = root.get<Tdata>(field);
	return true;
}

template<typename Tdata>
bool GetFieldData(std::string json_data, std::string field, Tdata& data) {

	return GetFieldData(MakeJsonPTree(json_data), field, data);
}

template<typename Tdata>
void FillListFromJsonGroup(boost::property_tree::ptree root, std::vector<Tdata>& datas_vector) {
	if (root.empty()) {
		return;
	}
	for (auto entry : root) {
		Tdata data(entry.second);
		datas_vector.push_back(data);
	}
}

template<typename Tdata>
void FillListFromJsonGroup(std::string json_data, std::vector<Tdata>& datas_vector) {
	boost::property_tree::ptree root = MakeJsonPTree(json_data);
	FillListFromJsonGroup(root, datas_vector);
}

struct VmData {
	VmData(boost::property_tree::ptree root) {
		GetFieldData(root, "cost", cost);
		GetFieldData(root, "cpu", cpu);
		GetFieldData(root, "cpu_load", cpu_load);
		GetFieldData(root, "failed", failed);
		GetFieldData(root, "failed_until", failed_until);
		GetFieldData(root, "id", id);
		GetFieldData(root, "ram", ram);
		GetFieldData(root, "ram_load", ram_load);
		GetFieldData(root, "type", type);
	}
	VmData() = default;
	int cost = 0;
	int cpu = 0;
	double cpu_load = 0.0;
	bool failed = true;
	std::string failed_until;
	int id = 0;
	int ram = 0;
	double ram_load = 0.0;
	std::string type;
};


class Price {
public:
	Price() = default;
	Price(boost::property_tree::ptree root) {
		GetFieldData(root, "cost"s, cost_);
		GetFieldData(root, "cpu"s, cpu_);
		GetFieldData(root, "id"s, id_);
		GetFieldData(root, "name"s, name_);
		GetFieldData(root, "ram"s, ram_);
		GetFieldData(root, "type"s, type_);
		vm_type = StringToVmType(type_);
		config_ = MashinesConfiguration{ cpu_, ram_ };
	}
	VmType GetType() {
		return vm_type;
	}
	int GetId() {
		return id_;
	}
	int GetCpu() {
		return cpu_;
	}
	MashinesConfiguration GetConfiguration() {
		return config_;
	}
	void Print() {
		std::cout << "ID = "s << id_ << " CPU = "s << cpu_ << " RAM = "s << ram_ << std::endl;
	}
private:
	int cost_ = 0;
	int cpu_ = 0;
	int id_ = 0;
	std::string name_ = ""s;
	int ram_ = 0;
	std::string type_ = ""s;
	VmType vm_type = VmType::ERR;
	MashinesConfiguration config_;
};

class VirtualMashine {
public:
	VirtualMashine() = default;
	VirtualMashine(VmType type, int ram, int cpu)
		: type_(type), ram_(ram), cpu_(cpu), id_(0)
	{
		CreateVm();
		config_.cpu = cpu_;
		config_.ram = ram_;
	}
	VirtualMashine(VmData data) :
		type_(StringToVmType(data.type)), ram_(data.ram), cpu_(data.cpu), id_(data.id), vm_data_(data)
	{
		config_.cpu = cpu_;
		config_.ram = ram_;
	}
	void Print() {
		std::cout << "ID = "s << id_ << " TYPE="s << vm_data_.type << " CPU = "s << cpu_ << " RAM = "s << ram_ << std::endl;
	}
	int GetMaxCpuRam() {
		return std::max(cpu_, ram_);
	}
	VmType Type() {
		return type_;
	}
	int GetId() {
		return id_;
	}
	bool IsFailed() {
		return vm_data_.failed;
	}
	MashinesConfiguration GetConfig() {
		return config_;
	}
private:
	int id_ = 0;
	int ram_ = 0;
	int cpu_ = 0;
	VmType type_ = VmType::ERR;
	VmData vm_data_;
	MashinesConfiguration config_;

	std::string MakeJsonRequestCreate() {
		std::string result;
		result = "{ \"cpu\": %cpu, \"ram\" : %ram, \"type\" : \"%type\" }"s;
		result.replace(result.find("%cpu", 0), 4, std::to_string(cpu_));
		result.replace(result.find("%ram", 0), 4, std::to_string(ram_));
		std::string stype;
		switch (type_) {
		case VmType::DB:
			stype = "db";
			break;
		case VmType::VM:
			stype = "vm";
			break;
		default:
			break;
		}
		result.replace(result.find("%type", 0), 5, stype);

		return result;
	}
	bool CreateVm() {
		std::string res = PostResponseSSLCreateNewVm(MakeJsonRequestCreate());
		if (res.empty() || !GetFieldData(res, "id", id_)) {
			return false;
		}
		vm_data_.failed = true;
		return true;
	}
};


class PriceList {
public:
	PriceList() {
		type_ = VmType::ERR;
	}
	PriceList(std::vector<Price> prices, VmType type) {
		for (Price p : prices) {
			if (p.GetType() == type) {
				prices_.push_back(p);
			}
		}
		type_ = type;
	}
	std::vector<MashinesConfiguration> MakeUpgradeList(int cpu_uprade_count, int ram_upgrade_count) {
		std::vector<MashinesConfiguration> upgrate_data;
		while (ram_upgrade_count > 0 || cpu_uprade_count > 0) {
			Price p = FindBestPrice(cpu_uprade_count, ram_upgrade_count);
			MashinesConfiguration conf = p.GetConfiguration();
			upgrate_data.push_back(conf);
			ram_upgrade_count -= conf.ram;
			cpu_uprade_count -= conf.cpu;
			if (cpu_uprade_count < 0) {
				cpu_uprade_count = 0;
			}
			if (ram_upgrade_count < 0) {
				ram_upgrade_count = 0;
			}
		}
		return upgrate_data;
	}
	void Print() {
		for (Price p : prices_) {
			p.Print();
		}
	}
	std::map<int, std::vector<int>> MakeExchangeList(std::vector<VirtualMashine> vms) {
		// Разбираем по группам
		std::map<int, std::vector<int>> to_exchange;
		std::map<int, std::vector<VirtualMashine>> price_to_machines;
		for (Price p : prices_) {
			price_to_machines[p.GetId()] = std::vector<VirtualMashine>{};
			MashinesConfiguration conf = p.GetConfiguration();
			for (VirtualMashine vm : vms) {
				if (vm.GetConfig() == conf) {
					price_to_machines.at(p.GetId()).push_back(vm);
				}
			}
		}
		// Теперь создаем UpdateList
		// Пока полуручная сборка
		// Карта прайсов - меньшие к большим и количественный коэффициент
		std::map<int, std::pair<int, int>> update_map;
		for (Price pout : prices_) {
			for (Price pin : prices_) {
				if (pout.GetId() != pin.GetId() && pout.GetCpu() < pin.GetCpu()) {
					update_map[pout.GetId()] = std::pair<int, int>{ pin.GetId(),   pin.GetCpu() / pout.GetCpu() };
				}
			}
		}
		for (auto id_map : update_map) {
			int price_mashines_count = price_to_machines.at(id_map.first).size();
			if (price_mashines_count >= id_map.second.second) {
				// Можно сделать Update!!!
				to_exchange[id_map.second.first] = std::vector<int>{};
				for (VirtualMashine vm : price_to_machines.at(id_map.first)) {
					to_exchange.at(id_map.second.first).push_back(vm.GetId());
				}
				while ( (to_exchange.at(id_map.second.first).size() / id_map.second.second) != 0){
					to_exchange.at(id_map.second.first).pop_back();
				}
			}
		}
		return to_exchange;
	}
	Price GetById(int id) {
		for (Price p : prices_) {
			if (p.GetId() == id) {
				return p;
			}
		}
		return Price{};
	}
private:
	Price FindBestPrice(int cpu_uprade_count, int ram_upgrade_count) {
		int id_best = 0;
		int cpu_best = 0;
		int ram_best = 0;
		for (Price p : prices_) {
			MashinesConfiguration conf = p.GetConfiguration();
			if ((conf.cpu <= cpu_uprade_count) && (conf.ram <= ram_upgrade_count)) {
				if (conf.cpu > cpu_best && conf.ram > ram_best) {
					id_best = p.GetId();
					cpu_best = conf.cpu;
					ram_best = conf.ram;
				}
			}
		}
		return GetById(id_best);
	}
	VmType type_;
	std::vector<Price> prices_;
};


class PricesList {
public:
	PricesList(std::string json_data) {
		std::vector<Price> prices;
		FillListFromJsonGroup(json_data, prices);
		vm_prices_ = PriceList(prices, VmType::VM);
		db_prices_ = PriceList(prices, VmType::DB);
	}

	std::vector<MashinesConfiguration> MakeUpgradeList(int cpu_uprade_count, int ram_upgrade_count, VmType type) {
		switch (type) {
		case VmType::VM:
			return vm_prices_.MakeUpgradeList(cpu_uprade_count, ram_upgrade_count);
			break;
		case VmType::DB:
			return db_prices_.MakeUpgradeList(cpu_uprade_count, ram_upgrade_count);
			break;
		default:
			return std::vector<MashinesConfiguration>{};
			break;
		}
	}

	std::map<int, std::vector<int>> MakeExchangeList(std::vector<VirtualMashine> vms, VmType type) {
		switch (type) {
		case VmType::VM:
			return vm_prices_.MakeExchangeList(vms);
			break;
		case VmType::DB:
			return db_prices_.MakeExchangeList(vms);
			break;
		default:
			return std::map<int, std::vector<int>> {};
			break;
		}
	}
	void Print() {
		std::cout << "[DB]"s << std::endl;
		db_prices_.Print();
		std::cout << "[VM]"s << std::endl;
		vm_prices_.Print();
	}

	Price GetById(int id) {
		Price p = db_prices_.GetById(id);
		if (p.GetId() != 0) {
			return p;
		}
		return vm_prices_.GetById(id);
	}
private:
	PriceList db_prices_;
	PriceList vm_prices_;
};

struct Statistic {
public :
	Statistic(const std::string&json_data) {
		boost::property_tree::ptree root = MakeJsonPTree(json_data);
		GetFieldData(root, "db_cpu"s, db_cpu);
		GetFieldData(root, "db_ram"s, db_ram);
		GetFieldData(root, "db_cpu_load"s, db_cpu_load);
		GetFieldData(root, "db_ram_load"s, db_ram_load);
		GetFieldData(root, "vm_cpu"s, vm_cpu);
		GetFieldData(root, "vm_ram"s, vm_ram);
		GetFieldData(root, "vm_cpu_load"s, vm_cpu_load);
		GetFieldData(root, "vm_ram_load"s, vm_ram_load);
		GetFieldData(root, "response_time"s, response_time);
		GetFieldData(root, "online"s, online);
		GetFieldData(root, "requests"s, requests);
	}
	int ComputeRamCpu(VmType type) {
		switch (type) {
		case VmType::DB:
			return ComputeDbRamCpu();
			break;
		case VmType::VM:
			return ComputeVmRamCpu();
			break;
		default:
			return 0;
			break;
		}
	}
	int ComputeDbRamCpu() const {
		//int ram = static_cast<int>(db_ram_load / db_ram / 10.0 * db_ram * 100.0 / static_cast<double>(DB_RAM_LOAD_AVG));
		//int cpu = static_cast<int>(db_cpu_load / db_cpu / 10.0 * db_cpu * 100.0 / static_cast<double>(DB_CPU_LOAD_AVG));
		int ram = static_cast<int>(db_ram_load  / static_cast<double>(DB_RAM_LOAD_AVG)) + 1;
		int cpu = static_cast<int>(db_cpu_load / static_cast<double>(DB_CPU_LOAD_AVG)) + 1;
		return std::max(ram, cpu);
	}
	int ComputeVmRamCpu() const{
		//int ram = static_cast<int>(vm_ram_load / vm_ram / 10.0 * vm_ram * 100.0 / static_cast<double>(VM_RAM_LOAD_AVG));
		//int cpu = static_cast<int>(vm_cpu_load / vm_cpu / 10.0 * vm_cpu * 100.0 / static_cast<double>(VM_CPU_LOAD_AVG));
		int ram = static_cast<int>(vm_ram_load  / static_cast<double>(VM_RAM_LOAD_AVG)) + 1;
		int cpu = static_cast<int>(vm_cpu_load  / static_cast<double>(VM_CPU_LOAD_AVG)) + 1;
		return std::max(ram, cpu);
	}

	/*Configuration ComputeConfiguration() {
		Configuration config;
		if (vm_cpu == 0 || db_cpu == 0) {
			ComputeEsimateLoad();
		}
		config.vm_cpu = static_cast<int>(vm_cpu_load * vm_cpu / static_cast<double>(VM_CPU_LOAD_AVG)) + 1;
		config.vm_ram = static_cast<int>(vm_ram_load * vm_ram / static_cast<double>(VM_RAM_LOAD_AVG)) + 1;
		config.db_ram = static_cast<int>(db_ram_load * db_ram / static_cast<double>(DB_RAM_LOAD_AVG)) + 1;
		config.db_cpu = db_ram;
	}
	void ComputeEsimateLoad() {
		vm_cpu_load = (static_cast<double>(requests) * 0.001 + 0.05) * 100.0;
		vm_ram_load = (static_cast<double>(requests) * 0,005 + 3) * 100.0;
		db_ram_load = (static_cast<double>(requests) * 0,001 + 2) * 100.0;
		db_cpu_load = vm_cpu_load;
	}*/

	//bool DoUpdate() {
	//	if ((db_ram_load > UPGRADE_LOAD_LIMIT) || (db_ram_load < DOWNGRADE_LOAD_LIMIT)) return true;
	//	if ((vm_ram_load > UPGRADE_LOAD_LIMIT) || (vm_ram_load < DOWNGRADE_LOAD_LIMIT)) return true;
	//	if ((vm_cpu_load > UPGRADE_LOAD_LIMIT) || (vm_cpu_load < DOWNGRADE_LOAD_LIMIT)) return true;
	//	return false;
	//}

	Statistic() = default;
	void Print() {
		std::cout << "DB CPU = "s << db_cpu << " CPU LOAD = "s << db_cpu_load << " RAM = "s << db_ram << " RAM LOAD = "s << db_ram_load << " NEDED UNIT = " << ComputeDbRamCpu() << std::endl;
		std::cout << "VM CPU = "s << vm_cpu << " CPU LOAD = "s << vm_cpu_load << " RAM = "s << vm_ram << " RAM LOAD = "s << vm_ram_load << " NEDED UNIT = " << ComputeVmRamCpu() << std::endl;
		std::cout << "ONLINE = " << online << " REQUESTS = " << requests << " RESPONCE = " << response_time << std::endl;
	}
	int db_cpu = 0;
	double db_cpu_load = 0.0;
	int db_ram = 0;
	double db_ram_load = 0.0;
	int vm_cpu = 0;
	double vm_cpu_load = 0.0;
	int vm_ram = 0;
	double vm_ram_load = 0.0;
	double response_time = 0.0;
	bool online = false;
	int requests = 0;
};


Statistic MakeStatistic(std::string json_data) {
	std::stringstream json_enc(json_data);
	boost::property_tree::ptree root;
	boost::property_tree::read_json(json_enc, root);

	if (root.empty()) {
		return Statistic();
	}
	return Statistic(json_data);//Statistic(root);
}

class StatAnalizator {
public:
	StatAnalizator(Statistic stat) {
		UpdateStat(stat);
	}
	void UpdateStat(Statistic stat) {
		stat_ = stat;
		current_config_.db.cpu = stat.db_cpu;
		current_config_.db.ram = stat.db_ram;
		current_config_.vm.cpu = stat.vm_cpu;
		current_config_.vm.ram = stat.vm_ram;
	}
	bool UpdateDecision() {
		if (!stat_.online) {
			return true;
		}
		if (stat_.response_time > RESPONCE_MAX_LIMIT) {
			return true;
		}
		if ((stat_.db_ram_load > UPGRADE_LOAD_LIMIT) || (stat_.db_ram_load < DOWNGRADE_LOAD_LIMIT)) {
			return true;
		}
		if ((stat_.vm_ram_load > UPGRADE_LOAD_LIMIT) || (stat_.vm_cpu_load > UPGRADE_LOAD_LIMIT)) {
			return true;
		}
		if ((stat_.vm_ram_load < DOWNGRADE_LOAD_LIMIT) && (stat_.vm_cpu_load < DOWNGRADE_LOAD_LIMIT)) {
			return true;
		}
		return false;
	}

	CloudConfiguration UpdateData() {
		CloudConfiguration update_data;
		CloudConfiguration goal = ComputeConfiguration();
		PrintMessage("GOAL"s);
		goal.Print();
		CloudConfiguration difference = goal - current_config_;
		// Используем упрощенный алгоритм
		update_data.vm = SimpleUpdateAlgorithm(difference.vm);
		update_data.db = SimpleUpdateAlgorithm(difference.db);
		PrintMessage("CALC"s);
		update_data.Print();
		return update_data;
	}
	
	void Print() {
		stat_.Print();
	}
private:
	CloudConfiguration ComputeConfiguration() {
		CloudConfiguration config;
		if (stat_.vm_cpu == 0 || stat_.db_cpu == 0) {
			ComputeEsimateLoad();
		}
		config.vm.cpu = static_cast<int>(stat_.vm_cpu_load * stat_.vm_cpu / static_cast<double>(VM_CPU_LOAD_AVG)) + 1;
		config.vm.ram = static_cast<int>(stat_.vm_ram_load * stat_.vm_ram / static_cast<double>(VM_RAM_LOAD_AVG)) + 1;
		config.vm.cpu = std::max(config.vm.cpu, config.vm.ram);
		config.vm.ram = config.vm.cpu;
		config.db.ram = static_cast<int>(stat_.db_ram_load * stat_.db_ram / static_cast<double>(DB_RAM_LOAD_AVG)) + 1;
		config.db.cpu = config.db.ram;
		return config;
	}
	MashinesConfiguration SimpleUpdateAlgorithm(MashinesConfiguration update_data) {
		MashinesConfiguration res;
		// В нашем случае используем упрощенный алгоритм
		if (update_data > 0) {
			// Увеличиывем на максимум из двух параметров
			int up_count = 0;
			if (update_data.cpu > 0) {
				up_count = update_data.cpu;
			}
			if (update_data.ram > 0) {
				up_count = std::max(up_count, update_data.ram);
			}
			res.cpu = up_count;
			res.ram = up_count;
		}
		else {
			// Уменьшаем на минимум из двух параметров
			if (std::abs(update_data.cpu) > std::abs(update_data.ram)) {
				res.cpu = update_data.ram;
				res.ram = update_data.ram;
			}
			else {
				res.cpu = update_data.cpu;
				res.ram = update_data.cpu;
			}
		}
		return res;
	}
	void ComputeEsimateLoad() {
		stat_.vm_cpu_load = (static_cast<double>(stat_.requests) * 0.001 + 0.05) * 100.0;
		stat_.vm_ram_load = (static_cast<double>(stat_.requests) * 0, 005 + 3) * 100.0;
		stat_.db_ram_load = (static_cast<double>(stat_.requests) * 0, 001 + 2) * 100.0;
		stat_.db_cpu_load = stat_.vm_cpu_load;
	}
	Statistic stat_;
	CloudConfiguration current_config_;
};





std::vector<VirtualMashine> GetVmList(std::string json_data) {
	std::stringstream json_enc(json_data);
	boost::property_tree::ptree root;
	boost::property_tree::read_json(json_enc, root);
	if (root.empty()) {
		return std::vector<VirtualMashine> {};
	}

	std::vector<VirtualMashine> vms;
	for (auto entry : root) {
		VmData vm_data(entry.second);
		vms.push_back(VirtualMashine(vm_data));
	}
	return vms;
}

class VmList {
public:

	static void Clear(std::vector<VirtualMashine> list) {
		// Очистка списка
		for (VirtualMashine vm : list) {
			DeleteResponseSSL(vm.GetId());
		}
		PrintMessage("Delete Done"s);
	}

	static bool HasFailed(std::vector<VirtualMashine> list, VmType type) {
		for (VirtualMashine vm : list) {
			if (!(vm.Type() == type)) {
				continue;
			}
			if (vm.IsFailed()) {
				return true;
			}
		}
		return false;
	}

	static int Unit1Count(std::vector<VirtualMashine> list, VmType type) {
		int count = 0;
		for (VirtualMashine vm : list) {
			if (!(vm.Type() == type)) {
				continue;
			}
			if (vm.GetMaxCpuRam() == 1) {
				count++;
			}
		}
		return count;
	}

	static int Get10Id(std::vector<VirtualMashine> list, VmType type) {
		for (VirtualMashine vm : list) {
			if (!(vm.Type() == type)) {
				continue;
			}
			if (vm.GetMaxCpuRam() == 10) {
				return vm.GetId();
			}
		}
		return 0;
	}

	static int GetUnits(std::vector<VirtualMashine> list, VmType type) {
		int units = 0;
		for (VirtualMashine vm : list) {
			if (vm.Type() == VmType::DB) {
				units += vm.GetMaxCpuRam();
			}
		}
		return units;
	}


	void static RemoveVm(std::vector<VirtualMashine> list, VmType type, int to_del) {

		int to_del_10 = to_del / 10;
		int to_del_1 = to_del % 10;
		if (to_del_10 > 0) {
			for (VirtualMashine vm : list) {
				if ( (vm.Type() == type) && vm.GetMaxCpuRam() == 10) {
					std::cout << DeleteResponseSSL(vm.GetId()) << std::endl;
					--to_del_10;
					if (to_del_10 == 0) {
						break;
					}
				}
			}
		}

		to_del_1 = to_del_1 + to_del_10 * 10;
		if (to_del_1 > 0) {
			for (VirtualMashine vm : list) {
				if ( (vm.Type() == type) && vm.GetMaxCpuRam() == 1) {
					std::cout << DeleteResponseSSL(vm.GetId()) << std::endl;
					--to_del_1;
					if (to_del_1 == 0) {
						break;
					}
				}
			}
		}
	}
};


int RebalanceVmMachines(const std::vector<VirtualMashine>& vms, const Statistic& stat) {
	if (stat.ComputeVmRamCpu() < 8 && !VmList::HasFailed(vms, VmType::VM) && VmList::Unit1Count(vms, VmType::VM) == 0) {
		for (int i = 1; i <= stat.ComputeVmRamCpu(); ++i) {
			VirtualMashine(VmType::VM, 1, 1);
		}
		return VmList::Get10Id(vms, VmType::VM);
	}
	return 0;
}

int AddVmMachines(int to_add) {
		int to_add_10 = to_add / 10;
		int to_add_1 = to_add - to_add_10 * 10;
		for (int i = 1; i <= to_add_10; ++i) {
			VirtualMashine(VmType::VM, 10, 10);
		}
		for (int i = 1; i <= to_add_1; ++i) {
			VirtualMashine(VmType::VM, 1, 1);
		}
		return 0;
}


void OperateVmMachines(const std::vector<VirtualMashine>& vms, const Statistic& stat) {

	int vm_unit = VmList::GetUnits(vms, VmType::VM);
	// 1 Определяем требуемое количество ресурсов

	// 2 Проверяем стоит ли лезть

	// 3 Удаляем

	// 4 Добавляем
	if (!VmList::HasFailed(vms, VmType::VM) && stat.ComputeVmRamCpu() > vm_unit * 1.2) {
		PrintMessage("VM added Needed");
		AddVmMachines(stat.ComputeVmRamCpu() - vm_unit);
	}
}

class VirtualMachinesPool {
public:
	VirtualMachinesPool(std::string json_data){
		UpdateList(json_data);
	}
	void UpdateServer(CloudConfiguration config, PricesList lists) {
		// Делаем обнолвение виртуальных машин по заданному параметру
		if (config.vm > 0) {
			PrintMessage("Adding VM"s);
			config.Print();
			std::vector<MashinesConfiguration> vms = lists.MakeUpgradeList(config.vm.cpu, config.vm.ram, VmType::VM);
			for (MashinesConfiguration conf:vms) {
				PrintMessage("UNIT:");
				conf.Print();
				VirtualMashine(VmType::VM, conf.cpu, conf.ram);
			}
			PrintMessage("Adding OK"s);
		}
		else {
			// Удаляем виртуальные машины
			MashinesConfiguration conf{ std::abs(config.vm.cpu), std::abs(config.vm.ram) };
			PrintMessage("Remove VM"s);
			conf.Print();
			std::vector<int> remove = DeleteList(conf, vms_, vm_balance_);
			DeleteServer(remove);
			PrintMessage("Remove OK"s);
		}
		if (config.db > 0) {
			PrintMessage("Adding DB"s);
			config.Print();
			std::vector<MashinesConfiguration> dbs = lists.MakeUpgradeList(config.db.cpu, config.db.ram, VmType::DB);
			for (MashinesConfiguration conf : dbs) {
				PrintMessage("UNIT:");
				conf.Print();
				VirtualMashine(VmType::DB, conf.cpu, conf.ram);
			}
			PrintMessage("Adding OK"s);
		}
		else {
			// Удалякм виртуальные машины
			MashinesConfiguration conf{ std::abs(config.db.cpu), std::abs(config.db.ram) };
			PrintMessage("Remove DB"s);
			conf.Print();
			std::vector<int> remove = DeleteList(conf, dbs_, db_balance_);
			DeleteServer(remove);
			PrintMessage("Remove OK"s);
		}
	}

	void Print() {
		std::cout << "VM"s << std::endl;
		for (VirtualMashine vm : vms_) {
			vm.Print();
		}
		if (vm_balance_) {
			std::cout << "VM REBALANCE NEEDED!"s << std::endl;
		}
		std::cout << "DB"s << std::endl;
		for (VirtualMashine vm : dbs_) {
			vm.Print();
		}
		if (db_balance_) {
			std::cout << "DB REBALANCE NEEDED!"s << std::endl;
		}
	}
	void UpdateList(std::string json_data) {
		vms_.clear();
		dbs_.clear();
		if (json_data.empty()) {
			// Защита от пустого списка
			return;
		}
		std::stringstream json_enc(json_data);
		boost::property_tree::ptree root;
		boost::property_tree::read_json(json_enc, root);
		if (root.empty()) {
			return;
		}
		for (auto entry : root) {
			VmData vm_data(entry.second);
			if (vm_data.type == "db"s) {
				dbs_.push_back(VirtualMashine(vm_data));
			}
			else if (vm_data.type == "vm"s) {
				vms_.push_back(VirtualMashine(vm_data));
			}
		}
	}

	bool HasFailed() {
		for (VirtualMashine vm : vms_) {
			if (vm.IsFailed()) {
				return true;
			}
		}
		for (VirtualMashine vm : dbs_) {
			if (vm.IsFailed()) {
				return true;
			}
		}
		return false;
	}

	void GroupMashines(PricesList prices) {
		// Пробуем сгруппировать машины 
		std::map<int, std::vector<int>> exchange_vms = prices.MakeExchangeList(vms_, VmType::VM);
		std::map<int, std::vector<int>> exchange_dbs = prices.MakeExchangeList(dbs_, VmType::DB);
		for (auto  data : exchange_vms) {
			PrintMessage("GROUP VM MACHINES!!");
			Price p = prices.GetById(data.first);
			MashinesConfiguration conf = p.GetConfiguration();
			int machines_count = data.second.size() / conf.cpu;
			for (int i = 1; i <= machines_count; ++i) {
				PrintMessage("ADD VM MACHINE!!");
				VirtualMashine(VmType::VM, conf.cpu, conf.ram);
			}
			for (int i : data.second) {
				exchange_remove_list_.push_back(i);
			}
		}
		for (auto data : exchange_dbs) {
			PrintMessage("GROUP DB MACHINES!!");
			Price p = prices.GetById(data.first);
			MashinesConfiguration conf = p.GetConfiguration();
			int machines_count = data.second.size() / conf.cpu;
			for (int i = 1; i <= machines_count; ++i) {
				PrintMessage("ADD DB MACHINE!!");
				VirtualMashine(VmType::DB, conf.cpu, conf.ram);
			}
			for (int i : data.second) {
				exchange_remove_list_.push_back(i);
			}
		}
	}
	void ClearExchangeList() {
		if (exchange_remove_list_.size() > 0) {
			DeleteServer(exchange_remove_list_);
		}
	}
private:
	std::vector<int> exchange_remove_list_;
	void DeleteServer(std::vector<int> to_del_ids) {
		for (int id : to_del_ids) {
			DeleteResponseSSL(id);
		}
	}
	std::vector<int> DeleteList(MashinesConfiguration conf, std::vector<VirtualMashine> list, bool& rabalance) {
		std::vector<int> remove;
		VirtualMashine vm = FindLimitedMax(list, conf.cpu, conf.ram);
		while ((vm.Type() != VmType::ERR) && (conf.cpu > 0 && conf.ram > 0)) {
			remove.push_back(vm.GetId());
			conf.cpu = conf.cpu - vm.GetConfig().cpu;
			conf.ram = conf.ram - vm.GetConfig().ram;
			RemoveFromListById(list, vm.GetId());
			vm = FindLimitedMax(list, conf.cpu, conf.ram);
		}
		if (conf.cpu > 0 || conf.ram > 0) {
			rabalance = true;
		}
		return remove;
	}
	void RemoveFromListById(std::vector<VirtualMashine>& list, int id) {
		int find = -1;
		for (int i = 0; i < list.size() && (find < 0); ++i) {
			if (list.at(i).GetId() == id) {
				find = i;
			}
		}
		// Удаляем элемент из массива
		if (find >= 0 && find < list.size()) {
			list.at(find) = std::move(list.back());
			list.pop_back();
		}
	}
	VirtualMashine FindLimitedMax(std::vector<VirtualMashine> list, int cpu_limit, int ram_limit) {
		int current_max_cpu = 0;;
		int current_max_ram = 0;
		int current_max_id = 0;
		VirtualMashine current_max;
		for (VirtualMashine vm : list) {
			MashinesConfiguration conf = vm.GetConfig();
			if ((conf.cpu <= cpu_limit) && (conf.ram <= ram_limit)) {
				if ( (conf.cpu >= current_max_cpu)  && (conf.ram >= current_max_ram) ) {
					current_max_id = vm.GetId();
					current_max_cpu = conf.cpu;
					current_max_ram = conf.ram;
					current_max = vm;
				}
			}
		}
		return current_max;
	}
	bool vm_balance_ = false;
	bool db_balance_ = false;
	std::vector<VirtualMashine> vms_;
	std::vector<VirtualMashine> dbs_;
};

int main() {
	
	std::cout << "START!s" << std::endl;

	// Получаем прайс лист
	PricesList price_list(GetPrice());
	price_list.Print();

	// Получаем список виртуальных машин
	// Ждем пока есть незапущенные машины
	VirtualMachinesPool pool(GetResources());
	pool.Print();
	while (pool.HasFailed()) {
		PrintMessage("WAIT FOR UPDATE....."s);
		Sleep(30000);
		pool.UpdateList(GetResources());
	}
	// Сжимаем список
	pool.GroupMashines(price_list);
	pool.UpdateList(GetResources());
	pool.Print();
	while (pool.HasFailed()) {
		PrintMessage("WAIT FOR UPDATE....."s);
		Sleep(30000);
		pool.UpdateList(GetResources());
	}
	pool.ClearExchangeList();

	// Создаем первичную статистику
	StatAnalizator analizator{Statistic( GetStatistic()) };
	analizator.Print();

	while (true) {

		PrintMessage("Analize status...");
		if (analizator.UpdateDecision()) {
			// Управляем виртуальными машинами!
			PrintMessage("Update Stage");
			analizator.Print();
			CloudConfiguration to_update = analizator.UpdateData();
			to_update.Print();
			pool.UpdateServer(to_update, price_list);
			pool.UpdateList(GetResources());

			while (pool.HasFailed()) {
				PrintMessage("WAIT FOR UPDATE......"s);
				Sleep(30000);
				pool.UpdateList(GetResources());
			}
			PrintMessage("UpdateSucessful");
		}
		Sleep(60000);
		analizator.UpdateStat(Statistic(GetStatistic()));
		analizator.Print();
	}

	return 0;


	

	if (false) {
	//	std::string test = PostResponseSSL(REQUEST_NEWVM, TOKEN, REQUEST_DBVM_JSON);
	}

	//std::cout << GetStatistic() << std::endl;
	//std::cout << GetResources() << std::endl;

	//GetRoot(GetResources(), "");

	bool rebalance_vm_flag = true;
	int rebalance_vm_id = 0;
	bool rebalance_db_flag = true;
	int rebalance_db_id = 0;

	while (true) {

		Statistic stat = MakeStatistic(GetStatistic());
		stat.Print();

		std::vector<VirtualMashine> vm_list = GetVmList(GetResources());
		std::vector<VirtualMashine> vms_list;
		std::vector<VirtualMashine> dbs_list;
		int db_unit = 0;
		int vm_unit = 0;
		for (VirtualMashine vm : vm_list) {
			vm.Print();
			if (vm.Type() == VmType::DB) {
				dbs_list.push_back(vm);
				db_unit += vm.GetMaxCpuRam();
			}
			else if (vm.Type() == VmType::VM) {
				vms_list.push_back(vm);
				vm_unit += vm.GetMaxCpuRam();
			}
		}


		
		if (rebalance_vm_flag && !VmList::HasFailed(vm_list, VmType::VM)) {
			DeleteResponseSSL(rebalance_vm_id);
			rebalance_vm_flag = false;
			rebalance_vm_id = 0;
		}

		if (rebalance_db_flag && !VmList::HasFailed(vm_list, VmType::DB)) {
			DeleteResponseSSL(rebalance_db_id);
			rebalance_db_flag = false;
			rebalance_db_id = 0;
		}

		// Перебалансируем
		if (stat.ComputeVmRamCpu() < 8 && !VmList::HasFailed(vm_list, VmType::VM) && VmList::Unit1Count(vm_list, VmType::VM) == 0) {
			rebalance_vm_id = VmList::Get10Id(vm_list, VmType::VM);
			for (int i = 1; i <= stat.ComputeVmRamCpu(); ++i) {
				VirtualMashine(VmType::VM, 1, 1);
			}
			rebalance_vm_flag = true;
		}

		if (stat.ComputeDbRamCpu() < 8 && !VmList::HasFailed(vm_list, VmType::DB) && VmList::Unit1Count(vm_list, VmType::DB) == 0) {
			rebalance_db_id = VmList::Get10Id(vm_list, VmType::DB);
			for (int i = 1; i <= stat.ComputeDbRamCpu(); ++i) {
				VirtualMashine(VmType::DB, 1, 1);
			}
			rebalance_db_flag = true;
		}

		PrintMessage("AddStage");
		if (!VmList::HasFailed(vm_list, VmType::DB) && stat.ComputeDbRamCpu() > db_unit * 1.2) {
			int to_add = stat.ComputeDbRamCpu() - db_unit;
			int to_add_10 = to_add / 10;
			int to_add_1 = to_add - to_add_10 * 10;
			for (int i = 1; i <= to_add_10; ++i) {
				VirtualMashine(VmType::DB, 10, 10);
			}
			for (int i = 1; i <= to_add_1; ++i) {
				VirtualMashine(VmType::DB, 1, 1);
			}
		}

		if (!VmList::HasFailed(vm_list, VmType::VM) && stat.ComputeVmRamCpu() > vm_unit * 1.2) {
			PrintMessage("VM added Needed");
			int to_add = stat.ComputeVmRamCpu() - vm_unit;
			int to_add_10 = to_add / 10;
			int to_add_1 = to_add - to_add_10 * 10;
			for (int i = 1; i <= to_add_10; ++i) {
				VirtualMashine(VmType::VM, 10, 10);
			}
			for (int i = 1; i <= to_add_1; ++i) {
				VirtualMashine(VmType::VM, 1, 1);
			}
		}

		PrintMessage("Remove Stage");
		// Удаляем объекты только если нет спящих
		if(!VmList::HasFailed(vm_list, VmType::VM) && !VmList::HasFailed(vm_list, VmType::DB) ){
			bool remove_flag = false;
			if ((!rebalance_vm_flag) && (vm_unit > 1) && (stat.ComputeVmRamCpu() < vm_unit / 1.2)) {
				int to_del = vm_unit - stat.ComputeVmRamCpu();
				if (to_del > vm_unit * 0.8) to_del = static_cast <int>(vm_unit * 0.8);
				if (to_del == 0) to_del = 1;
				VmList::RemoveVm(vm_list, VmType::VM, to_del);
				remove_flag = true;
			}
			if ((!rebalance_vm_flag) && (db_unit > 1) && (stat.ComputeDbRamCpu() < db_unit / 1.2)) {
				int to_del = db_unit - stat.ComputeDbRamCpu();
				if (to_del == 0) to_del = 1;
				if (to_del > db_unit * 0.8) to_del = static_cast <int>(db_unit * 0.8);
				VmList::RemoveVm(vm_list, VmType::DB, to_del);
				remove_flag = true;
			}
			if (remove_flag) {
				Sleep(60000);
				continue;
			}
		}

		/*if (stat.ComputeDbRamCpu() < db_unit / 1.2) {
			int to_del = stat.ComputeDbRamCpu() - db_unit;
			int to_del_10 = to_del / 10;
			int to_del_1 = to_del - to_del_10 * 10;
			bool flag_10 = false;
			if (to_del_10 > 0) {
				flag_10 = true;
				for (VirtualMashine vm : vm_list) {
					if (vm.Type() == VmType::DB && vm.GetMaxCpuRam() == 10) {
						DeleteResponseSSL(vm.GetId());
						flag_10 = false;
						break;
					}
				}
			}
			if (flag_10) {
				to_del_1 = 10;
			}
			if (to_del_1 > 0) {
				for (VirtualMashine vm : vm_list) {
					if (vm.Type() == VmType::DB && vm.GetMaxCpuRam() == 1) {
						DeleteResponseSSL(vm.GetId());
						break;
					}
				}
			}
		}

		if (stat.ComputeVmRamCpu() < vm_unit / 1.2) {
			int to_del = stat.ComputeVmRamCpu() - vm_unit;
			int to_del_10 = to_del / 10;
			int to_del_1 = to_del - to_del_10 * 10;
			bool flag_10 = false;
			if (to_del_10 > 0) {
				flag_10 = true;
				for (VirtualMashine vm : vm_list) {
					if (vm.Type() == VmType::VM && vm.GetMaxCpuRam() == 10) {
						DeleteResponseSSL(vm.GetId());
						flag_10 = false;
						break;
					}
				}
			}
			if (flag_10) {
				to_del_1 = 10;
			}
			if (to_del_1 > 0) {
				for (VirtualMashine vm : vm_list) {
					if (vm.Type() == VmType::VM && vm.GetMaxCpuRam() == 1) {
						DeleteResponseSSL(vm.GetId());
						break;
					}
				}
			}
		}*/

		Sleep(60000);

	}
		
	//std::cout << DeleteResponseSSL(153295) << std::endl;


}