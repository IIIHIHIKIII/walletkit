//  Created by Ed Gamble on 1/23/2018
//  Copyright (c) 2018 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <stdlib.h>
#include <malloc.h>
#include "BRPeerManager.h"
#include "BRChainParams.h"
#include "BRCoreJni.h"
#include "com_breadwallet_core_BRCorePeerManager.h"
#include "com_breadwallet_core_BRCoreTransaction.h"

#define JNI_COPY_TRANSACTION(tx)    \
    (com_breadwallet_core_BRCoreTransaction_JNI_COPIES_TRANSACTIONS && NULL != (tx) \
        ? BRTransactionCopy(tx) \
        : (tx))

/* Forward Declarations */
static void syncStarted(void *info);
static void syncStopped(void *info, int error);
static void txStatusUpdate(void *info);
static void saveBlocks(void *info, int replace, BRMerkleBlock *blocks[], size_t blockCount);
static void savePeers(void *info, int replace, const BRPeer peers[], size_t count);
static int networkIsReachable(void *info);
static void threadCleanup(void *info);

static void txPublished (void *info, int error);

//
// Statically Initialize Java References
//
static jclass blockClass;
static jmethodID blockConstructor;

static jclass peerClass;
static jmethodID peerConstructor;

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getConnectStatusValue
 * Signature: ()I
 */
JNIEXPORT jint
JNICALL Java_com_breadwallet_core_BRCorePeerManager_getConnectStatusValue
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerConnectStatus(peerManager);
}


/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    jniConnect
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_breadwallet_core_BRCorePeerManager_connect
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    BRPeerManagerConnect(peerManager);
    return;
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    jniDisconnect
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_breadwallet_core_BRCorePeerManager_disconnect
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    BRPeerManagerDisconnect (peerManager);
    return;
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    rescan
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_breadwallet_core_BRCorePeerManager_rescan
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    BRPeerManagerRescan (peerManager);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getEstimatedBlockHeight
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_breadwallet_core_BRCorePeerManager_getEstimatedBlockHeight
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerEstimatedBlockHeight (peerManager);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getLastBlockHeight
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_breadwallet_core_BRCorePeerManager_getLastBlockHeight
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerLastBlockHeight (peerManager);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getLastBlockTimestamp
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_com_breadwallet_core_BRCorePeerManager_getLastBlockTimestamp
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerLastBlockTimestamp (peerManager);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getSyncProgress
 * Signature: (J)D
 */
JNIEXPORT jdouble
JNICALL Java_com_breadwallet_core_BRCorePeerManager_getSyncProgress
        (JNIEnv *env, jobject thisObject, jlong startHeight) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerSyncProgress(peerManager, (uint32_t) startHeight);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getPeerCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
        Java_com_breadwallet_core_BRCorePeerManager_getPeerCount
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    return BRPeerManagerPeerCount (peerManager);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getDownloadPeerName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_com_breadwallet_core_BRCorePeerManager_getDownloadPeerName
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    const char *name = BRPeerManagerDownloadPeerName(peerManager);
    return (*env)->NewStringUTF (env, name);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    publishTransaction
 * Signature: (Lcom/breadwallet/core/BRCoreTransaction;)V
 */
