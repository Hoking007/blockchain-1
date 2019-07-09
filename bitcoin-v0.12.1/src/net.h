// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include "bloom.h"
#include "compat.h"
#include "limitedmap.h"
#include "netbase.h"
#include "protocol.h"
#include "random.h"
#include "streams.h"
#include "sync.h"
#include "uint256.h"

#include <deque>
#include <stdint.h>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/signals2/signal.hpp>

class CAddrMan;
class CScheduler;
class CNode;

namespace boost {
    class thread_group;
} // namespace boost

/** Time between pings automatically sent out for latency probing and keepalive (in seconds). */
static const int PING_INTERVAL = 2 * 60;
/** Time after which to disconnect, after waiting for a ping response (or inactivity). */
static const int TIMEOUT_INTERVAL = 20 * 60;
/** The maximum number of entries in an 'inv' protocol message */
static const unsigned int MAX_INV_SZ = 50000; // һ�� 'inv' Э����Ϣ����Ŀ��������ֵ
/** The maximum number of new addresses to accumulate before announcing. */
static const unsigned int MAX_ADDR_TO_SEND = 1000;
/** Maximum length of incoming protocol messages (no message over 2 MiB is currently acceptable). */
static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 2 * 1024 * 1024;
/** Maximum length of strSubVer in `version` message */
static const unsigned int MAX_SUBVERSION_LENGTH = 256;
/** -listen default */ // -listen ����Ĭ��ֵ
static const bool DEFAULT_LISTEN = true; // Ĭ�Ͽ���
/** -upnp default */
#ifdef USE_UPNP
static const bool DEFAULT_UPNP = USE_UPNP;
#else
static const bool DEFAULT_UPNP = false;
#endif
/** The maximum number of entries in mapAskFor */
static const size_t MAPASKFOR_MAX_SZ = MAX_INV_SZ;
/** The maximum number of entries in setAskFor (larger due to getdata latency)*/
static const size_t SETASKFOR_MAX_SZ = 2 * MAX_INV_SZ;
/** The maximum number of peer connections to maintain. */ // Ҫά�������Զ�������
static const unsigned int DEFAULT_MAX_PEER_CONNECTIONS = 125;
/** The default for -maxuploadtarget. 0 = Unlimited */ // �� -maxuploadtarget ����Ĭ��ֵ��0 = �����Ƶ�
static const uint64_t DEFAULT_MAX_UPLOAD_TARGET = 0;
/** Default for blocks only*/ // Ĭ�Ͻ�����
static const bool DEFAULT_BLOCKSONLY = false;

static const bool DEFAULT_FORCEDNSSEED = false;
static const size_t DEFAULT_MAXRECEIVEBUFFER = 5 * 1000;
static const size_t DEFAULT_MAXSENDBUFFER    = 1 * 1000;

// NOTE: When adjusting this, update rpcnet:setban's help ("24h") // ������������� rpcnet:setban �İ�����Ϣ��"24h"��
static const unsigned int DEFAULT_MISBEHAVING_BANTIME = 60 * 60 * 24;  // Default 24-hour ban

unsigned int ReceiveFloodSize(); // ��ȡ���ջ�������ֵ
unsigned int SendBufferSize(); // ��ȡ���ͻ�������ֵ

void AddOneShot(const std::string& strDest); // ���ӵ�˫�˶��� vOneShots
void AddressCurrentlyConnected(const CService& addr);
CNode* FindNode(const CNetAddr& ip);
CNode* FindNode(const CSubNet& subNet);
CNode* FindNode(const std::string& addrName);
CNode* FindNode(const CService& ip);
CNode* ConnectNode(CAddress addrConnect, const char *pszDest = NULL);
bool OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound = NULL, const char *strDest = NULL, bool fOneShot = false); // ����������
void MapPort(bool fUseUPnP);
unsigned short GetListenPort(); // ��ȡ�����˿�
bool BindListenPort(const CService &bindAddr, std::string& strError, bool fWhitelisted = false); // �󶨲������˿�
void StartNode(boost::thread_group& threadGroup, CScheduler& scheduler); // ���������߳�
bool StopNode(); // ֹͣ�������߳�
void SocketSendData(CNode *pnode); // ͨ���׽��ַ�������

