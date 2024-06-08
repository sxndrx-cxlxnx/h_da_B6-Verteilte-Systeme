namespace py Wettanbieter
namespace cpp Wettanbietercpp

service Wettanbieter {
    bool addBet(1: string nameanbieter, 2: i32 id, 3: string playera, 4: string playerb, 5: double winachance, 6: double winbchance, 7: double even),
    bool changeBet(1: string nameanbieter, 2: i32 id, 3: string playera, 4: string playerb, 5: double winachance, 6: double winbchance, 7: double even),
    bool deleteBet(1: string nameanbieter, 2: i32 id),

    bool betExists(1: string nameanbieter, 2: i32 betid),

    string getMyBetOptions(1: string nameanbieter),
    string getAllBetOptions(),
    string getSpecificBetOption(1: string nameanbieter, 2: i32 id),
    string getAllPlacedBets(),
    string getAllPlacedBetsToABetOption(1: string nameanbieter, 2: i32 id),

    bool placeNewBet(1: string nameanbieter, 2: i32 betid, 3: string namebettor, 4: double moneyset, 5: i32 ending, 6: double winachance, 7: double winbchance, 8: double even)
}