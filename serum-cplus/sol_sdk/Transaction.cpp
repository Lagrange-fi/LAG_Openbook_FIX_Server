#include "Transaction.hpp"



namespace solana
{
    void Transaction::add_instruction(const Instruction& instruction)
    {
        instructions.push_back(instruction);
    }
    
    void Transaction::set_recent_blockhash(const string& blockhash)
    {
        _message.recent_blockhash = Hash(blockhash);
    }

    Transaction::string Transaction::serialize() const
    {
        string msg = "";

        //TODO compact array
        msg.push_back(static_cast<char>(signatures.size()));

        for (const auto& sign: signatures)
            msg.insert(msg.end(), sign.begin(), sign.end());

        msg.insert(msg.end(), serialized_message.begin(), serialized_message.end());
        return msg;
    }

    void Transaction::sign(const Signers& signers)
    {
        for (const auto key: signers)
            _sign(key); 
    }

    void Transaction::_sign(const Keypair& private_key)
    {    
        CryptoPP::byte raw_private_key[SIZE_PUBKEY];
        CryptoPP::byte raw_public_key[SIZE_PUBKEY];
        memcpy(raw_private_key, private_key.data(), SIZE_PUBKEY);
        memcpy(raw_public_key, private_key.data() + SIZE_PUBKEY, SIZE_PUBKEY);
        CryptoPP::ed25519::Signer signer(raw_public_key, raw_private_key);

        if (serialized_message.size() == 0)
            serialized_message = get_message_for_sign();
        // std::cout<< to_hex_string(serialized_message) << std::endl;
        string signature;

        CryptoPP::AutoSeededRandomPool prng;
        // Determine maximum signature size
        size_t siglen = signer.MaxSignatureLength();
        signature.resize(siglen);

        // Sign, and trim signature to actual size
        siglen = signer.SignMessage(prng, (const CryptoPP::byte*)&serialized_message[0], serialized_message.size(), (CryptoPP::byte*)&signature[0]);
        signature.resize(siglen);

        signatures.push_back(signature);
    }

    Transaction::string Transaction::get_message_for_sign()
    {
        Keys signed_writable_accounts;
        Keys signed_accounts;
        Keys writable_accounts;
        Keys other_accounts;

        Keys unique_accounts;

        auto push_back_if_unique = [&unique_accounts](Keys& dest, const PublicKey& key){
            if (std::find_if(unique_accounts.begin(), unique_accounts.end(), [&key](const PublicKey& i){ return i == key;}) == unique_accounts.end()){
                dest.push_back(key);
                unique_accounts.push_back(key);
                return true;
            }
            return false;
        };
        for (const auto instr: instructions) {
            // _message.header.num_accounts += instr.accounts.size();
            for (const auto tmp: instr.accounts) {
                if (tmp.is_signer) {
                    // ++_message.header.num_required_signatures;
                    if (tmp.is_writable) {
                        if (push_back_if_unique(signed_writable_accounts, tmp.pubkey)) {
                            ++_message.header.num_accounts;
                            ++_message.header.num_required_signatures;
                        }
                    }
                    else {
                        if (push_back_if_unique(signed_accounts, tmp.pubkey))
                        {
                            ++_message.header.num_accounts;
                            ++_message.header.num_required_signatures;
                            ++_message.header.num_readonly_signed_accounts;
                        }
                    }
                }
                else {
                    if (tmp.is_writable) {
                        if(push_back_if_unique(writable_accounts, tmp.pubkey))
                            ++_message.header.num_accounts;
                    }
                    else {
                        if(push_back_if_unique(other_accounts, tmp.pubkey)){
                            ++_message.header.num_accounts;
                            ++_message.header.num_readonly_unsigned_accounts;
                        }
                    }
                }
            }
            
            if(push_back_if_unique(other_accounts, instr.program_id)) {
                _message.header.num_readonly_unsigned_accounts++;
                _message.header.num_accounts++;
            }
        }
        // std::reverse(signed_writable_accounts.begin(), signed_writable_accounts.end());

        _message.account_keys = bytes(SIZE_PUBKEY * _message.header.num_accounts, 0);

        size_t shift = 0;
        add_keys(_message.account_keys, signed_writable_accounts, shift);
        shift += signed_writable_accounts.size() * SIZE_PUBKEY;

        add_keys(_message.account_keys, signed_accounts, shift);
        shift += signed_accounts.size() * SIZE_PUBKEY;

        add_keys(_message.account_keys, writable_accounts, shift);
        shift += writable_accounts.size() * SIZE_PUBKEY;

        add_keys(_message.account_keys, other_accounts, shift);
        shift += other_accounts.size() * SIZE_PUBKEY;

        auto compiled_instructions = get_message_from_instructions(instructions, _message.account_keys);

        size_t size = 4+_message.account_keys.size()+SIZE_HASH+compiled_instructions.size();
        uint8_t msg[size];

        shift = 0;
        memcpy(msg, &_message.header, 4); 
        shift += 4;
        memcpy(msg + shift, _message.account_keys.data(), _message.account_keys.size()); 
        shift += _message.account_keys.size();
        memcpy(msg + shift, _message.recent_blockhash.data(), SIZE_HASH); 
        shift += SIZE_HASH;
        memcpy(msg + shift, compiled_instructions.data(), compiled_instructions.size()); 
        shift += compiled_instructions.size();

        // auto tt = to_hex(msg, size);
        // std::cout << tt << std::endl;
        return string((char*)msg, size);
    }

