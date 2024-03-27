#include "sqlite_database.h"

#include <iostream>
#include <string>
#include <vector>
#include <common.h>

namespace gdp1 {

SQLiteDatabase::SQLiteDatabase(const std::string& path)
    : db(nullptr)
    , dbPath(path) {
    }

SQLiteDatabase::~SQLiteDatabase() { close(); }

bool SQLiteDatabase::open() {
    int result = sqlite3_open(dbPath.c_str(), &db);
    if (result != SQLITE_OK) {
        LOG_ERROR("Error opening database: ", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return false;
    }
    return true;
}

void SQLiteDatabase::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool SQLiteDatabase::execute(const std::string& query) {
    char* errMsg;
    int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOG_ERROR("Error executing query: ", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::vector<std::vector<std::string>> SQLiteDatabase::query(const std::string& query) {
    std::vector<std::vector<std::string>> resultRows;

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int columns = sqlite3_column_count(stmt);
            std::vector<std::string> row;
            for (int i = 0; i < columns; i++) {
                const unsigned char* value = sqlite3_column_text(stmt, i);
                if (value) {
                    row.push_back(std::string(reinterpret_cast<const char*>(value)));
                } else {
                    row.push_back("");
                }
            }
            resultRows.push_back(row);
        }
        sqlite3_finalize(stmt);
    } else {
        LOG_ERROR("Error preparing query: ", sqlite3_errmsg(db));
    }

    return resultRows;
}

}  // namespace gdp1
