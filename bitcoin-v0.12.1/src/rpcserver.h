// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPCSERVER_H
#define BITCOIN_RPCSERVER_H

#include "amount.h"
#include "rpcprotocol.h"
#include "uint256.h"

#include <list>
#include <map>
#include <stdint.h>
#include <string>

#include <boost/function.hpp>

#include <univalue.h>

class CRPCCommand;

namespace RPCServer // RPC 服务
{
    void OnStarted(boost::function<void ()> slot);
    void OnStopped(boost::function<void ()> slot);
    void OnPreCommand(boost::function<void (const CRPCCommand&)> slot);
    void OnPostCommand(boost::function<void (const CRPCCommand&)> slot);
}

class CBlockIndex;
class CNetAddr;

class JSONRequest // JSON 请求类
{
public:
    UniValue id; // 请求的 id
    std::string strMethod; // 请求的方法
    UniValue params;

    JSONRequest() { id = NullUniValue; }
    void parse(const UniValue& valRequest); // 解析 JSON 请求
};

/** Query whether RPC is running */
bool IsRPCRunning(); // 查询 RPC 服务是否运行

/**
 * Set the RPC warmup status.  When this is done, all RPC calls will error out
 * immediately with RPC_IN_WARMUP.
 */ // 设置 RPC 预热新状态。当这步完成时，全部 RPC 调用将立刻使用 RPC_IN_WARMUP 错误输出。
void SetRPCWarmupStatus(const std::string& newStatus);
/* Mark warmup as done.  RPC calls will be processed from now on.  */
void SetRPCWarmupFinished(); // 标记预热完成，从现在开始处理 RPC 调用

/* returns the current warmup state.  */
bool RPCIsInWarmup(std::string *statusOut); // 获取当前 RPC 的预热状态

/**
 * Type-check arguments; throws JSONRPCError if wrong type given. Does not check that
 * the right number of arguments are passed, just that any passed are the correct type.
 * Use like:  RPCTypeCheck(params, boost::assign::list_of(str_type)(int_type)(obj_type));
 */ // 检查参数类型；如果给定错误类型，则抛出 JSONRPCError。不检查传递参数的个数，仅检查参数类型。
void RPCTypeCheck(const UniValue& params,
                  const std::list<UniValue::VType>& typesExpected, bool fAllowNull=false);

/*
  Check for expected keys/value types in an Object.
  Use like: RPCTypeCheckObj(object, boost::assign::map_list_of("name", str_type)("value", int_type));
*/
void RPCTypeCheckObj(const UniValue& o,
                  const std::map<std::string, UniValue::VType>& typesExpected, bool fAllowNull=false);

/** Opaque base class for timers returned by NewTimerFunc.
 * This provides no methods at the moment, but makes sure that delete
 * cleans up the whole state.
 */
class RPCTimerBase
{
public:
    virtual ~RPCTimerBase() {}
};

/**
 * RPC timer "driver".
 */ // RPC 定时器“驱动”。
class RPCTimerInterface // RPC 定时器接口类
{
public:
    virtual ~RPCTimerInterface() {}
    /** Implementation name */
    virtual const char *Name() = 0;
    /** Factory function for timers.
     * RPC will call the function to create a timer that will call func in *millis* milliseconds.
     * @note As the RPC mechanism is backend-neutral, it can use different implementations of timers.
     * This is needed to cope with the case in which there is no HTTP server, but
     * only GUI RPC console, and to break the dependency of pcserver on httprpc.
     */
    virtual RPCTimerBase* NewTimer(boost::function<void(void)>& func, int64_t millis) = 0;
};

/** Register factory function for timers */ // 注册定时器工厂函数
void RPCRegisterTimerInterface(RPCTimerInterface *iface);
/** Unregister factory function for timers */ // 解注册定时器工厂函数
void RPCUnregisterTimerInterface(RPCTimerInterface *iface);

/**
 * Run func nSeconds from now.
 * Overrides previous timer <name> (if any).
 */ // 从现在开始的 nSeconds 秒后运行该函数