    Transaction::bytes Transaction::get_message_from_instructions(const Instructions &instructions, const bytes &keys)
    {
        bytes msg = bytes();
        // TODO Compact-Array Format
        msg.push_back(instructions.size());
        for (const auto& instr: instructions) {
            auto compiled_instruction = compile_instruction(instr, keys);
            msg.insert(msg.end(), compiled_instruction.begin(), compiled_instruction.end());
        }

        return msg;
    }


    Transaction::bytes Transaction::compile_instruction(const Instruction& instr, const bytes& keys)
    {
        // TODO Compact-Array Format

        uint64_t size = 1 // the program id shift
        + 1 // num_accounts
        + instr.accounts.size() // accounts shift
        + 1 // data size
        + instr.data.size();
        
        bytes compiled_instruction = bytes(size);

        uint64_t shift = 0;
        memset(compiled_instruction.data(), index_of_pub_key(instr.program_id, keys), 1); 
        shift += 1;

        // TODO account_len into a Compact-Array Format
        memset(compiled_instruction.data() + shift, static_cast<uint8_t>(instr.accounts.size()), 1); 
        shift += 1;

        for (uint64_t  i = 0; i < instr.accounts.size(); i++) {
            memset(compiled_instruction.data() + shift, index_of_pub_key(instr.accounts[i].pubkey, keys), 1);
            shift += 1;
        }

        // TODO data_size into a Compact-Array Format
        memset(compiled_instruction.data() + shift, instr.data.size(), 1);
        shift += 1;

        memcpy(compiled_instruction.data() + shift, instr.data.data(), instr.data.size()); 
        shift += instr.data.size();

        return compiled_instruction;
    }

    uint8_t Transaction::index_of_pub_key(const PublicKey& pubkey, const bytes& keys)
    {
        uint8_t count = static_cast<uint8_t>(keys.size() / SIZE_PUBKEY);
        for (uint8_t i = 0; i < count; i++) {
            if (memcmp(pubkey.data(), keys.data() + i*SIZE_PUBKEY, SIZE_PUBKEY) == 0) {
                return i;
            }
        }
        return -1;
    }

    void Transaction::add_keys(bytes& data, const std::vector< PublicKey>& new_keys, size_t shift)
    {
        for (auto key : new_keys) {
            memcpy(data.data() + shift, key.data(), SIZE_PUBKEY);
            shift += SIZE_PUBKEY;
        }
    }
} // namespace solana