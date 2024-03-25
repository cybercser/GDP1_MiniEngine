#pragma once

#include "IO/sqlite_database.h"

class ScoresDAO {
private:
    gdp1::SQLiteDatabase& db;

public:
    ScoresDAO(gdp1::SQLiteDatabase& database)
        : db(database) {
        createTable();
    }

    bool createTable() {
        std::string query =
            "CREATE TABLE IF NOT EXISTS scores ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "player_id INTEGER NOT NULL,"
            "score INTEGER NOT NULL"
            ")";
        return db.execute(query);
    }

    bool saveScore(int playerId, int score) {
        std::string query = "INSERT INTO scores (player_id, score) VALUES (" + std::to_string(playerId) + ", " +
                            std::to_string(score) + ")";
        return db.execute(query);
    }

    int getHighestScore(int playerId) {
        std::string query = "SELECT MAX(score) FROM scores WHERE player_id = " + std::to_string(playerId);
        std::vector<std::vector<std::string>> result = db.query(query);
        if (!result.empty()) {
            return std::stoi(result[0][0]);
        }
        return 0;  // Default to 0 if no result or error
    }
};