typedef int NodeId;

struct CombinerAll
{
    typedef bool result_type;

    template<typename I>
    bool operator()(I first, I last) const
    {
        while (first != last) {
            if (!(*first)) return false;
            ++first;
        }
        return true;
    }
};

// Signals for message handling // ���ڴ�����Ϣ���ź�
struct CNodeSignals
{
    boost::signals2::signal<int ()> GetHeight;
    boost::signals2::signal<bool (CNode*), CombinerAll> ProcessMessages;
    boost::signals2::signal<bool (CNode*), CombinerAll> SendMessages;
    boost::signals2::signal<void (NodeId, const CNode*)> InitializeNode;
    boost::signals2::signal<void (NodeId)> FinalizeNode;
};


CNodeSignals& GetNodeSignals(); // ��ȡ�ڵ��ź�ȫ�ֶ��������


enum
{
    LOCAL_NONE,   // unknown // δ֪ 0
    LOCAL_IF,     // address a local interface listens on // ���ؽӿ�������ַ 1
    LOCAL_BIND,   // address explicit bound to // ��ʾ�󶨵��ĵ�ַ 2
    LOCAL_UPNP,   // address reported by UPnP // UPnP ����ĵ�ַ 3
    LOCAL_MANUAL, // address explicitly specified (-externalip=) // ��ʾָ���ĵ�ַ��-externalip=��4

    LOCAL_MAX // 5
};

bool IsPeerAddrLocalGood(CNode *pnode);
void AdvertizeLocal(CNode *pnode); // ��汾�ص�ַ���Զ�
void SetLimited(enum Network net, bool fLimited = true); // ����������������
bool IsLimited(enum Network net);
bool IsLimited(const CNetAddr& addr);
bool AddLocal(const CService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& addr, int nScore = LOCAL_NONE);
bool RemoveLocal(const CService& addr);
bool SeenLocal(const CService& addr);
bool IsLocal(const CService& addr);
bool GetLocal(CService &addr, const CNetAddr *paddrPeer = NULL);
bool IsReachable(enum Network net);
bool IsReachable(const CNetAddr &addr);
void SetReachable(enum Network net, bool fFlag = true); // ��������ɴ�
CAddress GetLocalAddress(const CNetAddr *paddrPeer = NULL);


extern bool fDiscover;
extern bool fListen;
extern uint64_t nLocalServices;
extern uint64_t nLocalHostNonce;
extern CAddrMan addrman;

/** Maximum number of connections to simultaneously allow (aka connection slots) */
extern int nMaxConnections; // ͬʱ�����������������Ҳ�����Ӳۣ�

extern std::vector<CNode*> vNodes; // �ѽ������ӵĽڵ��б�
extern CCriticalSection cs_vNodes; // �ڵ��б���
extern std::map<CInv, CDataStream> mapRelay; // �м�ӳ���б�
extern std::deque<std::pair<int64_t, CInv> > vRelayExpiration; // �м̹��ڶ���
extern CCriticalSection cs_mapRelay; // �м�ӳ���б���
extern limitedmap<CInv, int64_t> mapAlreadyAskedFor;

extern std::vector<std::string> vAddedNodes; // ���ӵĽڵ��б�
extern CCriticalSection cs_vAddedNodes;

extern NodeId nLastNodeId;
extern CCriticalSection cs_nLastNodeId;

/** Subversion as sent to the P2P network in `version` messages */
extern std::string strSubVersion; // Subversion �� `version` ��Ϣ�з��͵� P2P ����

struct LocalServiceInfo { // ���ط�����Ϣ�ṹ��
    int nScore; // ������
    int nPort; // �˿�
};

extern CCriticalSection cs_mapLocalHost;
extern std::map<CNetAddr, LocalServiceInfo> mapLocalHost;

