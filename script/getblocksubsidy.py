#!/usr/bin/python3

def GetBlockSubsidy(nHeight, nSubsidyHalvingInterval):
    halvings = nHeight // nSubsidyHalvingInterval;
    if halvings >= 64:
        return 0
    nSubsidy = 50 * 10**8
    nSubsidy = nSubsidy >> halvings
    return nSubsidy

if __name__ == "__main__":
    blocks = 210000
    print("And now,block subsidy: " + str(GetBlockSubsidy(blocks, 210000)))
