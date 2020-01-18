#include "Database.h"

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;

std::unordered_map<std::string, std::string> query_result;

static int callback_empty(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        query_result[azColName[i]] = (argv[i] ? argv[i] : "NULL");
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
    callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS Account ("
            "AccountID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ServerID INTEGER, "
            "Username varchar(30) NOT NULL UNIQUE, "
            "Password CHAR(60) NOT NULL, " //stored as a bcrypted hash
            "Salt varchar(16) NOT NULL, "
            "FOREIGN KEY(ServerID) REFERENCES ServerList(ServerID)"
        ")",
    callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS ShipPerk ("
            "ShipID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ShipClass varchar(30) NOT NULL UNIQUE, "
            "PerkID INTEGER NOT NULL UNIQUE, "
            "ExperienceNeeded UNSIGNED BIG INT NOT NULL"
        ")",
    callback_empty, 0, &m_ErrorMsg));

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
    callback_empty, 0, &m_ErrorMsg));


    return true;
}
unordered_map<string,string> Database::execute_query(const string& sql_query) {
    query_result.clear();
    auto res = sqlite3_exec(m_Database.database, sql_query.c_str(), callback, 0, &m_ErrorMsg);
    unordered_map<string, string> results;
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
    auto results = execute_query("select count(type) from sqlite_master where type='table' and name='ServerList';");
    if (results.count("count(type)")) {
        if (results.at("count(type)") == "0") {
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