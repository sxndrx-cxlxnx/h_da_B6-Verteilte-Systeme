namespace py Wettanbieter
namespace cpp Wettanbietercpp

service Wettanbieter {
    bool addBet(1: string nameanbieter, 2: i32 wettid, 3: i32 spielid, 4: double winachance, 5: double winbchance, 6: double even),
    bool changeBet(1: string nameanbieter, 2: i32 wettid, 3: i32 spielid, 4: double winachance, 5: double winbchance, 6: double even),
    bool deleteBet(1: string nameanbieter, 2: i32 id),

    bool betExists(1: string nameanbieter, 2: i32 betid),

    string getMyBetOptions(1: string nameanbieter),
    string getAllBetOptions(),
    string getSpecificBetOption(1: string nameanbieter, 2: i32 id),
    string getAllPlacedBets(),
    string getAllPlacedBetsToABetOption(1: string nameanbieter, 2: i32 id),

    bool placeNewBet(1: string nameanbieter, 2: i32 betid, 3: string namebettor, 4: double moneyset, 5: i32 ending, 6: double winachance, 7: double winbchance, 8: double even),

    bool addGame(1: i32 spielid, 2: string playera, 3: string playerb),
    bool endGame(1: i32 spielid, 2: i32 pointsA, 3: i32 pointsB),

    bool isGameOpen(1: i32 spielid),
    string getAllOpenGames(),
    string getAllClosedGames(),
    string getSpecificGame(1: i32 spielid),

    i32 getPossibleEarningBookholder(1: string nameanbieter, 2: i32 betid, 3: i32 ending),
    i32 getNumberOfBettors(1: string nameanbieter, 2: i32 betid),
    double getEarningBettor(1: string nameanbieter, 2: i32 betid, 3: string namebettor),
    i32 getEarningBookholder(1: string nameanbieter, 2: i32 betid),
    
    bool newRecommendation(1: i32 gameid, 2: double recquotea, 3: double recquoteb, 4: double recquoteeven),
    bool newDay(),
    string getAllRecommendations()
}