void RPCRunLater(const std::string& name, boost::function<void(void)> func, int64_t nSeconds);

typedef UniValue(*rpcfn_type)(const UniValue& params, bool fHelp); // RPC 命令对应函数行为的回调函数

class CRPCCommand // RPC 命令类
{
public:
    std::string category; // 所属类别
    std::string name; // 名称
    rpcfn_type actor; // 对应的函数行为
    bool okSafeMode; // 是否开启安全模式
};

/**
 * Bitcoin RPC command dispatcher.
 */ // 比特币 RPC 命令调度器
class CRPCTable // RPC 列表类
{
private:
    std::map<std::string, const CRPCCommand*> mapCommands; // RPC 命令列表
public:
    CRPCTable(); // 注册所有定义的 RPC 命令到 RPC 命令列表
    const CRPCCommand* operator[](const std::string& name) const; // 重载的下标运算符
    std::string help(const std::string& name) const;

    /**
     * Execute a method.
     * @param method   Method to execute
     * @param params   UniValue Array of arguments (JSON objects)
     * @returns Result of the call.
     * @throws an exception (UniValue) when an error happens.
     */ // 执行一个方法
    UniValue execute(const std::string &method, const UniValue &params) const;
};

extern const CRPCTable tableRPC; // 在 rpcserver.cpp 中创建的一个全局的常量对象

/**
 * Utilities: convert hex-encoded Values
 * (throws error if not hex).
 */
extern uint256 ParseHashV(const UniValue& v, std::string strName);
extern uint256 ParseHashO(const UniValue& o, std::string strKey);
extern std::vector<unsigned char> ParseHexV(const UniValue& v, std::string strName);
extern std::vector<unsigned char> ParseHexO(const UniValue& o, std::string strKey);

extern int64_t nWalletUnlockTime;
extern CAmount AmountFromValue(const UniValue& value); // 从指定值获取金额，包含类型和范围检测
extern UniValue ValueFromAmount(const CAmount& amount);
extern double GetDifficulty(const CBlockIndex* blockindex = NULL);
extern std::string HelpRequiringPassphrase();
extern std::string HelpExampleCli(const std::string& methodname, const std::string& args);
extern std::string HelpExampleRpc(const std::string& methodname, const std::string& args);

extern void EnsureWalletIsUnlocked();

extern UniValue getconnectioncount(const UniValue& params, bool fHelp); // 获取当前的连接数
extern UniValue getpeerinfo(const UniValue& params, bool fHelp); // 获取同辈节点信息
extern UniValue ping(const UniValue& params, bool fHelp); // ping 命令在 getpeerinfo 结果的 pingtime 字段查看
extern UniValue addnode(const UniValue& params, bool fHelp); // 添加节点
extern UniValue disconnectnode(const UniValue& params, bool fHelp); // 断开与指定节点的连接
extern UniValue getaddednodeinfo(const UniValue& params, bool fHelp); // 获取添加节点的信息
extern UniValue getnettotals(const UniValue& params, bool fHelp); // 获取网络流量信息
extern UniValue setban(const UniValue& params, bool fHelp); // 设置黑名单
extern UniValue listbanned(const UniValue& params, bool fHelp); // 列出黑名单
extern UniValue clearbanned(const UniValue& params, bool fHelp); // 清空黑名单

extern UniValue dumpprivkey(const UniValue& params, bool fHelp); // 导出私钥
extern UniValue importprivkey(const UniValue& params, bool fHelp); // 导入私钥
extern UniValue importaddress(const UniValue& params, bool fHelp); // 导入地址或脚本
extern UniValue importpubkey(const UniValue& params, bool fHelp); // 导入公钥
extern UniValue dumpwallet(const UniValue& params, bool fHelp); // 导出钱包
extern UniValue importwallet(const UniValue& params, bool fHelp); // 导入钱包

