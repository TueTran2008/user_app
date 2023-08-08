#ifndef GSM_FILEFS_H
#define GSM_FILEFS_H

#include <stdint.h>

typedef enum
{
    GSM_FILE_FS_BACKUP_THEN_UPDATE_FIRMWARE,
    GSM_FILE_FS_ROLLBACK_FIRMWARE,
} gsm_file_fs_type_t;

/**
 * @brief       Start file system download
 */
void gsm_file_fs_start(gsm_file_fs_type_t type);

#endif /* GSM_FILEFS_H */
