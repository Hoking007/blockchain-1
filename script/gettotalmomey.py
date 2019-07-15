#!/usr/bin/python
#coding=utf-8

start_block_reward = 50

reward_interval = 210000

total_block = 0

def GetTotalMoney():
    # 1 BTC = 1 0000 0000 Satoshis
    current_reward = 50 * 10**8
    total_money = 0
    global total_block # 需加 global 关键字进行声明，才能使用
    while current_reward > 0:
        total_money += reward_interval * current_reward
        current_reward /= 2
        total_block += reward_interval
    return total_money

if __name__ == "__main__":
    print "Total BTC to ever be created: ", GetTotalMoney(), " Satoshis" # python: 2099999997690000
    print "Total block to ever be created: ", total_block, " blocks"
    #print("Total BTC to ever be created: ", max_money(), " Satoshis") # python3: 2100000000000000.0
