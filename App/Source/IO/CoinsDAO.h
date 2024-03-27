#pragma once

#include "IO/sqlite_database.h"

class CoinsDAO {
private:
    gdp1::SQLiteDatabase& db;

public:
    CoinsDAO(gdp1::SQLiteDatabase& database)
        : db(database) {
        createTable();
    }

    bool createTable() {
        std::string query =
            "CREATE TABLE IF NOT EXISTS coins ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "player_id INTEGER NOT NULL,"
            "coins_collected INTEGER NOT NULL"
            ")";
        return db.execute(query);
    }

    bool saveCoinsCollected(int playerId, int coins) {
        std::string query = "INSERT INTO coins (player_id, coins_collected) VALUES (" + std::to_string(playerId) +
                            ", " + std::to_string(coins) + ")";
        return db.execute(query);
    }

    int getTotalCoins(int playerId) {
        std::string query = "SELECT SUM(coins_collected) FROM coins WHERE player_id = " + std::to_string(playerId);
        std::vector<std::vector<std::string>> result = db.query(query);
        if (!result.empty()) {
            return std::stoi(result[0][0]);
        }
        return 0;  // Default to 0 if no result or error
    }
};