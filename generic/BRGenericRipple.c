//
//  BRGenericRipple.c
//  Core
//
//  Created by Ed Gamble on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRGenericRipple.h"
#include "ripple/BRRippleAccount.h"
#include "ripple/BRRippleWallet.h"
#include "ripple/BRRippleTransaction.h"
#include "ripple/BRRippleFeeBasis.h"
#include "support/BRSet.h"
#include "ethereum/util/BRUtilHex.h"

// MARK: - Generic Network

// MARK: - Generic Account

static BRGenericAccountRef
genericRippleAccountCreate (const char *type, UInt512 seed) {
    return (BRGenericAccountRef) rippleAccountCreateWithSeed (seed);
}

static BRGenericAccountRef
genericRippleAccountCreateWithPublicKey (const char *type, BRKey key) {
    return (BRGenericAccountRef) rippleAccountCreateWithKey (key);
}

static BRGenericAccountRef
genericRippleAccountCreateWithSerialization (const char *type, uint8_t *bytes, size_t bytesCount) {
    return (BRGenericAccountRef) rippleAccountCreateWithSerialization (bytes, bytesCount);
}

static void
genericRippleAccountFree (BRGenericAccountRef account) {
    rippleAccountFree ((BRRippleAccount) account);
    free (account);
}

static BRGenericAddressRef
genericRippleAccountGetAddress (BRGenericAccountRef account) {
    return (BRGenericAddressRef) rippleAccountGetAddress((BRRippleAccount) account);
}

static uint8_t *
genericRippleAccountGetSerialization (BRGenericAccountRef account,
                                      size_t *bytesCount) {
    return rippleAccountGetSerialization ((BRRippleAccount) account, bytesCount);
}

static void
genericRippleAccountSignTransferWithSeed (BRGenericAccountRef account,
                                          BRGenericTransferRef transfer,
                                          UInt512 seed)
{
    // Get the transaction pointer from this transfer
    BRRippleTransaction transaction = rippleTransferGetTransaction((BRRippleTransfer) transfer);
    if (transaction) {
        // Hard code the sequence to 7
        rippleAccountSetSequence ((BRRippleAccount) account, 7);
        rippleAccountSignTransaction ((BRRippleAccount) account, transaction, seed);
    }
}

static void
genericRippleAccountSignTransferWithKey (BRGenericAccountRef account,
                                         BRGenericTransferRef transfer,
                                         BRKey *key)
{
    // Get the transaction pointer from this transfer
    BRRippleTransaction transaction = rippleTransferGetTransaction ((BRRippleTransfer) transfer);
    if (transaction) {
        // Hard code the sequence to 7
        rippleAccountSetSequence ((BRRippleAccount) account, 7);
//        rippleAccountSignTransaction(account, transaction, seed);
        assert (0);
    }
}

// MARK: - Generic Address

static BRGenericAddressRef
genericRippleAddressCreate (const char *string) {
    return (BRGenericAddressRef) rippleAddressCreateFromString (string);
}

static char *
genericRippleAddressAsString (BRGenericAddressRef address) {
    return rippleAddressAsString ((BRRippleAddress) address);
}

static int
genericRippleAddressEqual (BRGenericAddressRef address1,
                           BRGenericAddressRef address2) {
    return rippleAddressEqual ((BRRippleAddress) address1,
                               (BRRippleAddress) address2);
}

static void
genericRippleAddressFree (BRGenericAddressRef address) {
    rippleAddressFree ((BRRippleAddress) address);
}

// MARK: - Generic Transfer

static BRGenericTransferRef
genericRippleTransferCreate (BRGenericAddressRef source,
                             BRGenericAddressRef target,
                             UInt256 amount)
{
    BRRippleUnitDrops amountDrops = UInt64GetLE(amount.u8);

    return (BRGenericTransferRef) rippleTransferCreateNew ((BRRippleAddress) source,
                                                           (BRRippleAddress) target,
                                                           amountDrops);
}

static void
genericRippleTransferFree (BRGenericTransferRef transfer) {
    rippleTransferFree ((BRRippleTransfer) transfer);
}

static BRGenericAddressRef
genericRippleTransferGetSourceAddress (BRGenericTransferRef transfer) {
    return (BRGenericAddressRef) rippleTransferGetSource ((BRRippleTransfer) transfer);
}

