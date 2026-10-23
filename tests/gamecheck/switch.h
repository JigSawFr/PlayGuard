// Minimal fake <switch.h> for the gamecheck host tests: the ns and ncm calls
// core/gamecheck.c makes, played by tests/gamecheck/test.c.
#pragma once
#include "fake_switch_base.h"

#define KERNELRESULT(description) MAKERESULT(Module_Kernel, KernelError_##description)
enum { Module_Kernel = 1, Module_Libnx = 345 };
enum { KernelError_TimedOut = 117 };
enum { LibnxError_OutOfMemory = 2 };

bool hosversionAtLeast(u8 major, u8 minor, u8 micro);

// Services and raw commands (es, ns IApplicationManagerInterface): the mock
// sees the command, the input value, the output value and the buffers.
typedef struct { int id; bool open; } Service;
enum {
    SfBufferAttr_In           = 1u << 0,
    SfBufferAttr_Out          = 1u << 1,
    SfBufferAttr_HipcMapAlias = 1u << 2,
};
typedef struct { const void *ptr; size_t size; } SfBuffer;
typedef struct { u32 buffer_attrs[8]; SfBuffer buffers[8]; } SfDispatchParams;
Result mock_dispatch(Service *srv, u32 command, void *out, size_t out_size,
                     const void *in, size_t in_size, SfDispatchParams params);
