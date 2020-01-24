#pragma once
#ifndef GAME_DATABASE_H
#define GAME_DATABASE_H

class  Database;
class  DatabaseQueryResult;

#include <unordered_map>
#include <vector>
#include <string>
#include <sqlite3.h>

struct SqliteDatabase final {
    sqlite3*      database;
    std::string   databaseName;
    std::string   databaseFile;
    SqliteDatabase() {
        database = nullptr;
        databaseName = "";
        databaseFile = "";    
    }
};



class Database final {
    private:
        SqliteDatabase      m_Database;
        char*               m_ErrorMsg;

        void eval_sqlite3_statement(int sql_return_val);
        void process_row_and_save(sqlite3_stmt* statement);

        void init_tables();
        void init_default_ship_designs();
        void init_default_ship_perks();

        const bool init_database_with_defaults(const bool forceDeleteOfPrevData = false);

    public:

        static const unsigned int CONST_USERNAME_LENGTH_MAX = 18;
        static const unsigned int CONST_USERNAME_PASSWORD_LENGTH_MAX = 18;
        static const unsigned int CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX = 60;
        static const unsigned int CONST_SERVER_NAME_LENGTH_MAX = 30;
        static const unsigned int CONST_SALT_LENGTH_MAX = 16;
        static const unsigned int CONST_SHIP_CLASS_LENGTH_MAX = 22;

        class Callbacks final {
            friend class Database;
            private:
                static int callback_empty(void* NotUsed, int argc, char** argv, char** azColName);
                static int callback(void* NotUsed, int column_count, char** column_values, char** column_names);
        };

        Database();
        ~Database();

        //attempts to remove common harmful sql from the user input string. this should not be the only method of safety you use, use prepared statements and parameter binding as well whenever possible
        //this is an expensive cpu operation. returns true if the input was modified in some way (sanitized). false otherwise.
        const bool sanitize_user_input(std::string& input);

        //this is NOT SQL-injection safe. do NOT modify the sql_query with user input UNLESS you manually whitelist / sanitize the sql_query string
        void execute_query(const std::string& sql_query, const bool printRes = false);

        //this is NOT SQL-injection safe. do NOT modify the sql_query with user input UNLESS you manually whitelist / sanitize the sql_query string
        const DatabaseQueryResult execute_query_and_return_results(const std::string& sql_query, const bool printRes = false);

        int prepare_sql_statement(const std::string& sql_query, sqlite3_stmt** statement);

        void execute_query(sqlite3_stmt* statement);
        const DatabaseQueryResult execute_query_and_return_results(sqlite3_stmt* statement);



        const bool connect_to_database(const std::string& databaseFile);
        int disconnect_from_database();

        const bool create_new_server(const std::string& serverName, const unsigned short& serverPort, const std::string& ownerName, const std::string& ownerPassword);
        const bool create_new_account(std::string& serverName, const std::string& accountName, const std::string& accountPassword);

        const DatabaseQueryResult get_account(const std::string& username, const int& server_id);
        const int get_server_id(const std::string& serverName);
};


class DatabaseQueryResult final {
    friend class Database;
    friend class Database::Callbacks;

    private:

        std::unordered_map<std::string, size_t> column_name_to_index;
        std::vector<std::vector<std::string>>   rows;

    public:
        std::vector<std::vector<std::string>>::iterator begin();
        std::vector<std::vector<std::string>>::iterator end();
        std::vector<std::vector<std::string>>::const_iterator begin() const;
        std::vector<std::vector<std::string>>::const_iterator end() const;

        const std::string& get(const size_t& row, const std::string& column_name) const;
        const std::string& get(const size_t& row, const size_t& column_name) const;

        const std::string& get_column_name(const size_t& index) const;

        inline const size_t num_rows() const;
        inline const size_t num_columns() const;

        inline const bool has_column(const std::string& column) const;

        void clear();
};

#endif