#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

namespace gdp1 {

class SQLiteDatabase {
private:
    sqlite3* db;
    std::string dbPath;

public:
    SQLiteDatabase(const std::string& path);
    ~SQLiteDatabase();

    bool open();

    void close();

    bool execute(const std::string& query);

    std::vector<std::vector<std::string>> query(const std::string& query);
};

}  // namespace gdp1
