#!/usr/bin/python3

import sys

reward_interval = 210000

def GetCurrentCurrency(total_block):
    # 1 BTC = 1 0000 0000 Satoshis
    current_reward = 50 * 10**8
    total_money = 0
    while total_block > 0:
        if total_block > reward_interval:
            total_money += reward_interval * current_reward
        else:
            total_money += total_block * current_reward
        current_reward /= 2
        total_block -= reward_interval
    return total_money

if __name__ == "__main__":
    blocks = sys.argv[1]
    print("current currency: ", GetCurrentCurrency(int(blocks)))
