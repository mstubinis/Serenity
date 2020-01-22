#include "Database.h"
#include "../security/Security.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../ships/Ships.h"

#include <iostream>

using namespace std;

/*
#define SQLITE_OK           0   Successful result
#define SQLITE_ERROR        1   Generic error
#define SQLITE_INTERNAL     2   Internal logic error in SQLite
#define SQLITE_PERM         3   Access permission denied
#define SQLITE_ABORT        4   Callback routine requested an abort
#define SQLITE_BUSY         5   The database file is locked
#define SQLITE_LOCKED       6   A table in the database is locked
#define SQLITE_NOMEM        7   A malloc() failed
#define SQLITE_READONLY     8   Attempt to write a readonly database
#define SQLITE_INTERRUPT    9   Operation terminated by sqlite3_interrupt()
#define SQLITE_IOERR       10   Some kind of disk I/O error occurred
#define SQLITE_CORRUPT     11   The database disk image is malformed
#define SQLITE_NOTFOUND    12   Unknown opcode in sqlite3_file_control()
#define SQLITE_FULL        13   Insertion failed because database is full
#define SQLITE_CANTOPEN    14   Unable to open the database file
#define SQLITE_PROTOCOL    15   Database lock protocol error
#define SQLITE_EMPTY       16   Internal use only
#define SQLITE_SCHEMA      17   The database schema changed
#define SQLITE_TOOBIG      18   String or BLOB exceeds size limit
#define SQLITE_CONSTRAINT  19   Abort due to constraint violation
#define SQLITE_MISMATCH    20   Data type mismatch
#define SQLITE_MISUSE      21   Library used incorrectly
#define SQLITE_NOLFS       22   Uses OS features not supported on host
#define SQLITE_AUTH        23   Authorization denied
#define SQLITE_FORMAT      24   Not used
#define SQLITE_RANGE       25   2nd parameter to sqlite3_bind out of range
#define SQLITE_NOTADB      26   File opened that is not a database file
#define SQLITE_NOTICE      27   Notifications from sqlite3_log()
#define SQLITE_WARNING     28   Warnings from sqlite3_log()
#define SQLITE_ROW         100  sqlite3_step() has another row ready
#define SQLITE_DONE        101  sqlite3_step() has finished executing
end-of-error-codes */


DatabaseQueryResult query_result;

const string server_list_table_name      = "STHS_ServerList";
const string account_table_name          = "STHS_Account";
const string ship_perk_table_name        = "STHS_ShipPerk";
const string account_ship_exp_table_name = "STHS_AccountExperience";
const string ship_table_name             = "STHS_Ship";

int Database::Callbacks::callback_empty(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}
int Database::Callbacks::callback(void* NotUsed, int column_count, char** column_values, char** column_names) {
    //this function is invoked for each row
    if (query_result.column_name_to_index.size() == 0) {
        query_result.column_name_to_index.reserve(column_count);
        for (int i = 0; i < column_count; i++) {
            const auto val = column_names[i];
            query_result.column_name_to_index.emplace(val, i);
        }
    }

    query_result.rows.emplace_back();
    auto& row = query_result.rows[query_result.rows.size() - 1];
    row.reserve(column_count);
    for (int i = 0; i < column_count; i++) {
        const auto val = (column_values[i] ? column_values[i] : "NULL");
        row.push_back(std::move(val));
    }
    return 0;
}

Database::Database() {
    m_ErrorMsg = 0;
}
Database::~Database() {
    disconnect_from_database();
}


