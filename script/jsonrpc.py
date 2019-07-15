#!/usr/bin/python3

import json
import requests
import time

def jsonrpc(method, params):
    payload = json.dumps({"method": method, "params": params})
    response = requests.request("POST", url, data=payload, auth=auth)
    return response.json()

def getinfo():
    return jsonrpc("getinfo", [])

def getbestblockinfo():
    ret = jsonrpc("getbestblockhash", [])
    bestblockhash = ret["result"]
    return jsonrpc("getblock", [bestblockhash])

def getdifficulty():
    return jsonrpc("getdifficulty", [])

def getnetworkhashps():
    return jsonrpc("getnetworkhashps", [])

def RPCFunc():
    print("----------------------------------------")
    print("info:\n" + str(getinfo()))
    print("\nbestblockinfo:\n" + str(getbestblockinfo()))
    print("\ndifficulty:\n" + str(getdifficulty()))
    print("\nnetworkhashps:\n" + str(getnetworkhashps()))

if __name__ == "__main__":
    url = "http://127.0.0.1:8332"
    auth=("rpcuser", "rpcpassword")
    while (1):
        RPCFunc()
        time.sleep(7)
