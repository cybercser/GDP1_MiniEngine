#include "GameplayManager.h"

GameplayManager::GameplayManager(gdp1::SQLiteDatabase& database)
    : db(database)
    , killsDao(database)
    , coinsDao(database)
    , scoresDao(database) {

    }

bool GameplayManager::playerKilledZombie(int playerId, int zombieId) {
    return killsDao.saveKill(playerId, zombieId) && playerScored(playerId, 10);
}

int GameplayManager::getPlayerKillCount(int playerId) { return killsDao.getKillCount(playerId); }

bool GameplayManager::playerCollectedCoins(int playerId, int coins) {
    return coinsDao.saveCoinsCollected(playerId, coins);
}

int GameplayManager::getPlayerTotalCoins(int playerId) { return coinsDao.getTotalCoins(playerId); }

bool GameplayManager::playerScored(int playerId, int score) { return scoresDao.saveScore(playerId, score); }

int GameplayManager::getPlayerHighestScore(int playerId) { return scoresDao.getHighestScore(playerId); }

// Other gameplay-related methods can be added here

// Example method to handle gameplay logic
void GameplayManager::handleGameplayEvent(int playerId, PlayerEventType eventType, int eventData) {
    switch (eventType) {
        case PlayerEventType::KILL:
            playerKilledZombie(playerId, eventData);
            break;
        case PlayerEventType::COLLECT:
            playerCollectedCoins(playerId, eventData);
            break;
        default:
            // Handle unsupported event type
            break;
    }
}