#define serviceDispatch(srv, command, ...) \
    mock_dispatch((srv), (command), NULL, 0, NULL, 0, (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchOut(srv, command, value, ...) \
    mock_dispatch((srv), (command), &(value), sizeof(value), NULL, 0, (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchIn(srv, command, value, ...) \
    mock_dispatch((srv), (command), NULL, 0, &(value), sizeof(value), (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchInOut(srv, command, in_value, out_value, ...) \
    mock_dispatch((srv), (command), &(out_value), sizeof(out_value), &(in_value), sizeof(in_value), \
                  (SfDispatchParams){ __VA_ARGS__ })
Result smGetService(Service *out, const char *name);
void serviceClose(Service *srv);

typedef enum {
    NcmStorageId_None = 0, NcmStorageId_Host = 1, NcmStorageId_GameCard = 2, NcmStorageId_BuiltInSystem = 3,
    NcmStorageId_BuiltInUser = 4, NcmStorageId_SdCard = 5, NcmStorageId_Any = 6,
} NcmStorageId;
typedef enum {
    NcmContentMetaType_Application = 0x80, NcmContentMetaType_Patch = 0x81, NcmContentMetaType_AddOnContent = 0x82,
} NcmContentMetaType;
typedef enum { NcmContentInstallType_Full = 0 } NcmContentInstallType;
typedef enum { NcmContentType_Meta = 0, NcmContentType_Program = 1, NcmContentType_Data = 2, NcmContentType_Control = 3 } NcmContentType;

typedef struct { u8 c[0x10]; } NcmContentId;
typedef struct {
    u64 id;
    u32 version;
    u8 type;
    u8 install_type;
    u8 padding[2];
} NcmContentMetaKey;
typedef struct {
    NcmContentId content_id;
    u32 size_low;
    u8 size_high, attr, content_type, id_offset;
} NcmContentInfo;
typedef struct { u8 c[0x10]; } FsRightsId;
typedef enum { FsContentAttributes_None = 0, FsContentAttributes_All = 0xF } FsContentAttributes;
typedef struct { FsRightsId rights_id; u8 key_generation; u8 pad[7]; } NcmRightsId;
typedef struct { NcmStorageId storage; bool open; } NcmContentMetaDatabase;
typedef struct { NcmStorageId storage; bool open; } NcmContentStorage;

Result ncmInitialize(void);
void ncmExit(void);
Result ncmOpenContentMetaDatabase(NcmContentMetaDatabase *out, NcmStorageId storage_id);
Result ncmOpenContentStorage(NcmContentStorage *out, NcmStorageId storage_id);
void ncmContentMetaDatabaseClose(NcmContentMetaDatabase *db);
void ncmContentStorageClose(NcmContentStorage *cs);
Result ncmContentMetaDatabaseHas(NcmContentMetaDatabase *db, bool *out, const NcmContentMetaKey *key);
Result ncmContentMetaDatabaseGetRequiredSystemVersion(NcmContentMetaDatabase *db, u32 *out_version, const NcmContentMetaKey *key);
Result ncmContentMetaDatabaseListContentInfo(NcmContentMetaDatabase *db, s32 *out_entries_written, NcmContentInfo *out_info,
                                             s32 count, const NcmContentMetaKey *key, s32 start_index);
Result ncmContentStorageHas(NcmContentStorage *cs, bool *out, const NcmContentId *content_id);
Result ncmContentStorageGetRightsIdFromContentId(NcmContentStorage *cs, NcmRightsId *out, const NcmContentId *content_id,
                                                 FsContentAttributes attr);
Result ncmContentStorageDelete(NcmContentStorage *cs, const NcmContentId *content_id);
Result ncmContentMetaDatabaseRemove(NcmContentMetaDatabase *db, const NcmContentMetaKey *key);
Result ncmContentMetaDatabaseCommit(NcmContentMetaDatabase *db);

Result avmInitialize(void);
void avmExit(void);
Result avmGetLaunchRequiredVersion(u64 application_id, u32 *version);
Result avmPushLaunchVersion(u64 application_id, u32 version);

typedef struct { char name[0x200]; char author[0x100]; } NacpLanguageEntry;
// libnx's layout where nacp.c reads it: the title entries at 0, the ages at
// 0x3040, the display version at 0x3060, the titles' format at 0x3215, 0x4000
// bytes in all.
typedef struct { NacpLanguageEntry lang[16]; u8 x3000[0x40]; int8_t rating_age[0x20]; char display_version[0x10]; u8 rest[0xF90]; } NacpStruct;
typedef struct { NacpStruct nacp; u8 icon[0x20000]; } NsApplicationControlData;
typedef enum { NsApplicationControlSource_CacheOnly = 0, NsApplicationControlSource_Storage = 1 } NsApplicationControlSource;
typedef struct {
    u64 application_id;
    u8 last_event, attributes, reserved[6];
    u64 last_updated;
} NsApplicationRecord;
typedef struct {
    u8 meta_type, storageID, rights_check, reserved;
    u32 version;
    u64 application_id;
} NsApplicationContentMetaStatus;

typedef struct {
    u64 application_id;
    u8 unk_x8[0x4];
    u32 flags;
    u8 rest[0x40];
} NsApplicationView;
typedef struct { u8 unk_x0[0x80]; } NsApplicationOccupiedSize;
Result nsCalculateApplicationOccupiedSize(u64 application_id, NsApplicationOccupiedSize *out);
typedef struct { Service s; bool open; } NsProgressAsyncResult;
typedef struct { s64 current_size; s64 total_size; } NsSystemUpdateProgress;

Result nsInitialize(void);
void nsExit(void);
Result nsListApplicationRecord(NsApplicationRecord *records, s32 count, s32 entry_offset, s32 *out_entrycount);
Result nsGetApplicationControlData(NsApplicationControlSource source, u64 application_id, NsApplicationControlData *buffer,
                                   size_t size, u64 *actual_size);
Result nsListApplicationContentMetaStatus(u64 application_id, s32 index, NsApplicationContentMetaStatus *list, s32 count,
                                          s32 *out_entrycount);
Result nsCheckApplicationLaunchVersion(u64 application_id);
Result nsDeleteApplicationCompletely(u64 application_id);
Result nsGetApplicationView(NsApplicationView *views, const u64 *application_ids, s32 count);
Result nsGetApplicationManagerInterface(Service *srv_out);
Service *nsGetServiceSession_ApplicationManagerInterface(void);
Result nsRequestVerifyApplication(NsProgressAsyncResult *a, u64 application_id, u32 unk, void *buffer, size_t size);
Result nsProgressAsyncResultWait(NsProgressAsyncResult *a, u64 timeout);
Result nsProgressAsyncResultGet(NsProgressAsyncResult *a);
Result nsProgressAsyncResultGetProgress(NsProgressAsyncResult *a, void *buffer, size_t size);
Result nsProgressAsyncResultGetDetailResult(NsProgressAsyncResult *a);
Result nsProgressAsyncResultCancel(NsProgressAsyncResult *a);
void nsProgressAsyncResultClose(NsProgressAsyncResult *a);

// set: the console language (core/nacp_read.c), libnx's SetLanguage values.
typedef enum {
    SetLanguage_JA = 0, SetLanguage_ENUS = 1, SetLanguage_FR = 2, SetLanguage_DE = 3, SetLanguage_IT = 4,
    SetLanguage_ES = 5, SetLanguage_ZHCN = 6, SetLanguage_KO = 7, SetLanguage_NL = 8, SetLanguage_PT = 9,
    SetLanguage_RU = 10, SetLanguage_ZHTW = 11, SetLanguage_ENGB = 12, SetLanguage_FRCA = 13, SetLanguage_ES419 = 14,
    SetLanguage_ZHHANS = 15, SetLanguage_ZHHANT = 16, SetLanguage_PTBR = 17, SetLanguage_Total = 18,
} SetLanguage;
Result setInitialize(void);
void setExit(void);
Result setGetSystemLanguage(u64 *language_code);
Result setMakeLanguage(u64 language_code, SetLanguage *language);
