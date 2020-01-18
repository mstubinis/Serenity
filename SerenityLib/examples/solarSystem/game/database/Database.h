#pragma once
#ifndef GAME_DATABASE_H
#define GAME_DATABASE_H

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
        Database();
        ~Database();

        const bool init_database_with_defaults(const bool forceDeleteOfPrevData = false);

        std::unordered_map<std::string, std::string> execute_query(const std::string& sql_query);

        const bool connect_to_database(const std::string& databaseFile);
        int disconnect_from_database();
};

#endif