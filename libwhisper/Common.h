/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Common.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <string>
#include <chrono>

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/RLP.h>
#include <libp2p/Capability.h>

namespace dev
{
namespace shh
{

/* this makes these symbols ambiguous on VS2013
using h256 = dev::h256;
using h256s = dev::h256s;
using bytes = dev::bytes;
using RLPStream = dev::RLPStream;
using RLP = dev::RLP;
using bytesRef = dev::bytesRef;
using bytesConstRef = dev::bytesConstRef;
using h256Set = dev::h256Set;
*/

class WhisperHost;
class WhisperPeer;
class Whisper;
class Envelope;

enum WhisperPacket
{
	StatusPacket = 0,
	MessagesPacket,
	TopicFilterPacket,
	PacketCount
};

static const unsigned TopicBloomFilterSize = 64;
static const unsigned BitsPerBloom = 3;
static const unsigned WhisperProtocolVersion = 3;

using AbridgedTopic = FixedHash<4>;
using Topic = h256;

using AbridgedTopics = std::vector<AbridgedTopic>;
using Topics = h256s;

using TopicBloomFilterHash = FixedHash<TopicBloomFilterSize>;

AbridgedTopic abridge(Topic const& _topic);
AbridgedTopics abridge(Topics const& _topics);

class BuildTopic
{
public:
	BuildTopic() {}
	template <class T> BuildTopic(T const& _t) { shift(_t); }

	template <class T> BuildTopic& shift(T const& _r) { return shiftBytes(RLPStream().append(_r).out()); }
	template <class T> BuildTopic& operator()(T const& _t) { return shift(_t); }

	BuildTopic& shiftRaw(h256 const& _part) { m_parts.push_back(_part); return *this; }

	operator AbridgedTopics() const { return toAbridgedTopics(); }
	operator Topics() const { return toTopics(); }
	AbridgedTopics toAbridgedTopics() const;
	Topics toTopics() const { return m_parts; }

protected:
	BuildTopic& shiftBytes(bytes const& _b);

	h256s m_parts;
};

}

}