extern UniValue getgenerate(const UniValue& params, bool fHelp); // 获取挖矿状态
extern UniValue setgenerate(const UniValue& params, bool fHelp); // 设置挖矿状态，挖矿开关
extern UniValue generate(const UniValue& params, bool fHelp); // 生成指定数目个区块（回归测试网用）
extern UniValue getnetworkhashps(const UniValue& params, bool fHelp); // 获取全网算力
extern UniValue getmininginfo(const UniValue& params, bool fHelp); // 获取挖矿信息
extern UniValue prioritisetransaction(const UniValue& params, bool fHelp); // 设置交易的优先级
extern UniValue getblocktemplate(const UniValue& params, bool fHelp); // 获取区块模板
extern UniValue submitblock(const UniValue& params, bool fHelp); // 提交区块
extern UniValue estimatefee(const UniValue& params, bool fHelp); // 预估交易费
extern UniValue estimatepriority(const UniValue& params, bool fHelp); // 预估交易优先级
extern UniValue estimatesmartfee(const UniValue& params, bool fHelp); // 智能估计交易费
extern UniValue estimatesmartpriority(const UniValue& params, bool fHelp); // 智能估计交易优先级

extern UniValue getnewaddress(const UniValue& params, bool fHelp); // 获取新地址
extern UniValue getaccountaddress(const UniValue& params, bool fHelp); // 获取账户收款地址
extern UniValue getrawchangeaddress(const UniValue& params, bool fHelp); // 获取元交易找零地址
extern UniValue setaccount(const UniValue& params, bool fHelp); // 设置地址关联账户
extern UniValue getaccount(const UniValue& params, bool fHelp); // 获取地址所属账户
extern UniValue getaddressesbyaccount(const UniValue& params, bool fHelp); // 获取账户下的所有地址
extern UniValue sendtoaddress(const UniValue& params, bool fHelp); // 发送比特币到指定地址
extern UniValue signmessage(const UniValue& params, bool fHelp); // 签名消息
extern UniValue verifymessage(const UniValue& params, bool fHelp); // 验证签名消息
extern UniValue getreceivedbyaddress(const UniValue& params, bool fHelp); // 获取某地址接收到的金额
extern UniValue getreceivedbyaccount(const UniValue& params, bool fHelp); // 获取某账户接收到的金额
extern UniValue getbalance(const UniValue& params, bool fHelp); // 获取余额
extern UniValue getunconfirmedbalance(const UniValue& params, bool fHelp); // 获取未确认的余额
extern UniValue movecmd(const UniValue& params, bool fHelp); // 账户间转移资金
extern UniValue sendfrom(const UniValue& params, bool fHelp); // 从指定账户发送金额
extern UniValue sendmany(const UniValue& params, bool fHelp); // 发送金额到多个地址
extern UniValue addmultisigaddress(const UniValue& params, bool fHelp); // 添加多重签名地址
extern UniValue createmultisig(const UniValue& params, bool fHelp); // 创建多重签名
extern UniValue listreceivedbyaddress(const UniValue& params, bool fHelp); // 列出地址余额
extern UniValue listreceivedbyaccount(const UniValue& params, bool fHelp); // 列出账户余额
extern UniValue listtransactions(const UniValue& params, bool fHelp); // 列出最近的交易信息
extern UniValue listaddressgroupings(const UniValue& params, bool fHelp); // 列出地址分组
extern UniValue listaccounts(const UniValue& params, bool fHelp); // 列出账户及其余额
extern UniValue listsinceblock(const UniValue& params, bool fHelp); // 列出指定区块开始区块上的全部交易
extern UniValue gettransaction(const UniValue& params, bool fHelp); // 获取交易详细信息
extern UniValue abandontransaction(const UniValue& params, bool fHelp); // 抛弃钱包内的交易
extern UniValue backupwallet(const UniValue& params, bool fHelp); // 备份钱包
extern UniValue keypoolrefill(const UniValue& params, bool fHelp); // 再填充密钥池
extern UniValue walletpassphrase(const UniValue& params, bool fHelp); // 钱包解锁
extern UniValue walletpassphrasechange(const UniValue& params, bool fHelp); // 修改钱包密码
extern UniValue walletlock(const UniValue& params, bool fHelp); // 锁定钱包
extern UniValue encryptwallet(const UniValue& params, bool fHelp); // 加密钱包
extern UniValue validateaddress(const UniValue& params, bool fHelp); // 验证地址
extern UniValue getinfo(const UniValue& params, bool fHelp); // 获取比特币核心信息
extern UniValue getwalletinfo(const UniValue& params, bool fHelp); // 获取钱包信息
extern UniValue getblockchaininfo(const UniValue& params, bool fHelp); // 获取区块链信息
extern UniValue getnetworkinfo(const UniValue& params, bool fHelp); // 获取网络状态信息
extern UniValue setmocktime(const UniValue& params, bool fHelp); // 设置 mocktime
extern UniValue resendwallettransactions(const UniValue& params, bool fHelp); // 重新发送钱包交易

