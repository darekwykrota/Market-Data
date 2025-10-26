#ifndef _EOBI_LOG_H_
#define _EOBI_LOG_H_

#include "logger/logger.h"

#define EOBI_ID 30090
#define EOBI_DEBUG() LOG(DEBUG, EOBI_ID)
#define EOBI_INFO() LOG(INFO, EOBI_ID)
#define EOBI_LOCAL() LOG_LOCAL(INFO, EOBI_ID)
#define EOBI_WARN() LOG(WARNING, EOBI_ID)
#define EOBI_ERR() LOG(ERROR, EOBI_ID)

#endif
