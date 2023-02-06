#include "Instruction.hpp"


namespace solana
{
    Instruction::Instruction(): program_id(PublicKey()), accounts(AccountMetas(0)), data(bytes(0))
    {}

    Instruction::Instruction(const Instruction& instr) 
    : program_id(instr.program_id), accounts(instr.accounts), data(instr.data)
    {}

    Instruction::Instruction(Instruction&& instr)
    : program_id(std::move(instr.program_id)), accounts(std::move(instr.accounts)), data(std::move(instr.data))
    {}

    Instruction::~Instruction()
    {}

    void Instruction::set_account_id(const PublicKey& key)
    {
        program_id = key;
    }

    void Instruction::set_account_id(PublicKey&& key)
    {
        program_id = std::move(key);
    }

    void Instruction::set_data(const bytes& data)
    {
        this->data = data;
    }

    void Instruction::set_data(bytes&& data)
    {
        this->data = std::move(data);
    }

    void Instruction::set_data(const void* data, size_t size)
    {
        this->data = bytes(size);
        memcpy(this->data.data(), data, size);
    }

    void Instruction::set_accounts(const AccountMetas& accounts)
    {
        this->accounts = accounts;
    }

    void Instruction::set_accounts(AccountMetas&& accounts)
    {
        this->accounts = std::move(accounts);
    }

    // void Instruction::add_accounts(const AccountMetas& accounts)
    // {
    //     this->accounts.emplace_back(accounts);
    // }

    // void Instruction::add_accounts(AccountMetas&& accounts)
    // {
    //     this->accounts.emplace_back(std::move(accounts));
    // }

    void Instruction::clear_accounts()
    {
        accounts = AccountMetas(0);
    }
} // namespace solana


