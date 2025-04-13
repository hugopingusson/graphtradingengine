//
// Created by hugo on 13/04/25.
//

#include "DataBaseHelper.h"
#include "FutureHelper.h"


DataBaseHelper::DataBaseHelper():database_root("/media/hugo/T7/market_data/databento/mbp10_bin") {}


string DataBaseHelper::get_data_path(const string &date, const string &instrument, const string &exchange) {
    if (exchange=="cme") {
        return this->get_future_data_path(date,instrument);
    }


    throw std::runtime_error(fmt::format("DataBaseHelper::get_data_path: exchange={} not supported",exchange));

}


string DataBaseHelper::get_future_data_path(const string& date,const string& instrument) {
    FutureHelper future_helper=FutureHelper(instrument);
    string liquid_contract = future_helper.get_liquid_contract(date);

    path base_dir(this->database_root);
    path contract_dir(liquid_contract);
    path file(fmt::format("{}.bin",date));
    path full_path = base_dir / contract_dir / file;
    return full_path.string();
}