extern UniValue getrawtransaction(const UniValue& params, bool fHelp); // 获取原始交易信息
extern UniValue listunspent(const UniValue& params, bool fHelp); // 列出未花费的交易输出
extern UniValue lockunspent(const UniValue& params, bool fHelp); // 加解锁未花费的交易输出
extern UniValue listlockunspent(const UniValue& params, bool fHelp); // 列出锁定的未花费交易输出
extern UniValue createrawtransaction(const UniValue& params, bool fHelp); // 创建原始交易
extern UniValue decoderawtransaction(const UniValue& params, bool fHelp); // 解码原始交易
extern UniValue decodescript(const UniValue& params, bool fHelp); // 解码脚本
extern UniValue fundrawtransaction(const UniValue& params, bool fHelp); // 资助原始交易
extern UniValue signrawtransaction(const UniValue& params, bool fHelp); // 签名原始交易
extern UniValue sendrawtransaction(const UniValue& params, bool fHelp); // 发送原始交易
extern UniValue gettxoutproof(const UniValue& params, bool fHelp); // 获取交易证明
extern UniValue verifytxoutproof(const UniValue& params, bool fHelp); // 验证交易证明

extern UniValue getblockcount(const UniValue& params, bool fHelp); // 获取当前区块总数
extern UniValue getbestblockhash(const UniValue& params, bool fHelp); // 获取当前最佳块的哈希
extern UniValue getdifficulty(const UniValue& params, bool fHelp); // 获取当前挖矿难度
extern UniValue settxfee(const UniValue& params, bool fHelp); // 设置交易费
extern UniValue getmempoolinfo(const UniValue& params, bool fHelp); // 获取交易内存池信息
extern UniValue getrawmempool(const UniValue& params, bool fHelp); // 获取交易内存池元信息（交易索引）
extern UniValue getblockhash(const UniValue& params, bool fHelp); // 获取指定区块索引的区块哈希
extern UniValue getblockheader(const UniValue& params, bool fHelp); // 获取指定区块哈希的区块头信息
extern UniValue getblock(const UniValue& params, bool fHelp); // 获取区块信息
extern UniValue gettxoutsetinfo(const UniValue& params, bool fHelp); // 获取交易输出集合信息
extern UniValue gettxout(const UniValue& params, bool fHelp); // 获取一笔交易输出（链上或内存池中）的细节
extern UniValue verifychain(const UniValue& params, bool fHelp); // 验证区块链数据库
extern UniValue getchaintips(const UniValue& params, bool fHelp); // 获取链尖信息
extern UniValue invalidateblock(const UniValue& params, bool fHelp); // 无效化区块
extern UniValue reconsiderblock(const UniValue& params, bool fHelp); // 再考虑区块

bool StartRPC(); // 启动 RPC
void InterruptRPC();
void StopRPC(); // 停止 RPC
std::string JSONRPCExecBatch(const UniValue& vReq); // JSONRPC 批量执行

#endif // BITCOIN_RPCSERVER_H
