#pragma once
#include "PublicKey.hpp"
#include <string.h>
#include <vector>

namespace solana
{
    class Instruction 
    {
    public:
        typedef struct {
            PublicKey pubkey; /** An account's public key */
            bool is_writable; /** True if the `pubkey` can be loaded as a read-write account */
            bool is_signer; /** True if an Instruction requires a Transaction signature matching `pubkey` */
        } AccountMeta;

        typedef std::vector<AccountMeta> AccountMetas;
        typedef std::vector<uint8_t> bytes; 

    private:
        PublicKey program_id; /** Pubkey of the instruction processor that executes this instruction */
        AccountMetas accounts; /** Metadata for what accounts should be passed to the instruction processor */
        bytes data; /** Opaque data passed to the instruction processor */

    public:
        Instruction();
        Instruction(const Instruction&);
        Instruction(Instruction&&);

        void set_account_id(const PublicKey&);
        void set_account_id(PublicKey&&);
        void set_data(const bytes&);
        void set_data(bytes&&);
        void set_data(const void*, size_t);
        void set_accounts(const AccountMetas&);
        void set_accounts(AccountMetas&&);
        // void add_accounts(const AccountMetas&);
        // void add_accounts(AccountMetas&&);
        void clear_accounts();

        ~Instruction();

        friend class Transaction;
    };
} // namespace solana

