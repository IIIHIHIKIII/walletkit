
#ifndef BRCryptoBTC_h
#define BRCryptoBTC_h

#include "../BRCryptoHandlersExport.h"

#include "crypto/BRCryptoWalletSweeperP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRTransaction.h"
#include "bitcoin/BRChainParams.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK - Address

typedef struct BRCryptoAddressBTCRecord {
    struct BRCryptoAddressRecord base;

    /// The 'bitcoin/' address.  For BTC, addr.s is the string; for BCH, addr.s is
    /// encoded in a 'BCH' specific way.
    BRAddress addr;
} *BRCryptoAddressBTC;

extern BRCryptoAddress
cryptoAddressCreateAsBTC (BRCryptoBlockChainType type,
                          BRAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress);

private_extern BRAddress
cryptoAddressAsBTC (BRCryptoAddress address,
                    BRCryptoBoolean *isBitcoinAddr);

// MARK: - Network

typedef struct BRCryptoNetworkBTCRecord {
    struct BRCryptoNetworkRecord base;

    const BRChainParams *params;
} *BRCryptoNetworkBTC;

extern const BRChainParams *
cryptoNetworkAsBTC (BRCryptoNetwork network);

extern uint64_t
cryptoNetworkFeeAsBTC (BRCryptoNetworkFee networkFee);


// MARK: - Transfer

typedef struct BRCryptoTransferBTCRecord {
    struct BRCryptoTransferRecord base;

    // The BRTransaction; this is 100% owned by BRCryptoTransfer.  It can be accessed at any time.
    // Prior to signing the hash will be empty.
    BRTransaction *tid;

    // Tracking of 'deleted' and 'resolved'
    bool isDeleted;
    bool isResolved;

    uint64_t fee;
    uint64_t send;
    uint64_t recv;
} *BRCryptoTransferBTC;

extern BRCryptoTransferBTC
cryptoTransferCoerceBTC (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRWallet *wid,
                           BRTransaction *tid,
                           BRCryptoBlockChainType type);

private_extern BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transferBase);

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transferBase,
                      BRTransaction *btc);

// MARK: - Wallet

typedef struct BRCryptoWalletBTCRecord {
    struct BRCryptoWalletRecord base;
    BRWallet *wid;
} *BRCryptoWalletBTC;

extern BRCryptoWalletHandlers cryptoWalletHandlersBTC;

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoBlockChainType type,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWallet *wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc);

private_extern BRCryptoTransferBTC
cryptoWalletFindTransferByHashAsBTC (BRCryptoWallet wallet,
                                     UInt256 hash);
#if 0
private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWalletManager bwm,
                         BRWallet *wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWalletManager bwm,
                         BRWallet *wid);

#endif

// MARK: - (Wallet) Manager

typedef struct BRCryptoWalletManagerBTCRecord {
    struct BRCryptoWalletManagerRecord base;

    int ignoreTBD;
} *BRCryptoWalletManagerBTC;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC;

#ifdef REFACTOR
private_extern BRWalletManager
cryptoWalletManagerAsBTC (BRCryptoWalletManager manager) {
    assert (BLOCK_CHAIN_TYPE_BTC == manager->type);
    return manager->u.btc;
}

private_extern BREthereumEWM
cryptoWalletManagerAsETH (BRCryptoWalletManager manager) {
    assert (BLOCK_CHAIN_TYPE_ETH == manager->type);
    return manager->u.eth;
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasBTC (BRCryptoWalletManager manager,
                           BRWalletManager bwm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_BTC == manager->type && bwm == manager->u.btc);
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasETH (BRCryptoWalletManager manager,
                           BREthereumEWM ewm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_ETH == manager->type && ewm == manager->u.eth);
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasGEN (BRCryptoWalletManager manager,
                           BRGenericManager gwm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_GEN == manager->type && gwm == manager->u.gen);
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsBTC (BRCryptoWalletManager cwm,
                                    BRWallet *btc) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (btc == cryptoWalletAsBTC (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsETH (BRCryptoWalletManager cwm,
                                    BREthereumWallet eth) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (eth == cryptoWalletAsETH (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsGEN (BRCryptoWalletManager cwm,
                                    BRGenericWallet gen) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (gen == cryptoWalletAsGEN (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}
#endif

// MARK: - Wallet Sweeper

typedef struct BRCryptoWalletSweeperBTCRecord {
    struct BRCryptoWalletSweeperRecord base;

    BRAddressParams addrParams;
    uint8_t isSegwit;
    char * sourceAddress;
    BRArrayOf(BRTransaction *) txns;
} *BRCryptoWalletSweeperBTC;

// MARK: - Events

extern const BREventType *bwmEventTypes[];
extern const unsigned int bwmEventTypesCount;

// MARK: - Support

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint32_t feePerKB,
                           uint32_t sizeInByte);

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis);

private_extern BRCryptoHash
cryptoHashCreateAsBTC (UInt256 btc);

#ifdef __cplusplus
}
#endif

#endif // BRCryptoBTC_h