class CNodeStats // �ڵ�״̬��
{
public:
    NodeId nodeid;
    uint64_t nServices;
    bool fRelayTxes; // �м̽��ױ�־
    int64_t nLastSend; // ���һ�η���ʱ��
    int64_t nLastRecv; // ���һ�ν���ʱ��
    int64_t nTimeConnected;
    int64_t nTimeOffset;
    std::string addrName;
    int nVersion;
    std::string cleanSubVer;
    bool fInbound; // �����־��false ��ʾ����
    int nStartingHeight;
    uint64_t nSendBytes;
    uint64_t nRecvBytes;
    bool fWhitelisted;
    double dPingTime;
    double dPingWait;
    double dPingMin;
    std::string addrLocal;
};




class CNetMessage { // ������Ϣ��
public:
    bool in_data;                   // parsing header (false) or data (true) // ��ʾ��ǰ������ͷ������

    CDataStream hdrbuf;             // partially received header // ���յĲ�����Ϣͷ
    CMessageHeader hdr;             // complete header // ��������Ϣͷ
    unsigned int nHdrPos; // ��¼��Ϣͷ��ǰ���ݵ�λ��

    CDataStream vRecv;              // received message data // ���յ���Ϣ����
    unsigned int nDataPos; // ��¼��Ϣ�嵱ǰ���ݵ�λ��

    int64_t nTime;                  // time (in microseconds) of message receipt. // ��Ϣ����ʱ�䣨��΢��Ϊ��λ��

    CNetMessage(const CMessageHeader::MessageStartChars& pchMessageStartIn, int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), hdr(pchMessageStartIn), vRecv(nTypeIn, nVersionIn) {
        hdrbuf.resize(24); // Ԥ���� 24 �ֽڵ���Ϣͷ
        in_data = false;
        nHdrPos = 0;
        nDataPos = 0;
        nTime = 0;
    }

    bool complete() const // ���������Ϣ�Ƿ�����
    {
        if (!in_data)
            return false;
        return (hdr.nMessageSize == nDataPos);
    }

    void SetVersion(int nVersionIn)
    {
        hdrbuf.SetVersion(nVersionIn);
        vRecv.SetVersion(nVersionIn);
    }

    int readHeader(const char *pch, unsigned int nBytes); // ����Ϣͷ
    int readData(const char *pch, unsigned int nBytes); // ����Ϣ�壨���ݣ�
};


typedef enum BanReason // ��ֹԭ��ö��
{
    BanReasonUnknown          = 0, // δ֪ԭ��
    BanReasonNodeMisbehaving  = 1, // ������Ϊ
    BanReasonManuallyAdded    = 2 // �ֶ�����
} BanReason;

class CBanEntry // ��ֹ��Ŀ��
{
public:
    static const int CURRENT_VERSION=1; // ��ǰ�汾��
    int nVersion; // �汾��
    int64_t nCreateTime; // ������ֹʱ��
    int64_t nBanUntil; // ��ֹ����ʱ��
    uint8_t banReason; // ��ֹԭ��

    CBanEntry()
    {
        SetNull();
    }

    CBanEntry(int64_t nCreateTimeIn)
    {
        SetNull();
        nCreateTime = nCreateTimeIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
        READWRITE(nBanUntil);
        READWRITE(banReason);
    }

    void SetNull()
    {
        nVersion = CBanEntry::CURRENT_VERSION;
        nCreateTime = 0;
        nBanUntil = 0;
        banReason = BanReasonUnknown;
    }

    std::string banReasonToString()
    {
        switch (banReason) {
        case BanReasonNodeMisbehaving:
            return "node misbehaving";
        case BanReasonManuallyAdded:
            return "manually added";
        default:
            return "unknown";
        }
    }
};

typedef std::map<CSubNet, CBanEntry> banmap_t; // ��ֹ�б����������ֹ��Ŀ��ӳ��

/** Information about a peer */ // ���ڶԶ˽ڵ����Ϣ
class CNode // �Զ˽ڵ���Ϣ��
{
public:
    // socket
    uint64_t nServices;
    SOCKET hSocket; // �׽���
    CDataStream ssSend; // ����������
    size_t nSendSize; // total size of all vSendMsg entries
    size_t nSendOffset; // offset inside the first vSendMsg already sent
    uint64_t nSendBytes;
    std::deque<CSerializeData> vSendMsg;
    CCriticalSection cs_vSend;

