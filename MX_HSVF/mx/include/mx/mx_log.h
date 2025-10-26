#ifndef _MX_LOG_H_
#define _MX_LOG_H_

#include "logger/logger.h"

#define MX_ID 30090
#define MX_DEBUG() LOG(DEBUG, MX_ID)
#define MX_INFO() LOG(INFO, MX_ID)
#define MX_LOCAL() LOG_LOCAL(INFO, MX_ID)
#define MX_WARN() LOG(WARNING, MX_ID)
#define MX_ERR() LOG(ERROR, MX_ID)

#endif
