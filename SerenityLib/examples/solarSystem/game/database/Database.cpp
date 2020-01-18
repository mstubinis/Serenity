#include "Database.h"

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;

DatabaseQueryResult query_result;

int Database::Callbacks::callback_empty(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}
int Database::Callbacks::callback(void* NotUsed, int column_count, char** column_values, char** column_names) {

    //this function is invoked for each row
    int i;
    if (query_result.num_columns() == 0) {
        query_result.columns.reserve(column_count);
        query_result.column_name_to_index.reserve(column_count);
        for (i = 0; i < column_count; i++) {
            query_result.column_name_to_index.emplace(column_names[i], i);
        }
        query_result.columns.emplace_back();
    }
    //inital memory in place for columns

    //i = 0;
    for (i = 0; i < column_count; i++) {
        auto& column = query_result.columns[i];
        const auto val = (column_values[i] ? column_values[i] : "NULL");
        column.push_back(std::move(val));
    }
    return 0;
}

Database::Database() {
    m_ErrorMsg = 0;
}
Database::~Database() {
    disconnect_from_database();
}

void Database::eval_sqlite3_statement(int sql_return_val) {
    if (sql_return_val != SQLITE_OK) {
        std::cout << "SQL error: " << sqlite3_errmsg(m_Database.database) << std::endl;
        sqlite3_free(m_ErrorMsg);
    }
}
const bool Database::init_database_with_defaults(const bool forceDeleteOfPrevData) {
    
    if (!m_Database.database)
        return false;

    if (forceDeleteOfPrevData) {
        //delete prev database content and then
        if (boost::filesystem::exists(m_Database.databaseFile)) {
            const auto deleted = boost::filesystem::remove(m_Database.databaseFile);
            if (deleted) {
                connect_to_database(m_Database.databaseName);
            }
        }
    }

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS ServerList ("
            "ServerID INTEGER PRIMARY KEY AUTOINCREMENT, " 
            "Name varchar(30) NOT NULL UNIQUE, "
            "OwnerUsername varchar(30) NOT NULL UNIQUE, "
            "Port INTEGER NOT NULL, "
            "Salt varchar(16) NOT NULL, "
            "FOREIGN KEY(OwnerUsername) REFERENCES Account(Username)"
        ")", 
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS Account ("
            "AccountID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ServerID INTEGER, "
            "Username varchar(30) NOT NULL UNIQUE, "
            "Password CHAR(60) NOT NULL, " //stored as a bcrypted hash
            "Salt varchar(16) NOT NULL, "
            "FOREIGN KEY(ServerID) REFERENCES ServerList(ServerID)"
        ")",
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS ShipPerk ("
            "ShipID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ShipClass varchar(30) NOT NULL UNIQUE, "
            "PerkID INTEGER NOT NULL UNIQUE, "
            "ExperienceNeeded UNSIGNED BIG INT NOT NULL"
        ")",
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS AccountExperience ("
            "AccountID INTEGER, "
            "ServerID INTEGER, "
            "ShipID INTEGER NOT NULL UNIQUE, "
            "TotalExperience UNSIGNED BIG INT, "
            "FOREIGN KEY(AccountID) REFERENCES Account(AccountID), "
            "FOREIGN KEY(ShipID) REFERENCES ShipPerk(ShipID), "
            "FOREIGN KEY(ServerID) REFERENCES ServerList(ServerID)"
        ")",
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    return true;
}
void Database::execute_query(const string& sql_query) {
    query_result.clear();
    auto res = sqlite3_exec(m_Database.database, sql_query.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg);
}
const DatabaseQueryResult Database::execute_query_and_return_results(const string& sql_query) {
    query_result.clear();
    auto res = sqlite3_exec(m_Database.database, sql_query.c_str(), Callbacks::callback, 0, &m_ErrorMsg);
    DatabaseQueryResult results;
    results = query_result;
    query_result.clear();
    return results;
}
const bool Database::connect_to_database(const string& databaseName) {
    
    disconnect_from_database();
    const auto return_code = sqlite3_open(("../data/Server/" + databaseName + ".db").c_str(), &m_Database.database);

    if (return_code){
        //std::cout << "Can't open database: " << sqlite3_errmsg(m_Database.database) << std::endl;
        const auto close_return_val = disconnect_from_database();
        return false;
    }else{
        //std::cout << "Open database successfully" << std::endl;
    }
    m_Database.databaseName = databaseName;
    m_Database.databaseFile = ("../data/Server/" + databaseName + ".db");
    auto results = execute_query_and_return_results("select count(type) from sqlite_master where type='table' and name='ServerList';");
    if (results.has_column("count(type)")) {
        if (results["count(type)"][0] == "0") {
            init_database_with_defaults();
        }
    }
    
    return true;
}
int Database::disconnect_from_database() {
    if (!m_Database.database)
        return 0;
    const auto return_val = sqlite3_close(m_Database.database);
    return return_val;
}



vector<string>& DatabaseQueryResult::operator[](const size_t& column_index) {
    return columns[column_index];
}
vector<string>& DatabaseQueryResult::operator[](const string& column_name) {
    const auto& c_index = column_name_to_index.at(column_name);
    return columns[c_index];
}
const string& DatabaseQueryResult::getValueAtRowCol(const size_t& row, const size_t& column) {
    return columns[column][row];
}
const string& DatabaseQueryResult::getValueAtRowCol(const size_t& row, const string& column) {
    const auto& c_index = column_name_to_index.at(column);
    return columns[c_index][row];
}
const bool DatabaseQueryResult::has_column(const string& column) {
    return static_cast<bool>(column_name_to_index.count(column));
}
inline const size_t DatabaseQueryResult::num_rows() const {
    return (columns.size() == 0) ? 0 : columns[0].size();
}
inline const size_t DatabaseQueryResult::num_columns() const {
    return columns.size();
}
void DatabaseQueryResult::clear() {
    columns.clear();
    column_name_to_index.clear();
}