JNIEXPORT void JNICALL
Java_com_breadwallet_core_BRCorePeerManager_publishTransactionWithListener
        (JNIEnv *env, jobject thisObject, jobject transitionObject, jobject listenerObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);
    BRTransaction *transaction = (BRTransaction *) getJNIReference(env, transitionObject);

    // TODO: Dangerous - make a global listener but what if PublishTx is never called?
    jobject globalListener = (*env)->NewWeakGlobalRef (env, listenerObject);
    BRPeerManagerPublishTx(peerManager,
                           JNI_COPY_TRANSACTION(transaction),
                           globalListener,
                           txPublished);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    getRelayCount
 * Signature: ([B)J
 */
JNIEXPORT jlong JNICALL
Java_com_breadwallet_core_BRCorePeerManager_getRelayCount
        (JNIEnv *env, jobject thisObject, jbyteArray hashByteArray) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    uint8_t *hashData = (uint8_t *) (*env)->GetByteArrayElements(env, hashByteArray, 0);
    UInt256 hash = UInt256Get(hashData);

    return BRPeerManagerRelayCount(peerManager,hash);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    testSaveBlocksCallback
 * Signature: (Z[Lcom/breadwallet/core/BRCoreMerkleBlock;)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_BRCorePeerManager_testSaveBlocksCallback
        (JNIEnv *env, jobject thisObject, jboolean replace, jobjectArray blockObjectArray) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    size_t blockCount = (size_t) (*env)->GetArrayLength (env, blockObjectArray);
    BRMerkleBlock **blocks = (BRMerkleBlock **) calloc (blockCount, sizeof (BRMerkleBlock *));

    for (int i = 0; i < blockCount; i++) {
        jobject block = (*env)->GetObjectArrayElement (env, blockObjectArray, i);
        blocks[i] = (BRMerkleBlock *) getJNIReference (env, block);
        (*env)->DeleteLocalRef (env, block);
    }

    // Listener
    jfieldID listenerField = (*env)->GetFieldID (env, (*env)->GetObjectClass (env, thisObject),
                                                 "listener",
                                                 "Ljava/lang/ref/WeakReference;");
    assert (NULL != listenerField);
    jweak listenerWeakGlobalRef = (*env)->GetObjectField (env, thisObject, listenerField);

    saveBlocks(listenerWeakGlobalRef, replace, blocks, blockCount);

    if (NULL != blocks) free (blocks);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    testSavePeersCallback
 * Signature: (Z[Lcom/breadwallet/core/BRCorePeer;)V
 */
JNIEXPORT void JNICALL 
Java_com_breadwallet_core_BRCorePeerManager_testSavePeersCallback
        (JNIEnv *env, jobject thisObject, jboolean replace, jobjectArray peerObjectArray) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    size_t peerCount = (size_t) (*env)->GetArrayLength (env, peerObjectArray);
    BRPeer *peers = (BRPeer *) calloc (peerCount, sizeof (BRPeer));

    for (int i = 0; i < peerCount; i++) {
        jobject peer = (*env)->GetObjectArrayElement (env, peerObjectArray, i);
        peers[i] = *(BRPeer *) getJNIReference (env, peer);
        (*env)->DeleteLocalRef (env, peer);
    }

    // Listener
    jfieldID listenerField = (*env)->GetFieldID (env, (*env)->GetObjectClass (env, thisObject),
                                                 "listener",
                                                 "Ljava/lang/ref/WeakReference;");
    assert (NULL != listenerField);
    jweak listenerWeakGlobalRef = (*env)->GetObjectField (env, thisObject, listenerField);

    savePeers(listenerWeakGlobalRef, replace, peers, peerCount);

    if (NULL != peers) free (peers);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    jniCreateCorePeerManager
 * Signature: (Lcom/breadwallet/core/BRCoreChainParams;Lcom/breadwallet/core/BRCoreWallet;D[Lcom/breadwallet/core/BRCoreMerkleBlock;[Lcom/breadwallet/core/BRCorePeer;Lcom/breadwallet/core/BRCorePeerManager/Listener;)J
 */
JNIEXPORT jlong JNICALL
Java_com_breadwallet_core_BRCorePeerManager_createCorePeerManager
        (JNIEnv *env, jclass thisClass,
         jobject objParams,
         jobject objWallet,
         jdouble dblEarliestKeyTime,
         jobjectArray objBlocksArray,
         jobjectArray objPeersArray) {

    BRChainParams *params = (BRChainParams *) getJNIReference(env, objParams);
    BRWallet *wallet = (BRWallet *) getJNIReference(env, objWallet);
    uint32_t earliestKeyTime = (uint32_t) dblEarliestKeyTime;

    // Blocks
    size_t blocksCount = (*env)->GetArrayLength(env, objBlocksArray);
    BRMerkleBlock **blocks = (0 == blocksCount ? NULL : (BRMerkleBlock **) calloc(blocksCount, sizeof(BRMerkleBlock *)));

    // The upcoming call to BRPeerManagerNew() assumes that the blocks provided have their memory
    // ownership transferred to the Core.  Thus, we'll deep copy each block
    for (int index = 0; index < blocksCount; index++) {
        jobject objBlock = (*env)->GetObjectArrayElement (env, objBlocksArray, index);
        assert (!(*env)->IsSameObject (env, objBlock, NULL));

        BRMerkleBlock *block = (BRMerkleBlock *) getJNIReference(env, objBlock);
        assert (NULL != block);

        blocks[index] = BRMerkleBlockCopy(block);
        (*env)->DeleteLocalRef (env, objBlock);
    }

    // Peers
    size_t peersCount = (*env)->GetArrayLength(env, objPeersArray);
    BRPeer *peers = (0 == peersCount ? NULL : (BRPeer *) calloc(peersCount, sizeof(BRPeer)));

    for (int index =0; index < peersCount; index++) {
        jobject objPeer = (*env)->GetObjectArrayElement (env, objPeersArray, index);
        assert (!(*env)->IsSameObject (env, objPeer, NULL));

        BRPeer *peer = getJNIReference(env, objPeer);
        assert (NULL != peer);

        peers[index] = *peer; // block assignment
        (*env)->DeleteLocalRef (env, objPeer);
    }

    BRPeerManager *result = BRPeerManagerNew(params, wallet, earliestKeyTime,
                                             blocks, blocksCount,
                                             peers, peersCount);

    if (NULL != blocks) free(blocks);
    if (NULL != peers ) free(peers);

    return (jlong) result;
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    installListener
 * Signature: (Lcom/breadwallet/core/BRCorePeerManager/Listener;)V
 */
JNIEXPORT void
JNICALL Java_com_breadwallet_core_BRCorePeerManager_installListener
        (JNIEnv *env, jobject thisObject, jobject listenerObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    // Get a WeakGlobalRef - 'weak' to allow for GC; 'global' to allow BRCore thread access
    jobject listenerWeakRefGlobal = (*env)->NewWeakGlobalRef(env, listenerObject);

    // Assign listenerWeakRefGlobal back to thisObject.listener
    jfieldID listenerField = (*env)->GetFieldID(env, (*env)->GetObjectClass(env, thisObject),
                                                "listener",
                                                "Ljava/lang/ref/WeakReference;");
    assert (NULL != listenerField);
    (*env)->SetObjectField(env, thisObject, listenerField, listenerWeakRefGlobal);

    // Assign callbacks
    BRPeerManagerSetCallbacks (peerManager, (void *) listenerWeakRefGlobal,
                               syncStarted,
                               syncStopped,
                               txStatusUpdate,
                               saveBlocks,
                               savePeers,
                               networkIsReachable,
                               threadCleanup);
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    disposeNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_breadwallet_core_BRCorePeerManager_disposeNative
        (JNIEnv *env, jobject thisObject) {
    BRPeerManager *peerManager = (BRPeerManager *) getJNIReference(env, thisObject);

    // Locate 'globalListener', then DeleteWeakGlobalRef() to save global reference space.
    if (NULL != peerManager) {
        jfieldID listenerField = (*env)->GetFieldID (env, (*env)->GetObjectClass (env, thisObject),
                                                     "listener",
                                                     "Ljava/lang/ref/WeakReference;");
        assert (NULL != listenerField);

        jweak listenerWeakGlobalRef = (*env)->GetObjectField (env, thisObject, listenerField);
        if (JNIWeakGlobalRefType == (*env)->GetObjectRefType (env, listenerWeakGlobalRef)) {
            (*env)->DeleteWeakGlobalRef (env, listenerWeakGlobalRef);
        }

        BRPeerManagerFree(peerManager);
    }
}

/*
 * Class:     com_breadwallet_core_BRCorePeerManager
 * Method:    initializeNative
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_BRCorePeerManager_initializeNative
        (JNIEnv *env, jclass thisClass) {
    blockClass = (*env)->FindClass(env, "com/breadwallet/core/BRCoreMerkleBlock");
    assert (NULL != blockClass);
    blockClass = (*env)->NewGlobalRef (env, blockClass);

    blockConstructor = (*env)->GetMethodID(env, blockClass, "<init>", "(J)V");
    assert (NULL != blockConstructor);

    peerClass = (*env)->FindClass(env, "com/breadwallet/core/BRCorePeer");
    assert (NULL != peerClass);
    peerClass = (*env)->NewGlobalRef (env, peerClass);

    peerConstructor = (*env)->GetMethodID(env, peerClass, "<init>", "(J)V");
    assert (NULL != peerConstructor);
}

//
// Callbacks
//
static jmethodID
lookupListenerMethod (JNIEnv *env, jobject listener, char *name, char *type) {
    return (*env)->GetMethodID(env,
                               (*env)->GetObjectClass(env, listener),
                               name,
                               type);
}

static void showClassName (JNIEnv *env, jobject object, char *message) {
//    __android_log_print(ANDROID_LOG_DEBUG, "JNI", "Listener @ %s : %s", message, jniGetClassName(env, object));
}

static void
syncStarted(void *info) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "syncStarted");

    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "syncStarted",
                                 "()V");
    (*env)->CallVoidMethod(env, listener, listenerMethod);
    (*env)->DeleteLocalRef (env, listener);
}

static void
syncStopped(void *info, int error) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "syncStopped");

    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "syncStopped",
                                 "(Ljava/lang/String;)V");

    jstring errorString = (*env)->NewStringUTF (env, (error == 0 ? "" : strerror (error)));

    (*env)->CallVoidMethod(env, listener, listenerMethod, errorString);
    (*env)->DeleteLocalRef (env, listener);
}

static void
txStatusUpdate(void *info) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "txStatusUpdate");

    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "txStatusUpdate",
                                 "()V");

    (*env)->CallVoidMethod(env, listener, listenerMethod);
    (*env)->DeleteLocalRef (env, listener);
}

static void
saveBlocks(void *info, int replace, BRMerkleBlock *blocks[], size_t blockCount) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef(env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "saveBlocks");

    // The saveBlocks callback
    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "saveBlocks",
                                 "(Z[Lcom/breadwallet/core/BRCoreMerkleBlock;)V");
    assert (NULL != listenerMethod);

    // Create the Java BRCoreMerkleBlock array
    jobjectArray blockArray = (*env)->NewObjectArray(env, blockCount, blockClass, 0);

    for (int index = 0; index < blockCount; index++) {
        jobject blockObject = (*env)->NewObject(env, blockClass, blockConstructor,
                                                (jlong) BRMerkleBlockCopy(blocks[index]));

        (*env)->SetObjectArrayElement(env, blockArray, index, blockObject);
        (*env)->DeleteLocalRef(env, blockObject);
    }

    // Invoke the callback, fully constituted with the blocks array.
    (*env)->CallVoidMethod(env, listener, listenerMethod, replace, blockArray);
    (*env)->DeleteLocalRef(env, listener);
}

static void
savePeers(void *info, int replace, const BRPeer peers[], size_t count) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "savePeers");

    // The savePeers callback
    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "savePeers",
                                 "(Z[Lcom/breadwallet/core/BRCorePeer;)V");
    assert (NULL != listenerMethod);

    jobjectArray peerArray = (*env)->NewObjectArray(env, count, peerClass, 0);

    for (int index = 0; index < count; index++) {
        BRPeer *peer = (BRPeer *) malloc(sizeof(BRPeer));
        *peer = peers[index];

        jobject peerObject =
                (*env)->NewObject (env, peerClass, peerConstructor, (jlong) peer);

        (*env)->SetObjectArrayElement(env, peerArray, index, peerObject);
        (*env)->DeleteLocalRef (env, peerObject);
    }

    // Invoke the callback, fully constituted with the peers array.
    (*env)->CallVoidMethod (env, listener, listenerMethod, replace, peerArray);
    (*env)->DeleteLocalRef (env, listener);
}

static int
networkIsReachable(void *info) {
    JNIEnv *env = getEnv();
    if (NULL == env) return 0;

    jobject listener = (*env)->NewLocalRef(env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return 0; // GC reclaimed

    showClassName(env, listener, "networkIsReachable");

    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "networkIsReachable",
                                 "()Z");
    assert (NULL != listenerMethod);

    int networkIsOn = (*env)->CallBooleanMethod(env, listener, listenerMethod);
    (*env)->DeleteLocalRef(env, listener);

    return networkIsOn == JNI_TRUE;
}

static void
txPublished (void *info, int error) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    // Info is a GlobalWeakRef - by using NewLocalRef, we save the reference,
    // if it has not been reclaimed yet.  If it has been reclaimed, then it is NULL;
    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if ((*env)->IsSameObject (env, listener, NULL)) return; // GC reclaimed

    showClassName(env, listener, "txPublished");

    // Ensure this; see comment above (on txPublished use)
    (*env)->DeleteWeakGlobalRef (env, info);

    jmethodID listenerMethod =
            lookupListenerMethod(env, listener,
                                 "txPublished",
                                 "(Ljava/lang/String;)V");
    assert (NULL != listenerMethod);

    jstring errorString = (*env)->NewStringUTF (env, (error == 0 ? "" : strerror (error)));

    (*env)->CallVoidMethod(env, listener, listenerMethod, errorString);
    (*env)->DeleteLocalRef (env, listener);
}

static void
threadCleanup(void *info) {
    JNIEnv *env = getEnv();
    if (NULL == env) return;

    jobject listener = (*env)->NewLocalRef (env, (jobject) info);
    if (NULL == listener) return; // GC reclaimed

    showClassName(env, listener, "threadCleanup");

    (*env)->DeleteLocalRef (env, listener);

    releaseEnv();
}


