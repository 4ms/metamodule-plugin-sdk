#pragma once
#ifdef VCVRACK

#include "osdialog/osdialog.h"

#else

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osdialog_filters osdialog_filters;

typedef enum {
	OSDIALOG_OPEN,
	OSDIALOG_OPEN_DIR,
	OSDIALOG_SAVE,
} osdialog_file_action;

#ifdef __cplusplus
}
#endif

#endif
