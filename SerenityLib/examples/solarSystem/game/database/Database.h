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
    public:
        class Callbacks final {
            friend class Database;
            private:
                static int callback_empty(void* NotUsed, int argc, char** argv, char** azColName);
                static int callback(void* NotUsed, int column_count, char** column_values, char** column_names);
        };

        Database();
        ~Database();

        const bool init_database_with_defaults(const bool forceDeleteOfPrevData = false);

        void execute_query(const std::string& sql_query);
        const DatabaseQueryResult execute_query_and_return_results(const std::string& sql_query);

        const bool connect_to_database(const std::string& databaseFile);
        int disconnect_from_database();
};


class DatabaseQueryResult final {
    friend class Database;
    friend class Database::Callbacks;
    private:

        std::unordered_map<std::string, size_t> column_name_to_index;
        std::vector<std::vector<std::string>>   columns;

    public:
        const std::string& getValueAtRowCol(const size_t& row, const size_t& column);
        const std::string& getValueAtRowCol(const size_t& row, const std::string& column);

        std::vector<std::string>& operator[](const size_t& column_index);
        std::vector<std::string>& operator[](const std::string& column_name);

        inline const size_t num_rows() const;
        inline const size_t num_columns() const;

        inline const bool has_column(const std::string& column);

        void clear();
};

#endif