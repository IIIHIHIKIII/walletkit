package com.breadwallet.crypto;

public enum WalletManagerSyncDepth {
    /**
     * Sync from the block height of the last confirmed send transaction.
     */
    FROM_LAST_CONFIRMED_SEND,

    /**
     * Sync from the block height of the last trusted block; this is dependent on the
     * blockchain and mode as to how it determines trust.
     */
    FROM_LAST_TRUSTED_BLOCK,

    /**
     * Sync from the block height of the point in time when the account was created.
     */
    FROM_CREATION;

    public int toSerialization() {
        switch (this) {
            case FROM_LAST_CONFIRMED_SEND: return 0xa0;
            case FROM_LAST_TRUSTED_BLOCK:  return 0xb0;
            case FROM_CREATION:            return 0xc0;
            default: return 0; // error
        }
    }

    public static WalletManagerSyncDepth fromSerialization(int serialization) {
        switch (serialization) {
            case 0xa0: return FROM_LAST_CONFIRMED_SEND;
            case 0xb0: return FROM_LAST_TRUSTED_BLOCK;
            case 0xc0: return FROM_CREATION;
            default: return null;
        }
    }
}