    std::deque<CInv> vRecvGetData; // ���ջ�ȡ���� inv ����
    std::deque<CNetMessage> vRecvMsg; // ���յ�������Ϣ����
    CCriticalSection cs_vRecvMsg;
    uint64_t nRecvBytes;
    int nRecvVersion;

    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    int64_t nTimeOffset;
    CAddress addr; // �ڵ��ַ
    std::string addrName; // �ڵ�� IP
    CService addrLocal;
    int nVersion; // �ڵ�汾
    // strSubVer is whatever byte array we read from the wire. However, this field is intended
    // to be printed out, displayed to humans in various forms and so on. So we sanitize it and
    // store the sanitized version in cleanSubVer. The original should be used when dealing with
    // the network or wire types and the cleaned string used when displayed or logged.
    std::string strSubVer, cleanSubVer;
    bool fWhitelisted; // This peer can bypass DoS banning. // �����������־����ʾ��ͬ������ƹ� Dos ��ֹ
    bool fOneShot;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    bool fDisconnect;
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in its version message that we should not relay tx invs
    //    unless it loads a bloom filter.
    bool fRelayTxes;
    CSemaphoreGrant grantOutbound;
    CCriticalSection cs_filter;
    CBloomFilter* pfilter;
    int nRefCount; // �ڵ�����ü���
    NodeId id; // �������ӽڵ�����
protected:

    // Denial-of-service detection/prevention
    // Key is IP address, value is banned-until-time
    static banmap_t setBanned;
    static CCriticalSection cs_setBanned;
    static bool setBannedIsDirty;

    // Whitelisted ranges. Any node connecting from these is automatically // ��������Χ������Щ�ڵ����ӵ��κνڵ㶼���Զ����������
    // whitelisted (as well as those connecting to whitelisted binds). // �������ӵ��������󶨵Ľڵ㣩
    static std::vector<CSubNet> vWhitelistedRange;
    static CCriticalSection cs_vWhitelistedRange;

    // Basic fuzz-testing
    void Fuzz(int nChance); // modifies ssSend

public:
    uint256 hashContinue;
    int nStartingHeight;

    // flood relay // ���м�
    std::vector<CAddress> vAddrToSend; // �����͵ĵ�ַ�б�
    CRollingBloomFilter addrKnown; // ��֪�ĵ�ַ������
    bool fGetAddr; // ��ȡ��ַ��־
    std::set<uint256> setKnown;
    int64_t nNextAddrSend;
    int64_t nNextLocalAddrSend;

    // inventory based relay // �����м̵Ŀ������
    CRollingBloomFilter filterInventoryKnown; // ��³ķ������
    std::vector<CInv> vInventoryToSend; // �����Ϳ���б�
    CCriticalSection cs_inventory;
    std::set<uint256> setAskFor; // �������б�
    std::multimap<int64_t, CInv> mapAskFor; // ������ӳ���б� <ʱ�䣬�����Ŀ>
    int64_t nNextInvSend;
    // Used for headers announcements - unfiltered blocks to relay // ��������ͷͨ�� - �����м̵�δ��������
    // Also protected by cs_inventory // ͨ�����������
    std::vector<uint256> vBlockHashesToAnnounce; // ��֪ͨ�������ϣ�б�

    // Ping time measurement: // ping ʱ�������
    // The pong reply we're expecting, or 0 if no pong expected.
    uint64_t nPingNonceSent; // ����Ԥ�Ƶ� pong ��Ӧʱ�䣬���Ԥ���� pong ��Ϊ 0��
    // Time (in usec) the last ping was sent, or 0 if no ping was ever sent.
    int64_t nPingUsecStart; // �������һ�� ping ��ʱ�䣬���δ������ ping ��Ϊ 0��
    // Last measured round-trip time.
    int64_t nPingUsecTime;
    // Best measured round-trip time.
    int64_t nMinPingUsecTime;
    // Whether a ping is requested.
    bool fPingQueued; // �Ƿ�����һ�� ping

    CNode(SOCKET hSocketIn, const CAddress &addrIn, const std::string &addrNameIn = "", bool fInboundIn = false);
    ~CNode();

private:
    // Network usage totals
    static CCriticalSection cs_totalBytesRecv;
    static CCriticalSection cs_totalBytesSent;
    static uint64_t nTotalBytesRecv;
    static uint64_t nTotalBytesSent;

