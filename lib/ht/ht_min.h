#pragma once

#include "c74_min.h"

#include <span>
#include <vector>

#include "oscpkt/oscpkt.hh"

namespace ht::min {

template<typename T>
concept Container =  requires (T& x) {
    x.begin(); // 型Tに要求する操作をセミコロン区切りで列挙する。
    x.end();// ここでは、メンバ関数draw()を呼び出せることを要求している。
};


template<Container T>
constexpr auto to_container(oscpkt::PacketWriter& pr) {
    const auto head = reinterpret_cast<uint8_t*>(pr.packetData());
    const auto tail = head + pr.packetSize();
    return T(head, tail);
}


namespace osc {

/*
 atoms to blob
 */

struct message_t {
    uint64_t timetag;
    c74::min::atoms atoms;
};

using bundle_t = std::vector<message_t>;
using blob = std::vector<uint8_t>;


namespace atoms {

template<typename T>
bool is_convertible_to_blob(const std::span<T> atoms) {
    for(const auto& elm : atoms) {
        if(!std::in_range<uint8_t>(static_cast<int>(elm))) {
            return false;
        }
    }
    
    return true;
}

auto is_convertible_to_blob(const c74::min::atoms& atoms) {
    return is_convertible_to_blob(std::span{atoms});
}


template<typename T>
auto to_blob(const std::span<T> atoms) {
    blob result;
    
    result.reserve(atoms.size());
    
    for(const auto& elm : atoms) {
        result.emplace_back(int(elm));
    }
    
    return result;
}

auto to_blob(const c74::min::atoms& atoms) {
    return to_blob(std::span{atoms});
}

template<typename T>
constexpr auto to_message(const std::span<T> atoms, const oscpkt::TimeTag timetag = oscpkt::TimeTag::immediate(), const bool use_double = false) {
    oscpkt::Message msg(std::string(atoms[0]), timetag);
    
    const auto atms_size = atoms.size();
        
    for(auto i = 1; i < atms_size; i++) {
        const auto& arg = atoms[i];
        switch(arg.type()) {
            case c74::min::message_type::int_argument:  {
                if( int64_t(arg) < std::numeric_limits<int32_t>::min() || std::numeric_limits<int32_t>::max() < int64_t(arg)) {
                    msg.pushInt64(arg);
                } else {
                    msg.pushInt32(arg);
                }
                
                break;
            }
            case c74::min::message_type::float_argument:
                if(use_double) {
                    msg.pushDouble(arg);
                } else {
                    msg.pushFloat(arg);
                }
                
                break;
            case c74::min::message_type::symbol_argument: {
                if (std::string(arg) == "OSCBlob") {
                    const auto blob_size = int(atoms[i + 1]);
                    const auto head = i + 2;                    
                    const auto span = std::span{atoms}.subspan(head, blob_size);

                    if(atoms::is_convertible_to_blob(span)) {
                        auto blob = atoms::to_blob(span);
                        msg.pushBlob(reinterpret_cast<void*>(blob.data()), blob.size());
                    }
                    
                    i += 1 + blob_size;
                } else {
                    msg.pushStr(std::string(arg));
                }
                
                break;
            }
            default:
                break;
        }
    }
    
    return msg;
}

template<typename T>
constexpr auto to_message(const T atoms, const bool use_double) {
    return to_message(atoms, oscpkt::TimeTag::immediate(), use_double);
}

inline constexpr auto to_message(const c74::min::atoms& atoms, const oscpkt::TimeTag timetag, const bool use_double = false) {
    return to_message(std::span{atoms}, oscpkt::TimeTag::immediate(), false);
}

inline constexpr auto to_message(const c74::min::atoms& atoms, const bool use_double) {
    return to_message(atoms, oscpkt::TimeTag::immediate(), use_double);
}

inline constexpr auto to_message(const c74::min::atoms& atoms, const oscpkt::TimeTag timetag = oscpkt::TimeTag::immediate()) {
    return to_message(atoms, timetag, false);
}

/*
 atoms to packet
 */

template<typename T>
auto to_packet(std::span<T> atoms, const bool use_double = false) {
    const auto msg = to_message(atoms);
    oscpkt::PacketWriter pw;
    pw.addMessage(msg);
    return std::vector<uint8_t>(pw.packetData(), pw.packetData() + pw.packetSize());
}

auto to_packet(const c74::min::atoms& atoms, const bool use_double = false) {
    return to_packet(std::span{atoms}, use_double);
}

} // namespace atoms

namespace message {
auto to_atoms(const oscpkt::Message* msg) {
    c74::min::atoms atm;
    
    atm.emplace_back(msg->addressPattern());
    // 型タグの文字列を取得 (例: "ifs" なら Int, Float, String の順)
    std::string tags = msg->typeTags();
    
    // 引数を取り出すためのリーダーを取得
    oscpkt::Message::ArgReader arg = msg->arg();

    // 型タグを一文字ずつ判定して、適切な型でpopする
    for (size_t i = 0; i < tags.length(); ++i) {
        char type = tags[i];
        
        switch (type) {
            case oscpkt::TYPE_TAG_INT32: { // 32bit integer
                int32_t val;
                arg.popInt32(val);
                atm.emplace_back(val);
                break;
            }
            case oscpkt::TYPE_TAG_FLOAT: { // 32bit float
                float val;
                arg.popFloat(val);
                atm.emplace_back(val);
                break;
            }
            case oscpkt::TYPE_TAG_STRING: { // string
                std::string val;
                arg.popStr(val);
                atm.emplace_back(val);
                break;
            }
            case oscpkt::TYPE_TAG_BLOB: { // Blob
                std::vector<char> blob;
                arg.popBlob(blob);
                atm.emplace_back("OSCBlob");
                atm.emplace_back(blob.size());
                const auto a = c74::min::to_atoms(blob);
                atm.reserve(atm.size() + a.size());
                std::copy(a.begin(),a.end(),std::back_inserter(atm));
                break;
            }
                // --- OSC 1.0/1.1 expanded tag ---
            case oscpkt::TYPE_TAG_INT64:
                int64_t val;
                arg.popInt64(val);
                atm.emplace_back(int64_t(val));
                break;
            case oscpkt::TYPE_TAG_TRUE: // True
                atm.emplace_back(true);
                break;
            case oscpkt::TYPE_TAG_FALSE: // False
                atm.emplace_back(false);
                break;
            case 'N': // Null
                atm.emplace_back("null");
                break;
            case 'I': // Impulse (Bang)
                atm.emplace_back("bang");
                break;
            default:
                break;
        }
    }
    return atm;
}

}// namespace message

/*
 packet to atoms
 */
namespace packet {
template<typename T>
auto to_atoms(const std::span<T> packet, const bool error_check = false) {
    std::vector<message_t> results;
    
    if(error_check && !atoms::is_convertible_to_blob(packet)) {
        std::cerr << "Not a list of uint8_t ranged value" << std::endl;
        return results;
    }
    
    oscpkt::PacketReader pr(packet.data(), packet.size());
    if (!pr.isOk()) {
        std::cerr << "Could not parse the data" << std::endl;
        return results;
    }
    
    oscpkt::Message* msg;
    
    while (pr.isOk() && (msg = pr.popMessage()) != nullptr) {
        c74::min::atoms atm = message::to_atoms(msg);
        message_t result {
            .timetag = msg->timeTag(),
            .atoms = std::move(atm)
        };
        
        results.emplace_back(std::move(result));
    }
    return results;
}

auto to_atoms(const c74::min::atoms& packet, const bool error_check = false) {
    return to_atoms(std::span{packet}, error_check);
}
}   // namespace packet


}   // namespace osc
}   // namespace ht::min
