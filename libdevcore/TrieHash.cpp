#if !defined(ETH_EMSCRIPTEN)

#include "TrieHash.h"
#include "TrieCommon.h"
#include "TrieDB.h"	// @TODO replace ASAP!
#include "SHA3.h"
using namespace std;
using namespace dev;

namespace dev
{


/*/
#define APPEND_CHILD appendData
/*/
#define APPEND_CHILD appendRaw
/**/

#define ENABLE_DEBUG_PRINT 0

#if ENABLE_DEBUG_PRINT
bool g_hashDebug = false;
#endif

void hash256aux(HexMap const& _s, HexMap::const_iterator _begin, HexMap::const_iterator _end, unsigned _preLen, RLPStream& _rlp);

void hash256rlp(HexMap const& _s, HexMap::const_iterator _begin, HexMap::const_iterator _end, unsigned _preLen, RLPStream& _rlp)
{
#if ENABLE_DEBUG_PRINT
	static std::string s_indent;
	if (_preLen)
		s_indent += "  ";
#endif

	if (_begin == _end)
		_rlp << "";	// NULL
	else if (std::next(_begin) == _end)
	{
		// only one left - terminate with the pair.
		_rlp.appendList(2) << hexPrefixEncode(_begin->first, true, _preLen) << _begin->second;
#if ENABLE_DEBUG_PRINT
		if (g_hashDebug)
			std::cerr << s_indent << toHex(bytesConstRef(_begin->first.data() + _preLen, _begin->first.size() - _preLen), 1) << ": " << _begin->second << " = " << sha3(_rlp.out()) << std::endl;
#endif
	}
	else
	{
		// find the number of common prefix nibbles shared
		// i.e. the minimum number of nibbles shared at the beginning between the first hex string and each successive.
		unsigned sharedPre = (unsigned)-1;
		unsigned c = 0;
		for (auto i = std::next(_begin); i != _end && sharedPre; ++i, ++c)
		{
			unsigned x = std::min(sharedPre, std::min((unsigned)_begin->first.size(), (unsigned)i->first.size()));
			unsigned shared = _preLen;
			for (; shared < x && _begin->first[shared] == i->first[shared]; ++shared) {}
			sharedPre = std::min(shared, sharedPre);
		}
		if (sharedPre > _preLen)
		{
			// if they all have the same next nibble, we also want a pair.
#if ENABLE_DEBUG_PRINT
			if (g_hashDebug)
				std::cerr << s_indent << toHex(bytesConstRef(_begin->first.data() + _preLen, sharedPre), 1) << ": " << std::endl;
#endif
			_rlp.appendList(2) << hexPrefixEncode(_begin->first, false, _preLen, (int)sharedPre);
			hash256aux(_s, _begin, _end, (unsigned)sharedPre, _rlp);
#if ENABLE_DEBUG_PRINT
			if (g_hashDebug)
				std::cerr << s_indent << "= " << hex << sha3(_rlp.out()) << dec << std::endl;
#endif
		}
		else
		{
			// otherwise enumerate all 16+1 entries.
			_rlp.appendList(17);
			auto b = _begin;
			if (_preLen == b->first.size())
			{
#if ENABLE_DEBUG_PRINT
				if (g_hashDebug)
					std::cerr << s_indent << "@: " << b->second << std::endl;
#endif
				++b;
			}
			for (auto i = 0; i < 16; ++i)
			{
				auto n = b;
				for (; n != _end && n->first[_preLen] == i; ++n) {}
				if (b == n)
					_rlp << "";
				else
				{
#if ENABLE_DEBUG_PRINT
					if (g_hashDebug)
						std::cerr << s_indent << std::hex << i << ": " << std::dec << std::endl;
#endif
					hash256aux(_s, b, n, _preLen + 1, _rlp);
				}
				b = n;
			}
			if (_preLen == _begin->first.size())
				_rlp << _begin->second;
			else
				_rlp << "";

#if ENABLE_DEBUG_PRINT
			if (g_hashDebug)
				std::cerr << s_indent << "= " << hex << sha3(_rlp.out()) << dec << std::endl;
#endif
		}
	}
#if ENABLE_DEBUG_PRINT
	if (_preLen)
		s_indent.resize(s_indent.size() - 2);
#endif
}

}

#endif // ETH_EMSCRIPTEN
