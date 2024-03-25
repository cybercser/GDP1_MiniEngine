#pragma once

#include "IO/KillsDAO.h"
#include "IO/CoinsDAO.h"
#include "IO/ScoresDAO.h"

#include "PlayerEventType.h"

class GameplayManager {
private:
    gdp1::SQLiteDatabase& db;

    KillsDAO killsDao;
    CoinsDAO coinsDao;
    ScoresDAO scoresDao;

public:
    GameplayManager(gdp1::SQLiteDatabase& database);

    bool playerKilledZombie(int playerId, int zombieId);

    int getPlayerKillCount(int playerId);

    bool playerCollectedCoins(int playerId, int coins);

    int getPlayerTotalCoins(int playerId);

    bool playerScored(int playerId, int score);

    int getPlayerHighestScore(int playerId);

    // Other gameplay-related methods can be added here

    // Example method to handle gameplay logic
    void handleGameplayEvent(int playerId, PlayerEventType eventType, int eventData);
};
