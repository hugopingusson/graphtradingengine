//
// Created by hugo on 13/04/25.
//

#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H
#include <boost/filesystem.hpp>
#include <fmt/core.h>
using namespace std;
using namespace boost::filesystem;

class DataBaseHelper {

    public:
    DataBaseHelper();

    string get_data_path(const string& date,const string& instrument,const string& exchange);
    string get_future_data_path(const string& date,const string& instrument);

    protected:
    string database_root;

};



#endif //DATABASEHELPER_H
