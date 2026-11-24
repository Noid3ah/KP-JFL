#pragma once
#include <string>
#include "exceptions.h"

class MatchResult {
private:
    int homeScore;
    int awayScore;

public:
    MatchResult(int homeScore, int awayScore)
        : homeScore(homeScore), awayScore(awayScore) {
        if (homeScore < 0 || awayScore < 0) {
            throw InvalidScoreException("Scores cannot be negative (" + std::to_string(homeScore)
                                        + "-" + std::to_string(awayScore) + ")");
        }
    }
    int getHomeScore() const { return homeScore; }
    int getAwayScore() const { return awayScore; }

    // Points system: win = 3, draw = 1, loss = 0.
    int getHomePoints() const {
        if (homeScore > awayScore) {
            return 3;
        }
        if (homeScore == awayScore) {
            return 1;
        }
        return 0;
    }

    int getAwayPoints() const {
        if (awayScore > homeScore) {
            return 3;
        }
        if (awayScore == homeScore) {
            return 1;
        }
        return 0;
    }
};
