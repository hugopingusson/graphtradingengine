//
// Created by hugo on 15/03/25.
//

#ifndef CLASS_FUTUREHELPER_H
#define CLASS_FUTUREHELPER_H

#include <chrono>
#include <map>
#include <vector>

#include "boost/date_time/gregorian/gregorian.hpp"

// inline struct {
//     std::map<int,std::string> months_dict;
//     std::map<std::string,std::string> contract_code;
//     std::map<std::string,std::vector<std::string>> contract_scheme;
// }FutureInfo;





class FutureHelper {
    public:
    // FutureHelper();
    explicit FutureHelper(const std::string& pair);
    static std::map<int,boost::gregorian::date> get_roll_date(const int& year,const std::string& schema);
    std::string get_liquid_contract(const std::string& date);


    protected:
    std::string pair;

};


#endif //CLASS_FUTUREHELPER_H