static BRGenericAddressRef
genericRippleTransferGetTargetAddress (BRGenericTransferRef transfer) {
    return (BRGenericAddressRef) rippleTransferGetTarget ((BRRippleTransfer) transfer);
}

static UInt256
genericRippleTransferGetAmount (BRGenericTransferRef transfer) {
    BRRippleUnitDrops drops = rippleTransferGetAmount ((BRRippleTransfer) transfer);
    return createUInt256(drops);
}

static UInt256
genericRippleTransferGetFee (BRGenericTransferRef transfer) {
    BRRippleUnitDrops drops = rippleTransferGetFee ((BRRippleTransfer) transfer);
    return createUInt256(drops);
}

static BRGenericFeeBasis
genericRippleTransferGetFeeBasis (BRGenericTransferRef transfer) {
    BRRippleUnitDrops rippleFee = rippleTransferGetFee ((BRRippleTransfer) transfer);
    return (BRGenericFeeBasis) {
        createUInt256 (rippleFee),
        1
    };
}

static BRGenericHash
genericRippleTransferGetHash (BRGenericTransferRef transfer) {
    BRRippleTransactionHash hash = rippleTransferGetTransactionId ((BRRippleTransfer) transfer);
    UInt256 value;
    memcpy (value.u8, hash.bytes, 32);
    return (BRGenericHash) { value };
}

static uint8_t *
genericRippleTransferGetSerialization (BRGenericTransferRef transfer, size_t *bytesCount)
{
    uint8_t * result = NULL;
    *bytesCount = 0;
    BRRippleTransaction transaction = rippleTransferGetTransaction ((BRRippleTransfer) transfer);
    if (transaction) {
        result = rippleTransactionSerialize(transaction, bytesCount);
    }
    return result;
}

// MARK: Generic Wallet

static BRGenericWalletRef
genericRippleWalletCreate (BRGenericAccountRef account) {
    return (BRGenericWalletRef) rippleWalletCreate ((BRRippleAccount) account);
}

static void
genericRippleWalletFree (BRGenericWalletRef wallet) {
    rippleWalletFree ((BRRippleWallet) wallet);
}

static UInt256
genericRippleWalletGetBalance (BRGenericWalletRef wallet) {
    return createUInt256 (rippleWalletGetBalance ((BRRippleWallet) wallet));
}

static BRGenericTransferRef
genericRippleWalletCreateTransfer (BRGenericWalletRef wallet,
                                   BRGenericAddressRef target,
                                   UInt256 amount,
                                   BRGenericFeeBasis estimatedFeeBasis) {
    BRRippleAddress source = rippleWalletGetSourceAddress ((BRRippleWallet) wallet);
    BRRippleUnitDrops drops  = amount.u64[0];

    return (BRGenericTransferRef) rippleTransferCreateNew (source,
                                                           (BRRippleAddress) target,
                                                           drops);
}

static BRGenericFeeBasis
genericRippleWalletEstimateFeeBasis (BRGenericWalletRef wallet,
                                     BRGenericAddressRef address,
                                     UInt256 amount,
                                     UInt256 pricePerCostFactor) {
    return (BRGenericFeeBasis) {
        pricePerCostFactor,
        1
    };
}

/// MARK: - File Service

static const char *fileServiceTypeTransactions = "transactions";

enum {
    RIPPLE_TRANSACTION_VERSION_1
};

static UInt256
fileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity) {
    BRRippleTransaction transaction = (BRRippleTransaction) entity;
    BRRippleTransactionHash transactionHash = rippleTransactionGetHash(transaction);

    UInt256 hash;
    memcpy (hash.u32, transactionHash.bytes, 32);

    return hash;
}

static uint8_t *
fileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount) {
    BRRippleTransaction transaction = (BRRippleTransaction) entity;

    size_t bufferCount;
    uint8_t *buffer = rippleTransactionSerialize (transaction, &bufferCount);

    // Require (for now) a valid serialization
    assert (NULL != buffer && 0 != bufferCount);

    if (NULL != bytesCount) *bytesCount = (uint32_t) bufferCount;
    return buffer;
}

static void *
fileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    return rippleTransactionCreateFromBytes (bytes, bytesCount);
}

// MARK: - Generic Manager