    // outbound limit & stats
    static uint64_t nMaxOutboundTotalBytesSentInCycle;
    static uint64_t nMaxOutboundCycleStartTime;
    static uint64_t nMaxOutboundLimit;
    static uint64_t nMaxOutboundTimeframe;

    CNode(const CNode&);
    void operator=(const CNode&);

public:

    NodeId GetId() const {
      return id;
    }

    int GetRefCount()
    {
        assert(nRefCount >= 0);
        return nRefCount;
    }

    // requires LOCK(cs_vRecvMsg)
    unsigned int GetTotalRecvSize()
    {
        unsigned int total = 0;
        BOOST_FOREACH(const CNetMessage &msg, vRecvMsg)
            total += msg.vRecv.size() + 24;
        return total;
    }

    // requires LOCK(cs_vRecvMsg)
    bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);

    // requires LOCK(cs_vRecvMsg)
    void SetRecvVersion(int nVersionIn)
    {
        nRecvVersion = nVersionIn;
        BOOST_FOREACH(CNetMessage &msg, vRecvMsg)
            msg.SetVersion(nVersionIn);
    }

    CNode* AddRef() // ���ü����� 1
    {
        nRefCount++;
        return this;
    }

    void Release() // ���ü����� 1
    {
        nRefCount--;
    }



    void AddAddressKnown(const CAddress& addr)
    {
        addrKnown.insert(addr.GetKey());
    }

    void PushAddress(const CAddress& addr)
    {
        // Known checking here is only to save space from duplicates.
        // SendMessages will filter it again for knowns that were added
        // after addresses were pushed.
        if (addr.IsValid() && !addrKnown.contains(addr.GetKey())) {
            if (vAddrToSend.size() >= MAX_ADDR_TO_SEND) {
                vAddrToSend[insecure_rand() % vAddrToSend.size()] = addr;
            } else {
                vAddrToSend.push_back(addr);
            }
        }
    }


    void AddInventoryKnown(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            filterInventoryKnown.insert(inv.hash);
        }
    }

    void PushInventory(const CInv& inv)
    {
        {
            LOCK(cs_inventory); // �������
            if (inv.type == MSG_TX && filterInventoryKnown.contains(inv.hash)) // ��Ϊ�������� �� ��³ķ�����������˸ý������� inv �Ĺ�ϣ
                return; // ɶҲ����ֱ�ӷ���
            vInventoryToSend.push_back(inv); // ������뷢�Ϳ���б�
        }
    }

    void PushBlockHash(const uint256 &hash)
    {
        LOCK(cs_inventory); // �������
        vBlockHashesToAnnounce.push_back(hash); // ���������ϣ�����б�
    }

    void AskFor(const CInv& inv);

    // TODO: Document the postcondition of this function.  Is cs_vSend locked?
    void BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(cs_vSend); // ��ʼ����Ϣͷ�����뷢��������

    // TODO: Document the precondition of this function.  Is cs_vSend locked?
    void AbortMessage() UNLOCK_FUNCTION(cs_vSend);

    // TODO: Document the precondition of this function.  Is cs_vSend locked?
    void EndMessage() UNLOCK_FUNCTION(cs_vSend);

    void PushVersion(); // ���Ͱ汾


    void PushMessage(const char* pszCommand) // ������Ϣ
    {
        try
        {
            BeginMessage(pszCommand); // ������Ϣͷ�����뷢��������
            EndMessage(); // ������Ϣ�壬��������Ϣ
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1>
    void PushMessage(const char* pszCommand, const T1& a1)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
    {
        try
        {
            BeginMessage(pszCommand); // ��ʼ����Ϣͷ���� 24 bytes��
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9; // ��Ϣ�壨x bytes��
            EndMessage(); // �����Ϣ���ݴ�С��У��ͣ�����Ϣ���뷢����Ϣ���� vSendMsg��Ȼ����� SocketSendData ������Ϣ
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    void CloseSocketDisconnect();

    // Denial-of-service detection/prevention
    // The idea is to detect peers that are behaving
    // badly and disconnect/ban them, but do it in a
    // one-coding-mistake-won't-shatter-the-entire-network
    // way.
    // IMPORTANT:  There should be nothing I can give a
    // node that it will forward on that will make that
    // node's peers drop it. If there is, an attacker
    // can isolate a node and/or try to split the network.
    // Dropping a node for sending stuff that is invalid
    // now but might be valid in a later version is also
    // dangerous, because it can cause a network split
    // between nodes running old code and nodes running
    // new code.
    static void ClearBanned(); // needed for unit testing
    static bool IsBanned(CNetAddr ip);
    static bool IsBanned(CSubNet subnet);
    static void Ban(const CNetAddr &ip, const BanReason &banReason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false); // ת�������������������غ���
    static void Ban(const CSubNet &subNet, const BanReason &banReason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false); // ������������ֹ�б���
    static bool Unban(const CNetAddr &ip); // ��������Ľ�����������غ���
    static bool Unban(const CSubNet &ip);
    static void GetBanned(banmap_t &banmap);
    static void SetBanned(const banmap_t &banmap);

    //!check is the banlist has unwritten changes
    static bool BannedSetIsDirty();
    //!set the "dirty" flag for the banlist
    static void SetBannedSetDirty(bool dirty=true);
    //!clean unused entries (if bantime has expired)
    static void SweepBanned(); // ������õ���Ŀ������ֹʱ���ѹ��ڣ�

    void copyStats(CNodeStats &stats);

    static bool IsWhitelistedRange(const CNetAddr &ip);
    static void AddWhitelistedRange(const CSubNet &subnet); // ����������������

    // Network stats
    static void RecordBytesRecv(uint64_t bytes);
    static void RecordBytesSent(uint64_t bytes);

    static uint64_t GetTotalBytesRecv();
    static uint64_t GetTotalBytesSent();

    //!set the max outbound target in bytes // ���ֽ�Ϊ��λ�����������Ŀ��ֵ������������ֵ��
    static void SetMaxOutboundTarget(uint64_t limit);
    static uint64_t GetMaxOutboundTarget();

    //!set the timeframe for the max outbound target
    static void SetMaxOutboundTimeframe(uint64_t timeframe);
    static uint64_t GetMaxOutboundTimeframe();

    //!check if the outbound target is reached
    // if param historicalBlockServingLimit is set true, the function will
    // response true if the limit for serving historical blocks has been reached
    static bool OutboundTargetReached(bool historicalBlockServingLimit);

    //!response the bytes left in the current max outbound cycle
    // in case of no limit, it will always response 0
    static uint64_t GetOutboundTargetBytesLeft();

    //!response the time in second left in the current max outbound cycle
    // in case of no limit, it will always response 0
    static uint64_t GetMaxOutboundTimeLeftInCycle();
};



class CTransaction;
void RelayTransaction(const CTransaction& tx); // ת���������غ���
void RelayTransaction(const CTransaction& tx, const CDataStream& ss); // �м̽���

/** Access to the (IP) address database (peers.dat) */
class CAddrDB // IP ��ַ���ݿ⣨���ڱ��� peers.dat �м�¼�� IP��
{
private:
    boost::filesystem::path pathAddr; // ���� peers.dat �����·��
public:
    CAddrDB(); // ·��ƴ�ӣ�����Ŀ¼ + "peers.dat"
    bool Write(const CAddrMan& addr);
    bool Read(CAddrMan& addr);
};

/** Access to the banlist database (banlist.dat) */
class CBanDB // ���ʽ�ֹ�б����ݿ⣨banlist.dat��
{
private:
    boost::filesystem::path pathBanlist; // �������ݿ��ļ�·��
public:
    CBanDB(); // ·��ƴ�ӣ�����Ŀ¼ + "banlist.dat"
    bool Write(const banmap_t& banSet);
    bool Read(banmap_t& banSet);
};

void DumpBanlist(); // ������ֹ�б�����������

/** Return a timestamp in the future (in microseconds) for exponentially distributed events. */ // ����ָ���ֲ��¼���δ����ʱ������Ժ���Ϊ��λ��
int64_t PoissonNextSend(int64_t nNow, int average_interval_seconds);

#endif // BITCOIN_NET_H