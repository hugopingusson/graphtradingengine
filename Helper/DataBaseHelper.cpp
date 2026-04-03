//
// Created by hugo on 13/04/25.
//

#include "DataBaseHelper.h"
#include "FutureHelper.h"
#include "SaphirManager.h"


DataBaseHelper::DataBaseHelper():database_root("/media/hugo/T7/market_data/databento/mbp10_bin") {}


string DataBaseHelper::get_data_path(const string &date, const string &instrument, const string &exchange) {
    SaphirManager sm =SaphirManager();
    if (sm.get_futures_instruments().contains(instrument)){
        FutureHelper future_helper=FutureHelper(instrument);
        string liquid_contract = future_helper.get_liquid_contract(date);
        path base_dir(sm.get_database_root(exchange));
        path contract_dir(liquid_contract);
        path exchange_dir(exchange);
        path file(fmt::format("{}.bin",date));
        path full_path = base_dir / exchange_dir/contract_dir / file;
        return full_path.string();
    }

    else {
        path base_dir(sm.get_database_root(exchange));
        path contract_dir(instrument);
        path exchange_dir(exchange);
        path file(fmt::format("{}.bin",date));
        path full_path = base_dir / exchange_dir/contract_dir/ file;
        return full_path.string();
    }

    throw std::runtime_error(fmt::format("DataBaseHelper::get_data_path: exchange={} not supported",exchange));

}


// string DataBaseHelper::get_future_data_path(const string& date,const string& instrument) {
//     FutureHelper future_helper=FutureHelper(instrument);
//     string liquid_contract = future_helper.get_liquid_contract(date);
//     SaphirManager sm =SaphirManager();
//
//     path base_dir(sm.get_database_root(exchange));
//     path contract_dir(liquid_contract);
//     path file(fmt::format("{}.bin",date));
//     path full_path = base_dir / contract_dir / file;
//     return full_path.string();
// }