static BRGenericTransferRef
genericRippleWalletManagerRecoverTransfer (const char *hash,
                                           const char *from,
                                           const char *to,
                                           const char *amount,
                                           const char *currency,
                                           uint64_t timestamp,
                                           uint64_t blockHeight) {
    BRRippleUnitDrops amountDrops;
    sscanf(amount, "%llu", &amountDrops);
    BRRippleAddress toAddress   = rippleAddressCreateFromString(to);
    BRRippleAddress fromAddress = rippleAddressCreateFromString(from);
    // Convert the hash string to bytes
    BRRippleTransactionHash txId;
    decodeHex(txId.bytes, sizeof(txId.bytes), hash, strlen(hash));

    BRRippleTransfer transfer = rippleTransferCreate(fromAddress, toAddress, amountDrops, txId, timestamp, blockHeight);

    rippleAddressFree (toAddress);
    rippleAddressFree (fromAddress);

    return (BRGenericTransferRef) transfer;
}

static BRArrayOf(BRGenericTransferRef)
genericRippleWalletManagerRecoverTransfersFromRawTransaction (uint8_t *bytes,
                                                            size_t   bytesCount) {
    return NULL;
}

static void
genericRippleWalletManagerInitializeFileService (BRFileServiceContext context,
                                                 BRFileService fileService) {
    if (1 != fileServiceDefineType (fileService, fileServiceTypeTransactions, RIPPLE_TRANSACTION_VERSION_1,
                                    context,
                                    fileServiceTypeTransactionV1Identifier,
                                    fileServiceTypeTransactionV1Reader,
                                    fileServiceTypeTransactionV1Writer) ||

        1 != fileServiceDefineCurrentVersion (fileService, fileServiceTypeTransactions,
                                              RIPPLE_TRANSACTION_VERSION_1))

        return; //  bwmCreateErrorHandler (bwm, 1, fileServiceTypeTransactions);
}


static BRArrayOf(BRGenericTransferRef)
genericRippleWalletManagerLoadTransfers (BRFileServiceContext context,
                                         BRFileService fileService) {
    BRSetOf (BRRippleTransaction) transactionsSet = rippleTransactionSetCreate (5);
    // Load all transactions while upgrading.
    fileServiceLoad (fileService, transactionsSet, fileServiceTypeTransactions, 1);

    BRArrayOf(BRGenericTransferRef) result;
    array_new (result, BRSetCount(transactionsSet));
    FOR_SET (BRGenericTransferRef, transaction, transactionsSet) {
        array_add (result, transaction);
    }
    BRSetFree (transactionsSet);
    return result;
}

static BRGenericAPISyncType
genericRippleWalletManagerGetAPISyncType (void) {
    return GENERIC_SYNC_TYPE_TRANSFER;
}

// MARK: - Generic Handlers

struct BRGenericHandersRecord genericRippleHandlersRecord = {
    "xrp",
    { // Network
    },

    {    // Account
        genericRippleAccountCreate,
        genericRippleAccountCreateWithPublicKey,
        genericRippleAccountCreateWithSerialization,
        genericRippleAccountFree,
        genericRippleAccountGetAddress,
        genericRippleAccountGetSerialization,
        genericRippleAccountSignTransferWithSeed,
        genericRippleAccountSignTransferWithKey,
    },

    {    // Address
        genericRippleAddressCreate,
        genericRippleAddressAsString,
        genericRippleAddressEqual,
        genericRippleAddressFree
    },

    {    // Transfer
        genericRippleTransferCreate,
        genericRippleTransferFree,
        genericRippleTransferGetSourceAddress,
        genericRippleTransferGetTargetAddress,
        genericRippleTransferGetAmount,
        genericRippleTransferGetFee,
        genericRippleTransferGetFeeBasis,
        NULL,
        genericRippleTransferGetHash,
        genericRippleTransferGetSerialization,
    },

    {   // Wallet
        genericRippleWalletCreate,
        genericRippleWalletFree,
        genericRippleWalletGetBalance,
        genericRippleWalletCreateTransfer,
        genericRippleWalletEstimateFeeBasis
    },

    { // Wallet Manager
        genericRippleWalletManagerRecoverTransfer,
        genericRippleWalletManagerRecoverTransfersFromRawTransaction,
        genericRippleWalletManagerInitializeFileService,
        genericRippleWalletManagerLoadTransfers,
        genericRippleWalletManagerGetAPISyncType,
    },
};

const BRGenericHandlers genericRippleHandlers = &genericRippleHandlersRecord;
