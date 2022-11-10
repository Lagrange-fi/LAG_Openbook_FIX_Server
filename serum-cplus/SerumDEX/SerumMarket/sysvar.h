#pragma once

// Public key of the synthetic account that serves the current network time.
#define SYSVAR_CLOCK_PUBKEY "SysvarC1ock11111111111111111111111111111111"
// "SysvarC1ock11111111111111111111111111111111"

// Public key of the synthetic account that serves recent blockhashes.
#define SYSVAR_RECENT_BLOCKHASHES_PUBKEY "SysvarRecentB1ockHashes11111111111111111111"
// "SysvarRecentB1ockHashes11111111111111111111"

// Public key of the synthetic account that serves the network fee resource consumption.
#define SYSVAR_RENT_PUBKEY "SysvarRent111111111111111111111111111111111"
// "SysvarRent111111111111111111111111111111111"

// Public key of the synthetic account that serves the network rewards.
#define SYSVAR_REWARDS_PUBKEY "SysvarRewards111111111111111111111111111111"
// "SysvarRewards111111111111111111111111111111"

// Public key of the synthetic account that serves the stake history.
#define SYSVAR_STAKE_HISTORY_PUBKEY "SysvarStakeHistory1111111111111111111111111"
// "SysvarStakeHistory1111111111111111111111111"

/*
* The EpochSchedule sysvar contains epoch scheduling constants that are set in genesis, and enables calculating the
* number of slots in a given epoch, the epoch for a given slot, etc. (Note: the epoch schedule is distinct from the
* `leader schedule <https://docs.solana.com/terminology#leader-schedule>`_).
*/
#define SYSVAR_EPOCH_SCHEDULE_PUBKEY "SysvarEpochSchedu1e111111111111111111111111"
// "SysvarEpochSchedu1e111111111111111111111111"

/*
* The Instructions sysvar contains the serialized instructions in a Message while that Message is being processed.
* This allows program instructions to reference other instructions in the same transaction.
* Read more information on `instruction introspection
* <https://docs.solana.com/implemented-proposals/instruction_introspection>`_.
*/
#define SYSVAR_INSTRUCTIONS_PUBKEY "Sysvar1nstructions1111111111111111111111111"
// "Sysvar1nstructions1111111111111111111111111"

// The SlotHashes sysvar contains the most recent hashes of the slot's parent banks. It is updated every slot.
#define SYSVAR_SLOT_HASHES_PUBKEY "SysvarS1otHashes111111111111111111111111111"
// "SysvarS1otHashes111111111111111111111111111"