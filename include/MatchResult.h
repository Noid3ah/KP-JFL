#pragma once
#include <string>
#include "exceptions.h"

using namespace std;

// holds the final score for a match and works out points — win 3, draw 1, loss 0
class MatchResult {
private:
    int homeScore;
    int awayScore;

public:
    MatchResult(int homeScore, int awayScore)
        : homeScore(homeScore), awayScore(awayScore) {
        if (homeScore < 0 || awayScore < 0) {
            throw InvalidScoreException("Scores cannot be negative (" + to_string(homeScore)
                                        + "-" + to_string(awayScore) + ")");
        }
    }

    int getHomeScore() const { return homeScore; }
    int getAwayScore() const { return awayScore; }

    // standard league points from the home team's perspective
    int getHomePoints() const {
        if (homeScore > awayScore) return 3;
        if (homeScore == awayScore) return 1;
        return 0;
    }

    int getAwayPoints() const {
        if (awayScore > homeScore) return 3;
        if (awayScore == homeScore) return 1;
        return 0;
    }
};
