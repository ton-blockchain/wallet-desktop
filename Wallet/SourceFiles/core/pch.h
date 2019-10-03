// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QWidget>

#include <cmath>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <optional>

#include <range/v3/all.hpp>

#include "base/assertion.h"
#include "base/basic_types.h"
#include "base/variant.h"
#include "base/optional.h"
#include "base/algorithm.h"
#include "base/invoke_queued.h"
#include "base/flat_set.h"
#include "base/flat_map.h"
#include "base/weak_ptr.h"
#include "base/observer.h"

#include <gsl/gsl>
#include <rpl/rpl.h>
#include <crl/crl.h>
