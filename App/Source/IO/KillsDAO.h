#pragma once


#include "IO/sqlite_database.h"

class KillsDAO {
private:
    gdp1::SQLiteDatabase& db;

public:
    KillsDAO(gdp1::SQLiteDatabase& database)
        : db(database) {
        createTable();
    }

    bool createTable() {
        std::string query =
            "CREATE TABLE IF NOT EXISTS kills ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "player_id INTEGER NOT NULL,"
            "zombie_id INTEGER NOT NULL"
            ")";
        return db.execute(query);
    }

    bool saveKill(int playerId, int zombieId) {
        std::string query = "INSERT INTO kills (player_id, zombie_id) VALUES (" + std::to_string(playerId) + ", " +
                            std::to_string(zombieId) + ")";
        return db.execute(query);
    }

    int getKillCount(int playerId) {
        std::string query = "SELECT COUNT(*) FROM kills WHERE player_id = " + std::to_string(playerId);
        std::vector<std::vector<std::string>> result = db.query(query);
        if (!result.empty()) {
            return std::stoi(result[0][0]);
        }
        return 0;  // Default to 0 if no result or error
    }
};