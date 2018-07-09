#ifndef CRETE_INCLUDE_COMMON
#define CRETE_INCLUDE_COMMON

#ifdef __cplusplus
extern "C" {
#endif

static const char *CRETE_RAMDISK_PATH = "/tmp/ramdisk";
static const char *CRETE_PROC_MAPS_PATH = "/tmp/proc-maps.log";
static const char *CRETE_CONFIG_SERIALIZED_PATH = "/tmp/harness.config.serialized";
static const char *CRETE_ENV_SEC_CMD = "CRETE_ENV_SEC_CMD";
static const char *CRETE_CONCOLIC_NAME_SUFFIX = "CRETE_CONCOLIC_NAME_SUFFIX";

static const char *CRETE_SANDBOX_PATH = "/tmp/sandbox";

static const char *CRETE_REPLAY_CURRENT_TC = "/tmp/crete.replay.current.tc.bin";
static const char *CRETE_REPLAY_GCOV_PREFIX = "/tmp/gcov";

static const char *CRETE_SVM_TEST_FOLDER = "crete_svm_test_pool";
static const char *CRETE_SVM_TEST_FOLDER_SE = "crete_svm_test_pool_se";

// CUSTOMIZED EXIT CODE
static const int CRETE_EXIT_CODE_SIG_BASE = 30;

#ifdef __cplusplus
}
#endif

#endif
