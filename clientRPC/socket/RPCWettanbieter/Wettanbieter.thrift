namespace php Wettanbieter
service Wettanbieter {
    bool addBet(1: i32 id, 2: string playera, 3: string playerb, 4: double winachance, 5: double winbchance, 6: double even),
    bool changeBet(1: i32 id, 2: string playera, 3: string playerb, 4: double winachance, 5: double winbchance, 6: double even),
    bool deleteBet(1: i32 id, 2: string playera, 3: string playerb, 4: double winachance, 5: double winbchance, 6: double even)
}