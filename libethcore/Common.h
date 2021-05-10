
#pragma once

#include <string>
#include <functional>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libdevcrypto/Common.h>

namespace dev
{
namespace eth
{

/// Current protocol version.
extern const unsigned c_protocolVersion;

/// Current minor protocol version.
extern const unsigned c_minorProtocolVersion;

/// Current database version.
extern const unsigned c_databaseVersion;

/// User-friendly string representation of the amount _b in wei.
std::string formatBalance(bigint const& _b);

DEV_SIMPLE_EXCEPTION(InvalidAddress);

/// Convert the given string into an address.
Address toAddress(std::string const& _s);

/// Get information concerning the currency denominations.
std::vector<std::pair<u256, std::string>> const& units();

/// The log bloom's size (2048-bit).
using LogBloom = h2048;

/// Many log blooms.
using LogBlooms = std::vector<LogBloom>;

// The various denominations; here for ease of use where needed within code.
static const u256 ether = exp10<18>();
static const u256 finney = exp10<15>();
static const u256 szabo = exp10<12>();
static const u256 shannon = exp10<9>();
static const u256 wei = exp10<0>();

using Nonce = h64;

using BlockNumber = unsigned;

static const BlockNumber LatestBlock = (BlockNumber)-2;
static const BlockNumber PendingBlock = (BlockNumber)-1;
static const h256 LatestBlockHash = h256(2);
static const h256 EarliestBlockHash = h256(1);
static const h256 PendingBlockHash = h256(0);

static const u256 DefaultBlockGasLimit = 4712388;

enum class RelativeBlock: BlockNumber
{
	Latest = LatestBlock,
	Pending = PendingBlock
};

class Transaction;

struct ImportRoute
{
	h256s deadBlocks;
	h256s liveBlocks;
	std::vector<Transaction> goodTranactions;
};


enum class ImportResult
{
	Success = 0,
	UnknownParent,
	FutureTimeKnown,
	FutureTimeUnknown,
	AlreadyInChain,
	AlreadyKnown,
	Malformed,
	OverbidGasPrice,
	BadChain,
	ZeroSignature
};

struct ImportRequirements
{
	using value = unsigned;
	enum
	{
		ValidSeal = 1, ///< Validate seal
		UncleBasic = 4, ///< Check the basic structure of the uncles.
		TransactionBasic = 8, ///< Check the basic structure of the transactions.
		UncleSeals = 16, ///< Check the basic structure of the uncles.
		TransactionSignatures = 32, ///< Check the basic structure of the transactions.
		Parent = 64, ///< Check parent block header.
		UncleParent = 128, ///< Check uncle parent block header.
		PostGenesis = 256, ///< Require block to be non-genesis.
		CheckUncles = UncleBasic | UncleSeals, ///< Check uncle seals.
		CheckTransactions = TransactionBasic | TransactionSignatures, ///< Check transaction signatures.
		OutOfOrderChecks = ValidSeal | CheckUncles | CheckTransactions, ///< Do all checks that can be done independently of prior blocks having been imported.
		InOrderChecks = Parent | UncleParent, ///< Do all checks that cannot be done independently of prior blocks having been imported.
		Everything = ValidSeal | CheckUncles | CheckTransactions | Parent | UncleParent,
		None = 0
	};
};


/// Super-duper signal mechanism. TODO: replace with somthing a bit heavier weight.
template<typename... Args> class Signal
{
public:
	using Callback = std::function<void(Args...)>;

	class HandlerAux
	{
		friend class Signal;

	public:
		~HandlerAux() { if (m_s) m_s->m_fire.erase(m_i); }
		void reset() { m_s = nullptr; }
		void fire(Args const&... _args) { m_h(_args...); }

	private:
		HandlerAux(unsigned _i, Signal* _s, Callback const& _h): m_i(_i), m_s(_s), m_h(_h) {}

		unsigned m_i = 0;
		Signal* m_s = nullptr;
		Callback m_h;
	};

	~Signal()
	{
		for (auto const& h : m_fire)
			if (auto l = h.second.lock())
				l->reset();
	}

	std::shared_ptr<HandlerAux> add(Callback const& _h)
	{
		auto n = m_fire.empty() ? 0 : (m_fire.rbegin()->first + 1);
		auto h =  std::shared_ptr<HandlerAux>(new HandlerAux(n, this, _h));
		m_fire[n] = h;
		return h;
	}

	void operator()(Args const&... _args)
	{
		for (auto const& f: valuesOf(m_fire))
			if (auto h = f.lock())
				h->fire(_args...);
	}

private:
	std::map<unsigned, std::weak_ptr<typename Signal::HandlerAux>> m_fire;
};

}


}
