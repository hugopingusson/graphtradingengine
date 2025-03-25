//
// Created by hugo on 15/03/25.
//

#include "FutureHelper.h"

struct FutureInfo{
    std::map<int,std::string> months_dict = {
        {1, "F"},  // January
        {2, "G"},  // February
        {3, "H"},  // March
        {4, "J"},  // April
        {5, "K"},  // May
        {6, "M"},  // June
        {7, "N"},  // July
        {8, "Q"},  // August
        {9, "U"},  // September
        {10, "V"}, // October
        {11, "X"}, // November
        {12, "Z"}  // December
    };
    std::map<std::string,std::string> contract_code ={{"EURUSD","6E"}};
    std::map<std::string,std::vector<std::string>> contract_scheme = {
        {"quarterly", {"6E"}}
    };
};


using namespace std;
using namespace boost::gregorian;


FutureHelper::FutureHelper(const std::string& pair):pair(pair){

}


std::map<int,date> FutureHelper::get_roll_date(const int& year, const std::string &schema) {
    std::vector<int> months;

    if(schema=="quarterly") {
        months = std::vector<int>({3,6,9,12});
    }else if(schema=="monthly") {
        months = std::vector<int>({1,2,3,4,5,6,7,8,9,10});
    }else {
        throw std::invalid_argument("Invalid schema specified");
    }

    std::map<int,date> roll_dates;

    for (int i = 0; i < months.size(); i++) {
        roll_dates[months[i]]=date(year,months[i],01)+days(3);
    }

    return roll_dates;

}


std::string FutureHelper::get_liquid_contract(const std::string& date) {
    boost::gregorian::date boost_date = boost::gregorian::date(from_simple_string(date));
    FutureInfo future_info = FutureInfo();

    std::string contract_code=future_info.contract_code.at(this->pair);
    std::string contract_schema;



    for (auto contract_schema_it : future_info.contract_scheme) {
        if (std::count(contract_schema_it.second.begin(),contract_schema_it.second.end(),contract_code)) {
            contract_schema=contract_schema_it.first;
        }
    }

    if (contract_schema.empty()) {
        throw std::invalid_argument("couldn't not find contract scheme in get_liquid_contract");
    }

    std::map<int,boost::gregorian::date> roll_dates=FutureHelper::get_roll_date(boost_date.year(),contract_schema);
    int maturity = 0;

    std::map<int,boost::gregorian::date>::iterator it;
    for (it =roll_dates.begin(); it != roll_dates.end(); it++) {

        if (boost_date<it->second) {
            maturity=it->first;
            break;
        }

    }

    if (maturity==0) {
        return contract_code+future_info.months_dict.at(roll_dates.begin()->first)+std::to_string((boost_date.year()+1)%10);
    }else {
        return contract_code+future_info.months_dict.at(maturity)+std::to_string(boost_date.year()%10);
    }

}

