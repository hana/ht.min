/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"

#include <vector>

using namespace c74::min;


class ht_COBS : public object<ht_COBS> {
private:
    vector<int> input_buffer;
        
    static size_t getEncodedBufferSize(size_t unencodedBufferSize) {
        return unencodedBufferSize + unencodedBufferSize / 254 + 2;
    }
    
    template<typename T>
    static void limit(std::vector<T> &buf)    {
        for(auto val : buf) {
            if (255 < val) {
                val = 255;
            } else if (val < 0) {
                val = 0;
            }
        }
    }
            
    static size_t encode(const int* buffer, size_t size, int* encodedBuffer) {
        size_t read_index = 0;
        size_t write_index = 1;
        size_t code_index = 0;
        int code = 1;

        while(read_index < size) {
            if(buffer[read_index] == 0) {
                encodedBuffer[code_index] = code;
                 code = 1;
                 code_index = write_index++;
                 read_index++;
            } else {
                encodedBuffer[write_index++] = buffer[read_index++];
                code++;

                if (code == 0xFF)  {
                    encodedBuffer[code_index] = code;
                    code = 1;
                    code_index = write_index++;
                }
            }
        }
        encodedBuffer[code_index] = code;

        return write_index;
    };
    
    static size_t decode(const int* encodedBuffer, size_t size,  int* decodedBuffer) {
        if(size == 0) {
            return 0;
        }

        size_t read_index  = 0;
        size_t write_index = 0;
        int code       = 0;
        int i          = 0;

        while (read_index < size) {
            code = encodedBuffer[read_index];

            if (read_index + code > size && code != 1) {
                return 0;
            }

            read_index++;

            for (i = 1; i < code; i++) {
                decodedBuffer[write_index++] = encodedBuffer[read_index++];
            }

            if (code != 0xFF && read_index != size)  {
                decodedBuffer[write_index++] = '\0';
            }
        }

        return write_index;
    }
    
    template<typename T>
    static T encode(const T& inputBuffer) {
        T encodedBuffer(getEncodedBufferSize(inputBuffer.size()), 0);
        encode(&inputBuffer[0], inputBuffer.size(), &encodedBuffer[0]);

        return encodedBuffer;
    };

    template<typename T>
    static T decode(const T& inputBuffer) {
        T decodedBuffer(inputBuffer.size() - 1, 0);
        decode(&inputBuffer[0], inputBuffer.size(), &decodedBuffer[0]);
        decodedBuffer.pop_back();
        return decodedBuffer;
    }
    mutex m_mutex;
    
public:
    MIN_DESCRIPTION	{"Encode/decode binary by Consistent Overhead Byte Stuffing (COBS) format."};
    MIN_TAGS		{"utilities"};
    MIN_AUTHOR		{"Hananosuke Takimoto"};
    MIN_RELATED		{"serial, itoa, atoi"};

    inlet<>  input	{ this, "(list) data to encode/decode" };
    outlet<> output	{ this, "(anything) result" };

    enum class operations : int { encode, decode, enum_count};
    enum_map operations_range = {"encode", "decode"};
    
    argument<symbol> operation_arg {this, "operation", "Operation mode - encode/decode.",
        MIN_ARGUMENT_FUNCTION {
            if (arg == "decode") {
                operation = operations::decode;
            } else {
                operation = operations::encode;
            }
        }
    };
    
    // the actual attribute for the message
    attribute<operations> operation { this, "operation", operations::encode, operations_range,
        description {
            "Choose the operation to perfome with the input. encode or decode."
        }
    };
            
    function process = MIN_FUNCTION {
        const auto buffer = from_atoms<std::vector<int>>(args);
        switch (operation) {
            case operations::encode: {
                std::vector<int> encodedBuffer(getEncodedBufferSize(buffer.size()), 0);
                limit(encodedBuffer);
                encode(buffer.data(), buffer.size(), encodedBuffer.data());
                output.send(to_atoms(encodedBuffer));
                break;
            }
            case operations::decode: {
                if(buffer[0] == 0) {
                    cout << "COBS-encoded message should not start with 0." << endl;
                    return {};
                }
                std::vector<int> decodedBuffer(buffer.size() - 2, 0);
                decode(buffer.data(), buffer.size(), decodedBuffer.data());
                output.send(to_atoms(decodedBuffer));
                break;
            }
            default: {
                break;
            }
        }
        return {};
    };

    // respond to the bang message to do something
    message<threadsafe::yes> list { this, "list", "List of binary to process.", process};
    message<threadsafe::yes> anything { this, "anything", "Things to process.", process};
    
    message<> number {this, "int", "int stream to be decoded", MIN_FUNCTION {
        const int value = args[0];
        
        switch (operation) {
            case operations::encode: {
                std::vector<int> vec;
                vec.emplace_back(value);
                const auto encodedBuffer = encode(vec);
                output.send(to_atoms(encodedBuffer));
                break;
                }
            case operations::decode: {
                if (value == 0) {
                    if(1 < input_buffer.size()) {
                        input_buffer.emplace_back(value);
                        const auto decodedBuffer = decode(input_buffer);
                        output.send(to_atoms(decodedBuffer));
                        input_buffer.clear();
                    }
                } else {
                    input_buffer.emplace_back(value);
                }
                
                break;
            }
            default: {
                break;
            }
        }
        return {};
    }};

    message<> clear {this, "clear",
        MIN_FUNCTION {            
            input_buffer.clear();
            return{};
        }
    };
    
    
//    message<> setup { this, "setup",
//        MIN_FUNCTION {
//
//            return {};
//        }
//    };

private:

};


MIN_EXTERNAL(ht_COBS);