void Database::init_tables() {
    string sql1 = ("CREATE TABLE IF NOT EXISTS " + server_list_table_name + "("
        "ServerID INTEGER, "
        "Name varchar(" + to_string(CONST_SERVER_NAME_LENGTH_MAX) + ") NOT NULL UNIQUE, "
        "OwnerUsername varchar(" + to_string(CONST_USERNAME_LENGTH_MAX) + ") NOT NULL, "
        "Port INTEGER NOT NULL, "
        "PRIMARY KEY(ServerID), "
        "FOREIGN KEY(OwnerUsername) REFERENCES " + account_table_name + "(Username)"
        ");");
    (sqlite3_exec(m_Database.database, sql1.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));

    string sql2 = ("CREATE TABLE IF NOT EXISTS " + account_table_name + "("
        "AccountID INTEGER, "
        "ServerID INTEGER, "
        "Username varchar(" + to_string(CONST_USERNAME_LENGTH_MAX) + ") NOT NULL, "
        "Password CHAR(" + to_string(CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX) + ") NOT NULL, " //stored as a hash
        "Salt varchar(" + to_string(CONST_SALT_LENGTH_MAX) + ") NOT NULL, "
        "PRIMARY KEY(AccountID), "
        "FOREIGN KEY(ServerID) REFERENCES " + server_list_table_name + "(ServerID)"
        ");");
    (sqlite3_exec(m_Database.database, sql2.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));

    string sql3 = ("CREATE TABLE IF NOT EXISTS " + ship_perk_table_name + "("
        "PerkID INTEGER, "
        "ShipID INTEGER NOT NULL, "
        "ExperienceNeeded UNSIGNED BIG INT NOT NULL, "
        "PRIMARY KEY(PerkID), "
        "FOREIGN KEY(ShipID) REFERENCES " + ship_table_name + "(ShipID) "
        ");");
    (sqlite3_exec(m_Database.database, sql3.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));

    string sql4 = ("CREATE TABLE IF NOT EXISTS " + account_ship_exp_table_name + "("
        "AccountID INTEGER NOT NULL, "
        "ServerID INTEGER NOT NULL, "
        "ShipID INTEGER NOT NULL, "
        "TotalExperience UNSIGNED BIG INT, "
        "FOREIGN KEY(AccountID) REFERENCES " + account_table_name + "(AccountID), "
        "FOREIGN KEY(ShipID) REFERENCES " + ship_perk_table_name + "(ShipID), "
        "FOREIGN KEY(ServerID) REFERENCES " + server_list_table_name + "(ServerID), "
        "PRIMARY KEY(AccountID, ServerID, ShipID)"
        ");");
    (sqlite3_exec(m_Database.database, sql4.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));

    string sql5 = ("CREATE TABLE IF NOT EXISTS " + ship_table_name + "("
        "ShipID INTEGER, "
        "ShipClass varchar(" + to_string(CONST_SHIP_CLASS_LENGTH_MAX) + ") NOT NULL, "
        "PRIMARY KEY(ShipID)"
        ");");
    (sqlite3_exec(m_Database.database, sql5.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));
}
void Database::init_default_ship_designs() {
    string query = "INSERT INTO " + ship_table_name + "(ShipClass) VALUES (?)";

    for (auto& ship_info : Ships::Database) {
        auto& ship_class = ship_info.second.Class;

        sqlite3_stmt* statement_add_ship;
        prepare_sql_statement(query, &statement_add_ship);
        (sqlite3_bind_text(statement_add_ship, 1, ship_class.c_str(), ship_class.length(), SQLITE_STATIC));

        execute_query(statement_add_ship);
    }
}
void Database::init_default_ship_perks() {
    /*
    string sql3 = ("CREATE TABLE IF NOT EXISTS " + ship_perk_table_name + "("
        "PerkID INTEGER, "
        "ShipID INTEGER NOT NULL, "
        "ExperienceNeeded UNSIGNED BIG INT NOT NULL, "
        "PRIMARY KEY(PerkID), "
        "FOREIGN KEY(ShipID) REFERENCES " + ship_table_name + "(ShipID)"
        ");");
    (sqlite3_exec(m_Database.database, sql3.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg));
    */

}

