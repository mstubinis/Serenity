#include "Database.h"
#include "../security/Security.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

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

const bool Database::create_new_server(const string& in_serverName, const unsigned short& in_serverPort, const string& in_ownerName, const string& in_ownerPassword) {
    string sql_add_server = "INSERT INTO STHS_ServerList (Name, OwnerUsername, Port) VALUES (?,?,?)";
    sqlite3_stmt* st_add_server;
    sqlite3_prepare_v2(m_Database.database, sql_add_server.c_str(), -1, &st_add_server, NULL);
    sqlite3_bind_text(st_add_server, 1, in_serverName.c_str(), in_serverName.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(st_add_server, 2, in_ownerName.c_str(), in_ownerName.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(st_add_server, 3, in_serverPort);
    sqlite3_step(st_add_server);
    sqlite3_finalize(st_add_server);

    create_new_account(const_cast<string&>(in_serverName), in_ownerName, in_ownerPassword);

    return true;
}
const bool Database::create_new_account(string& in_serverName, const string& in_accountName, const string& in_accountPassword) {  
    //first, process / sanitize user input
    const bool server_name_changed = Database::sanitize_user_input(in_serverName);
    string salt = Security::generate_user_salt(in_accountName, Database::CONST_SALT_LENGTH_MAX);
    string pw   = Security::argon2id(salt, in_accountPassword, 1, 2, 1 << 16, Database::CONST_SALT_LENGTH_MAX, Database::CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX);

    // now retrieve server id
    int server_id = -1;
    //TODO: secure this?
    const auto results = execute_query_and_return_results("SELECT ServerID FROM STHS_ServerList WHERE Name = " + in_serverName + ";");
    server_id = stoi(results["ServerID"][0]);

    //TODO: handle when this fails?
    if (server_id != -1) {
        string sql_add_account = "INSERT INTO STHS_Account (ServerID, Username, Password, Salt) VALUES (?,?,?,?)";
        sqlite3_stmt* st_add_account;
        sqlite3_prepare_v2(m_Database.database, sql_add_account.c_str(), -1, &st_add_account, NULL);
        sqlite3_bind_int(st_add_account, 1, server_id);
        sqlite3_bind_text(st_add_account, 2, in_accountName.c_str(), in_accountName.size(), SQLITE_TRANSIENT);
        sqlite3_bind_text(st_add_account, 3, pw.c_str(), pw.size(), SQLITE_TRANSIENT);
        sqlite3_bind_text(st_add_account, 4, salt.c_str(), salt.size(), SQLITE_TRANSIENT);
        sqlite3_step(st_add_account);
        sqlite3_finalize(st_add_account);

        return true;
    }
    return false;
}


const bool Database::sanitize_user_input(string& input) {
    string cpy = (input);
    auto lamda = [&](char c) {
        if (c == '\0' || c == '\'' || c == '\"' || c == '\b' || c == '\n' || c == '\r' || c == '\t' || c == '\Z' || c == '\\' || c == '\%' || c == '\_' || c == ' ' || c == '_' || c == ';' || c == ':'
        || c == '+' || c == '=' || c == '-' || c == '(' || c == ')' || c == '{' || c == '}' || c == '?' || c == '/' || c == '>' || c == '<' || c == '.' || c == ',' || c == ']' || c == '[' || c == '*'
        || c == '&' || c == '^' || c == '%' || c == '$' || c == '#' || c == '!' || c == '"' || c == '`' || c == '~')
            return true;
        return false;
    };
    input.erase(std::remove_if(input.begin(), input.end(), lamda), input.end());
    /*
    auto lamda_permute = [&](string& input_, vector<string>& results) {
        results.clear();
        int n = input_.length();

        // Number of permutations is 2^n 
        int max = 1 << n;

        // Converting string to lower case 
        transform(input_.begin(), input_.end(), input_.begin(), ::tolower);
        // Using all subsequences and permuting them
        results.reserve(max);
        for (int i = 0; i < max; i++) {

            // If j-th bit is set, we convert it to upper case
            string combination = input_;
            for (int j = 0; j < n; j++)
                if (((i >> j) & 1) == 1)
                    combination[j] = toupper(input_.at(j));
            results.push_back(combination);
        }
    };
    
    vector<string> bad_strs = {
        "DROP",
        "DELETE",
        "ADD",
        "CREATE",
        "ALTER",
        "TABLE",
        "SET",
        "RENAME",
        "INSERT",
        "DETACH",
        "CASCADE",
        "UPDATE",
    };

    vector<string> perms;
    for (auto& bad_str : bad_strs) {
        lamda_permute(bad_str, perms);
        for (auto& str : perms)
            boost::replace_all(input, str, "");
    }
    */
    return (cpy == input) ? false : true;
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
        ("CREATE TABLE IF NOT EXISTS STHS_ServerList ("
            "ServerID INTEGER PRIMARY KEY AUTOINCREMENT, " 
            "Name varchar(" + to_string(CONST_SERVER_NAME_LENGTH_MAX) + ") NOT NULL UNIQUE, "
            "OwnerUsername varchar(" + to_string(CONST_USERNAME_LENGTH_MAX) + ") NOT NULL UNIQUE, "
            "Port INTEGER NOT NULL, "
            "FOREIGN KEY(OwnerUsername) REFERENCES Account(Username)"
        ")").c_str(), 
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        ("CREATE TABLE IF NOT EXISTS STHS_Account ("
            "AccountID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ServerID INTEGER, "
            "Username varchar(" + to_string(CONST_USERNAME_LENGTH_MAX) + ") NOT NULL UNIQUE, "
            "Password CHAR(" + to_string(CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX) + ") NOT NULL, " //stored as a hash
            "Salt varchar(" + to_string(CONST_SALT_LENGTH_MAX) + ") NOT NULL, "
            "FOREIGN KEY(ServerID) REFERENCES ServerList(ServerID)"
        ")").c_str(),
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        ("CREATE TABLE IF NOT EXISTS STHS_ShipPerk ("
            "ShipID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ShipClass varchar(" + to_string(CONST_SHIP_CLASS_LENGTH_MAX) + ") NOT NULL UNIQUE, "
            "PerkID INTEGER NOT NULL UNIQUE, "
            "ExperienceNeeded UNSIGNED BIG INT NOT NULL"
        ")").c_str(),
    Callbacks::callback_empty, 0, &m_ErrorMsg));

    eval_sqlite3_statement(sqlite3_exec(m_Database.database,
        "CREATE TABLE IF NOT EXISTS STHS_AccountExperience ("
            "AccountID INTEGER NOT NULL, "
            "ServerID INTEGER NOT NULL, "
            "ShipID INTEGER NOT NULL UNIQUE, "
            "TotalExperience UNSIGNED BIG INT, "
            "FOREIGN KEY(AccountID) REFERENCES Account(AccountID), "
            "FOREIGN KEY(ShipID) REFERENCES ShipPerk(ShipID), "
            "FOREIGN KEY(ServerID) REFERENCES ServerList(ServerID), "
            "PRIMARY KEY(AccountID, ServerID, ShipID)"
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
        std::cout << "Can't open database: " << sqlite3_errmsg(m_Database.database) << std::endl;
        const auto close_return_val = disconnect_from_database();
        return false;
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



const vector<string>& DatabaseQueryResult::operator[](const size_t& column_index) const {
    return columns[column_index];
}
const vector<string>& DatabaseQueryResult::operator[](const string& column_name) const {
    const auto& c_index = column_name_to_index.at(column_name);
    return columns[c_index];
}
const vector<string>& DatabaseQueryResult::operator[](const char* column_name) const {
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
const string& DatabaseQueryResult::getValueAtRowCol(const size_t& row, const char* column) {
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
