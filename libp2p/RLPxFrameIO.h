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
/** @file RLPXFrameIO.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2015
 */


#pragma once

#include <memory>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/ECDHE.h>
#include <libdevcrypto/CryptoPP.h>
#include "Common.h"
namespace ba = boost::asio;
namespace bi = boost::asio::ip;

namespace dev
{
namespace p2p
{
	
class RLPXHandshake;

class RLPXSocket: public std::enable_shared_from_this<RLPXSocket>
{
public:
	RLPXSocket(bi::tcp::socket* _socket): m_socket(std::move(*_socket)) {}
	~RLPXSocket() { close(); }
	
	bool isConnected() const { return m_socket.is_open(); }
	void close() { try { boost::system::error_code ec; m_socket.shutdown(bi::tcp::socket::shutdown_both, ec); if (m_socket.is_open()) m_socket.close(); } catch (...){} }
	bi::tcp::endpoint remoteEndpoint() { try { return m_socket.remote_endpoint(); } catch (...){ return bi::tcp::endpoint(); } }
	bi::tcp::socket& ref() { return m_socket; }
	
protected:
	bi::tcp::socket m_socket;
};

class RLPXFrameIO
{
	friend class Session;
public:
	RLPXFrameIO(RLPXHandshake const& _init);
	
	void writeSingleFramePacket(bytesConstRef _packet, bytes& o_bytes);

	/// Authenticates and decrypts header in-place.
	bool authAndDecryptHeader(h256& io_cipherWithMac);
	
	/// Authenticates and decrypts frame in-place.
	bool authAndDecryptFrame(bytesRef io_cipherWithMac);
	
	h128 egressDigest();
	
	h128 ingressDigest();
	
	void updateEgressMACWithHeader(bytesConstRef _headerCipher);
	
	void updateEgressMACWithEndOfFrame(bytesConstRef _cipher);
	
	void updateIngressMACWithHeader(bytesConstRef _headerCipher);
	
	void updateIngressMACWithEndOfFrame(bytesConstRef _cipher);
	
protected:
	bi::tcp::socket& socket() { return m_socket->ref(); }
	
private:
	void updateMAC(CryptoPP::SHA3_256& _mac, h128 const& _seed = h128());

	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameEnc;
	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameDec;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption m_macEnc;
	CryptoPP::SHA3_256 m_egressMac;
	CryptoPP::SHA3_256 m_ingressMac;
	
	std::shared_ptr<RLPXSocket> m_socket;
};
	
}
}