const bool Database::create_new_server(const string& in_serverName, const unsigned short& in_serverPort, const string& in_ownerName, const string& in_ownerPassword) {
    string sql_add_server = "INSERT INTO " + server_list_table_name + "(Name, OwnerUsername, Port) VALUES (?,?,?)";
    sqlite3_stmt* st_add_server;

    (sqlite3_prepare_v2(m_Database.database, sql_add_server.c_str(), -1, &st_add_server, NULL));
    (sqlite3_bind_text(st_add_server, 1, in_serverName.c_str(), in_serverName.length(), SQLITE_STATIC));
    (sqlite3_bind_text(st_add_server, 2, in_ownerName.c_str(),  in_ownerName.length(),  SQLITE_STATIC));
    (sqlite3_bind_int( st_add_server, 3, in_serverPort));
    (sqlite3_step(st_add_server));
    (sqlite3_finalize(st_add_server));

    create_new_account(const_cast<string&>(in_serverName), in_ownerName, in_ownerPassword);

    return true;
}
const DatabaseQueryResult Database::get_account(const string& username, const int& server_id) {
    string query_account = "SELECT * FROM STHS_Account WHERE ServerID =? AND Username =?;";
    sqlite3_stmt* st_get_acc;
    prepare_sql_statement(query_account, &st_get_acc);
    sqlite3_bind_int(st_get_acc, 1, server_id);
    sqlite3_bind_text(st_get_acc, 2, username.c_str(), username.length(), SQLITE_STATIC);
    auto res_acc = execute_query_and_return_results(st_get_acc);
    return std::move(res_acc);
}
const int Database::get_server_id(const string& serverName) {
    // now retrieve server id
    int server_id = -1;
    string query = "SELECT ServerID FROM " + server_list_table_name + " WHERE Name =?;";
    sqlite3_stmt* st_get_server_id;
    prepare_sql_statement(query, &st_get_server_id);
    sqlite3_bind_text(st_get_server_id, 1, serverName.c_str(), serverName.length(), SQLITE_STATIC);
    auto results = execute_query_and_return_results(st_get_server_id);
    server_id = stoi(results.get(0, "ServerID"));
    return server_id;
}
const bool Database::create_new_account(string& in_serverName, const string& in_accountName, const string& in_accountPassword) {  
    string account_salt      = Security::generate_user_salt(in_accountName, Database::CONST_SALT_LENGTH_MAX);
    string pepper            = "gjsuwmejfidlskdfig59dgfkkdfiejrmfj";
    string peppered_password = Security::sha256(pepper + in_accountPassword);
    string hashed_password   = Security::argon2id(account_salt, peppered_password, 1, 2, 1 << 16, Database::CONST_SALT_LENGTH_MAX, Database::CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX);

    int server_id = get_server_id(in_serverName);

    //TODO: handle when this fails?
    if (server_id != -1) {
        string sql_add_account = "INSERT INTO " + account_table_name + "(ServerID, Username, Password, Salt) VALUES (?,?,?,?)";
        sqlite3_stmt* st_add_account;
        (sqlite3_prepare_v2(m_Database.database, sql_add_account.c_str(), -1, &st_add_account, NULL));
        (sqlite3_bind_int( st_add_account, 1, server_id));
        (sqlite3_bind_text(st_add_account, 2, in_accountName.c_str(),   in_accountName.length(), SQLITE_STATIC));
        (sqlite3_bind_text(st_add_account, 3, hashed_password.c_str(),  hashed_password.size(),  SQLITE_STATIC));
        (sqlite3_bind_text(st_add_account, 4, account_salt.c_str(),     account_salt.size(),     SQLITE_STATIC));
        (sqlite3_step(st_add_account));
        (sqlite3_finalize(st_add_account));

        return true;
    }
    return false;
}

int Database::prepare_sql_statement(const string& sql_query, sqlite3_stmt** statement) {
    int result = sqlite3_prepare_v2(m_Database.database, sql_query.c_str(), -1, statement, NULL);
    return result;
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
    if (sql_return_val != SQLITE_OK && sql_return_val != SQLITE_DONE && sql_return_val != SQLITE_ROW) {
        std::cout << "SQL error: " << sqlite3_errmsg(m_Database.database) << std::endl;
        sqlite3_free(m_ErrorMsg);
    }
}
const bool Database::init_database_with_defaults(const bool forceDeleteOfPrevData) {
    if (!m_Database.database)
        return false;
    if (forceDeleteOfPrevData) {
        //delete prev database content
        if (boost::filesystem::exists(m_Database.databaseFile)) {
            const auto deleted = boost::filesystem::remove(m_Database.databaseFile);
            if (deleted) {
                connect_to_database(m_Database.databaseName);
            }
        }
    }

    init_tables();
    init_default_ship_designs();
    init_default_ship_perks();

    return true;
}
void Database::execute_query(const string& sql_query, const bool printRes) {
    query_result.clear();
    auto res = sqlite3_exec(m_Database.database, sql_query.c_str(), Callbacks::callback_empty, 0, &m_ErrorMsg);
    if (printRes)
        eval_sqlite3_statement(res);
}
const DatabaseQueryResult Database::execute_query_and_return_results(const string& sql_query, const bool printRes) {
    query_result.clear();
    auto res = sqlite3_exec(m_Database.database, sql_query.c_str(), Callbacks::callback, 0, &m_ErrorMsg);
    if (printRes)
        eval_sqlite3_statement(res);
    DatabaseQueryResult results;
    results = query_result;
    query_result.clear();
    return results;
}
void Database::process_row_and_save(sqlite3_stmt* statement) {

    int num_cols = sqlite3_data_count(statement);
    if (query_result.column_name_to_index.size() == 0) {
        query_result.column_name_to_index.reserve(num_cols);
        for (int i = 0; i < num_cols; i++) {
            const auto col_name = sqlite3_column_name(statement, i);
            query_result.column_name_to_index.emplace(col_name, i);
        }
    }

    query_result.rows.emplace_back();
    auto& row = query_result.rows[query_result.rows.size() - 1];
    row.reserve(num_cols);
    for (int i = 0; i < num_cols; ++i) {
        int type = sqlite3_column_type(statement, i);
        switch (type) {
            case SQLITE_INTEGER: {
                auto val = to_string(sqlite3_column_int(statement, i));
                row.push_back(std::move(val));
                break;
            }case SQLITE_FLOAT: {
                auto val = to_string(sqlite3_column_double(statement, i));
                row.push_back(std::move(val));
                break;
            }case SQLITE_BLOB: {
                //TODO: test if this actually works
                auto val = string(reinterpret_cast<const char*>(sqlite3_column_blob(statement, i)));
                row.push_back(std::move(val));
                break;
            }case SQLITE_NULL: {
                auto val = "";
                row.push_back(std::move(val));
                break;
            }case SQLITE3_TEXT: {
                const auto val = string(reinterpret_cast<const char*>(sqlite3_column_text(statement, i)));
                row.push_back(std::move(val));
                break;
            }default: {
                auto val = "";
                row.push_back(std::move(val));
                break;
            }
        }    
    }
}

const DatabaseQueryResult Database::execute_query_and_return_results(sqlite3_stmt* statement) {
    query_result.clear();
    while (sqlite3_step(statement) == SQLITE_ROW) {
        process_row_and_save(statement);
    }
    sqlite3_finalize(statement);
    DatabaseQueryResult results;
    results = query_result;
    query_result.clear();
    return results;
}
void Database::execute_query(sqlite3_stmt* statement) {
    query_result.clear();
    while (sqlite3_step(statement) == SQLITE_ROW) {
    }
    sqlite3_finalize(statement);
}

const bool Database::connect_to_database(const string& databaseName) {  
    disconnect_from_database();
    const int return_code = sqlite3_open(("../data/Server/" + databaseName + ".db").c_str(), &m_Database.database);
    if (return_code){
        std::cout << "Can't open database: " << sqlite3_errmsg(m_Database.database) << std::endl;
        disconnect_from_database();
        return false;
    }
    m_Database.databaseName = databaseName;
    m_Database.databaseFile = ("../data/Server/" + databaseName + ".db");
    string sql = "SELECT count(type) FROM sqlite_master WHERE type='table' AND name='" + server_list_table_name + "';";
    const auto results = execute_query_and_return_results(sql);
    if (results.has_column("count(type)")) {
        const auto& count = results.get(0, "count(type)");
        if (count == "0") {
            init_database_with_defaults();
        }
    }
    return true;
}
int Database::disconnect_from_database() {
    if (!m_Database.database)
        return 0;
    const int return_val = sqlite3_close(m_Database.database);
    return return_val;
}




const string& DatabaseQueryResult::get(const size_t& row, const string& column_name) const {
    const size_t& c_index = column_name_to_index.at(column_name);
    return rows[row][c_index];
}
const string& DatabaseQueryResult::get(const size_t& row, const size_t& column) const {
    return rows[row][column];
}
vector<vector<string>>::iterator DatabaseQueryResult::begin() {
    return rows.begin();
}
vector<vector<string>>::iterator DatabaseQueryResult::end() {
    return rows.end();
}
vector<vector<string>>::const_iterator DatabaseQueryResult::begin() const {
    return rows.begin();
}
vector<vector<string>>::const_iterator DatabaseQueryResult::end() const {
    return rows.end();
}

const bool DatabaseQueryResult::has_column(const string& column) const {
    return static_cast<bool>(column_name_to_index.count(column));
}
inline const size_t DatabaseQueryResult::num_rows() const {
    return rows.size();
}
inline const size_t DatabaseQueryResult::num_columns() const {
    return (rows.size() > 0) ? rows[0].size() : 0;
}
const string& DatabaseQueryResult::get_column_name(const size_t& index) const {
    for (auto& itr : column_name_to_index) {
        if (itr.second == index) {
            return itr.first;
        }
    }
    return "";
}
void DatabaseQueryResult::clear() {
    rows.clear();
    column_name_to_index.clear();
}
